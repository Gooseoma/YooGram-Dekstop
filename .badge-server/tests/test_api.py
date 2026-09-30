import base64
import json
import re
import time

import pytest
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey
from fastapi.testclient import TestClient

from app.config import Settings
from app.main import create_app

PASSWORD = "correct-horse-battery"


@pytest.fixture()
def client(tmp_path):
    settings = Settings(
        admin_password=PASSWORD,
        session_secret="x" * 40,
        data_dir=tmp_path,
        cookie_secure=False,
    )
    return TestClient(create_app(settings))


def login(client):
    r = client.post("/admin/login", data={"password": PASSWORD}, follow_redirects=False)
    assert r.status_code == 303
    page = client.get("/admin").text
    return re.search(r'name="csrf" value="([^"]+)"', page).group(1)


def grant(client, csrf, **over):
    data = {"csrf": csrf, "tg_id": "123", "kind": "emoji", "emoji": "✅"}
    data.update(over)
    return client.post("/admin/grant", data=data)


def fetch(client):
    body = client.get("/v1/badges").json()
    pub = Ed25519PublicKey.from_public_bytes(
        base64.b64decode(client.get("/v1/pubkey").json()["public_key"])
    )
    pub.verify(base64.b64decode(body["sig"]), body["payload"].encode("utf-8"))
    return json.loads(body["payload"])


def test_empty_list_is_signed(client):
    data = fetch(client)
    assert data["badges"] == {}


def test_tampered_payload_fails_verification(client):
    body = client.get("/v1/badges").json()
    pub = Ed25519PublicKey.from_public_bytes(
        base64.b64decode(client.get("/v1/pubkey").json()["public_key"])
    )
    with pytest.raises(Exception):
        pub.verify(base64.b64decode(body["sig"]), (body["payload"] + " ").encode())


def test_grant_emoji_and_label(client):
    csrf = login(client)
    assert grant(client, csrf).status_code == 200
    r = grant(
        client, csrf, tg_id="456", kind="label", label="Developer",
        color="#FF0000", tooltip="YooGram dev",
    )
    assert r.status_code == 200
    badges = fetch(client)["badges"]
    assert badges["123"] == {"kind": "emoji", "color": "#2a9df4", "emoji": "✅"}
    assert badges["456"] == {
        "kind": "label", "color": "#ff0000", "label": "Developer",
        "tooltip": "YooGram dev",
    }


def test_version_bumps_and_etag(client):
    csrf = login(client)
    first = client.get("/v1/badges")
    assert client.get(
        "/v1/badges", headers={"If-None-Match": first.headers["etag"]}
    ).status_code == 304
    v0 = fetch(client)["version"]
    grant(client, csrf)
    assert fetch(client)["version"] == v0 + 1
    assert client.get(
        "/v1/badges", headers={"If-None-Match": first.headers["etag"]}
    ).status_code == 200


def test_revoke(client):
    csrf = login(client)
    grant(client, csrf)
    client.post("/admin/revoke", data={"csrf": csrf, "tg_id": "123"})
    assert fetch(client)["badges"] == {}


def test_expired_grants_are_dropped_and_version_bumps(tmp_path):
    from app.db import Store

    store = Store(tmp_path / "badges.db")
    store.grant(1, "emoji", "x", "", "#2a9df4", "", expires_at=int(time.time()) - 10)
    store.grant(2, "emoji", "y", "", "#2a9df4", "", expires_at=int(time.time()) + 999)
    before = store._db.execute("SELECT value FROM meta").fetchone()[0]
    assert [g.tg_id for g in store.all()] == [2]
    assert store.version() == before + 1


def test_grant_with_days_sets_expiry(client):
    csrf = login(client)
    grant(client, csrf, days="1")
    assert "123" in fetch(client)["badges"]


@pytest.mark.parametrize("over", [
    {"tg_id": "abc"}, {"tg_id": "-5"}, {"kind": "weird"},
    {"kind": "emoji", "emoji": ""}, {"kind": "label", "label": ""},
    {"color": "red"}, {"kind": "label", "label": "x" * 25},
    {"days": "0"}, {"days": "99999"},
])
def test_validation_rejects(client, over):
    csrf = login(client)
    assert grant(client, csrf, **over).status_code == 400
    assert fetch(client)["badges"] == {}


def test_admin_requires_login(client):
    assert client.get("/admin", follow_redirects=False).status_code == 303
    r = client.post("/admin/grant", data={"tg_id": "1", "emoji": "x"},
                    follow_redirects=False)
    assert r.status_code == 303
    assert fetch(client)["badges"] == {}


def test_csrf_required(client):
    login(client)
    assert grant(client, "wrong").status_code == 403
    assert fetch(client)["badges"] == {}


def test_wrong_password_and_lockout(client):
    for _ in range(5):
        assert client.post("/admin/login", data={"password": "nope"}).status_code == 401
    r = client.post("/admin/login", data={"password": PASSWORD})
    assert r.status_code == 429


def test_xss_is_escaped(client):
    csrf = login(client)
    grant(client, csrf, kind="label", label="<script>x</script>")
    page = client.get("/admin").text
    assert "<script>x</script>" not in page
    assert "&lt;script&gt;" in page


def test_docs_disabled(client):
    assert client.get("/docs").status_code == 404
    assert client.get("/openapi.json").status_code == 404
