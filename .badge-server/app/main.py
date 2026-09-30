import hashlib
import hmac
import json
import secrets
import time
from pathlib import Path

from fastapi import FastAPI, Form, Request
from fastapi.responses import JSONResponse, PlainTextResponse, RedirectResponse, Response
from fastapi.templating import Jinja2Templates
from starlette.middleware.sessions import SessionMiddleware

from .config import Settings, load_settings
from .db import Store, ValidationError, validate
from .signing import Signer

TEMPLATES = Jinja2Templates(directory=str(Path(__file__).parent / "templates"))
MAX_FAILED_LOGINS = 5
LOCKOUT_SECONDS = 15 * 60


def client_ip(request: Request) -> str:
    return request.headers.get("cf-connecting-ip") or (
        request.client.host if request.client else "unknown"
    )


def build_payload(store: Store) -> str:
    badges = {}
    for g in store.all():
        entry = {"kind": g.kind, "color": g.color}
        if g.kind == "emoji":
            entry["emoji"] = g.emoji
        else:
            entry["label"] = g.label
        if g.tooltip:
            entry["tooltip"] = g.tooltip
        badges[str(g.tg_id)] = entry
    return json.dumps(
        {"v": 1, "version": store.version(), "badges": badges},
        sort_keys=True,
        ensure_ascii=False,
        separators=(",", ":"),
    )


def create_app(settings: Settings | None = None) -> FastAPI:
    settings = settings or load_settings()
    store = Store(settings.db_path)
    signer = Signer(settings.key_path)
    failures: dict[str, list[float]] = {}

    app = FastAPI(docs_url=None, redoc_url=None, openapi_url=None)
    app.add_middleware(
        SessionMiddleware,
        secret_key=settings.session_secret,
        session_cookie="yg_admin",
        https_only=settings.cookie_secure,
        same_site="strict",
        max_age=8 * 3600,
    )

    @app.middleware("http")
    async def security_headers(request: Request, call_next):
        response = await call_next(request)
        response.headers["X-Content-Type-Options"] = "nosniff"
        response.headers["X-Frame-Options"] = "DENY"
        response.headers["Referrer-Policy"] = "no-referrer"
        response.headers["Content-Security-Policy"] = (
            "default-src 'none'; style-src 'unsafe-inline'; form-action 'self'; "
            "base-uri 'none'; frame-ancestors 'none'"
        )
        if request.url.path.startswith("/admin"):
            response.headers["Cache-Control"] = "no-store"
        return response

    def is_admin(request: Request) -> bool:
        return bool(request.session.get("admin"))

    def csrf_ok(request: Request, token: str) -> bool:
        expected = request.session.get("csrf", "")
        return bool(expected) and hmac.compare_digest(expected, token)

    def locked_out(ip: str) -> bool:
        now = time.time()
        recent = [t for t in failures.get(ip, []) if now - t < LOCKOUT_SECONDS]
        failures[ip] = recent
        return len(recent) >= MAX_FAILED_LOGINS

    # ---------- public API ----------

    @app.get("/healthz")
    def healthz():
        return PlainTextResponse("ok")

    @app.get("/v1/pubkey")
    def pubkey():
        return {"alg": "ed25519", "public_key": signer.public_key_b64()}

    @app.get("/v1/badges")
    def badges(request: Request):
        payload = build_payload(store)
        etag = '"' + hashlib.sha256(payload.encode()).hexdigest()[:32] + '"'
        headers = {"ETag": etag, "Cache-Control": "public, max-age=60"}
        if request.headers.get("if-none-match") == etag:
            return Response(status_code=304, headers=headers)
        body = {"payload": payload, "sig": signer.sign(payload.encode("utf-8"))}
        return JSONResponse(body, headers=headers)

    # ---------- admin ----------

    @app.get("/")
    def root():
        return RedirectResponse("/admin", status_code=303)

    @app.get("/admin/login")
    def login_page(request: Request):
        return TEMPLATES.TemplateResponse(request, "login.html", {"error": None})

    @app.post("/admin/login")
    def login(request: Request, password: str = Form("")):
        ip = client_ip(request)
        if locked_out(ip):
            return TEMPLATES.TemplateResponse(
                request,
                "login.html",
                {"error": "Too many attempts. Try again later."},
                status_code=429,
            )
        if not hmac.compare_digest(
            password.encode(), settings.admin_password.encode()
        ):
            failures.setdefault(ip, []).append(time.time())
            return TEMPLATES.TemplateResponse(
                request, "login.html", {"error": "Wrong password."}, status_code=401
            )
        failures.pop(ip, None)
        request.session.clear()
        request.session["admin"] = True
        request.session["csrf"] = secrets.token_urlsafe(32)
        return RedirectResponse("/admin", status_code=303)

    @app.post("/admin/logout")
    def logout(request: Request, csrf: str = Form("")):
        if is_admin(request) and csrf_ok(request, csrf):
            request.session.clear()
        return RedirectResponse("/admin/login", status_code=303)

    def render_admin(request: Request, error=None, notice=None, status_code=200):
        return TEMPLATES.TemplateResponse(
            request,
            "admin.html",
            {
                "grants": store.all(),
                "csrf": request.session["csrf"],
                "public_key": signer.public_key_b64(),
                "version": store.version(),
                "error": error,
                "notice": notice,
                "now": int(time.time()),
            },
            status_code=status_code,
        )

    @app.get("/admin")
    def admin(request: Request):
        if not is_admin(request):
            return RedirectResponse("/admin/login", status_code=303)
        return render_admin(request)

    @app.post("/admin/grant")
    def grant(
        request: Request,
        csrf: str = Form(""),
        tg_id: str = Form(""),
        kind: str = Form("emoji"),
        emoji: str = Form(""),
        label: str = Form(""),
        color: str = Form(""),
        tooltip: str = Form(""),
        days: str = Form(""),
    ):
        if not is_admin(request):
            return RedirectResponse("/admin/login", status_code=303)
        if not csrf_ok(request, csrf):
            return render_admin(request, error="Bad CSRF token.", status_code=403)
        try:
            values = validate(tg_id, kind, emoji, label, color, tooltip)
            expires_at = None
            if days.strip():
                n = int(days.strip())
                if not 1 <= n <= 3650:
                    raise ValidationError("Days must be between 1 and 3650")
                expires_at = int(time.time()) + n * 86400
        except ValueError as e:
            return render_admin(request, error=str(e), status_code=400)
        store.grant(*values, expires_at=expires_at)
        return render_admin(request, notice=f"Badge saved for {values[0]}.")

    @app.post("/admin/revoke")
    def revoke(request: Request, csrf: str = Form(""), tg_id: str = Form("")):
        if not is_admin(request):
            return RedirectResponse("/admin/login", status_code=303)
        if not csrf_ok(request, csrf):
            return render_admin(request, error="Bad CSRF token.", status_code=403)
        try:
            removed = store.revoke(int(tg_id))
        except ValueError:
            return render_admin(request, error="Bad ID.", status_code=400)
        return render_admin(
            request, notice="Badge removed." if removed else "Nothing to remove."
        )

    return app
