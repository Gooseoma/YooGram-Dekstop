# YooGram badge server

Small service that hands the YooGram desktop client a signed list of
`Telegram user ID -> badge`. Badges are either an **emoji** (like a verified
check) or a **text label** (like "Developer"). Issue and remove them from a
web admin page, no client update needed.

> Lives in `.badge-server/` only to keep it apart from the client sources.
> Do not copy it into the client tree; move it to its own repository.

## Run

```bash
cp .env.example .env        # set a long admin password and a random secret
python3 -c "import secrets; print(secrets.token_hex(32))"   # for the secret
docker compose up -d --build
```

Open `/admin`, sign in, add badges. The page also shows the **public key**:
embed it in the client. The private key is generated on first start in
`data/signing_key.pem` (mode 600). Back it up: losing it means shipping a new
public key in a client update.

Without Docker: `pip install -r requirements.txt` and
`uvicorn app.asgi:app --port 8080 --proxy-headers`
(load `.env` into the environment first).

## API

| Endpoint | Purpose |
|---|---|
| `GET /v1/badges` | `{"payload": "<json string>", "sig": "<base64 ed25519>"}`. Supports `ETag` / `If-None-Match` (304). |
| `GET /v1/pubkey` | Public key, for checking it matches the one in the client. |
| `GET /healthz` | Liveness. |

The signature covers the exact bytes of the `payload` string (UTF-8), so the
client must verify first and only then parse. Payload:

```json
{"v":1,"version":12,"badges":{"123456789":{"kind":"emoji","color":"#2a9df4","emoji":"✅","tooltip":"Developer"},
                              "987654321":{"kind":"label","color":"#ff0000","label":"Developer"}}}
```

Client rules: verify the signature; ignore the list if `version` is lower than
the cached one; keep the last good list when the server is unreachable.

## Cloudflare

With SSL mode **Flexible** the hop Cloudflare -> server is plain HTTP, so the
admin password and session cookie cross it unencrypted. Badge data is safe
(signed), the admin login is not. Better options, in order:

1. **Cloudflare Tunnel** (`cloudflared`): no open port, encrypted end to end.
   Point it at `http://localhost:8080` and drop the `ports:` mapping.
2. **Origin Certificate** + SSL mode *Full (strict)*, with nginx/caddy on the
   server terminating TLS.
3. At minimum: firewall the server to Cloudflare IP ranges only, and put
   `/admin*` behind Cloudflare Access or an IP rule.

The cookie is `Secure` by default (browser talks HTTPS to Cloudflare).
Real client IPs for login throttling come from `CF-Connecting-IP`; that header
is only trustworthy when the server accepts traffic from Cloudflare alone.

## Tests

```bash
pip install -r requirements-dev.txt
python -m pytest
```
