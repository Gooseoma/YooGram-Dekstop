import base64
import os
from pathlib import Path

from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey


class Signer:
    def __init__(self, key_path: Path):
        if key_path.exists():
            key = serialization.load_pem_private_key(key_path.read_bytes(), None)
            if not isinstance(key, Ed25519PrivateKey):
                raise RuntimeError("signing key must be Ed25519")
        else:
            key = Ed25519PrivateKey.generate()
            key_path.parent.mkdir(parents=True, exist_ok=True)
            pem = key.private_bytes(
                serialization.Encoding.PEM,
                serialization.PrivateFormat.PKCS8,
                serialization.NoEncryption(),
            )
            fd = os.open(key_path, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
            with os.fdopen(fd, "wb") as f:
                f.write(pem)
        self._key = key

    def sign(self, data: bytes) -> str:
        return base64.b64encode(self._key.sign(data)).decode("ascii")

    def public_key_b64(self) -> str:
        raw = self._key.public_key().public_bytes(
            serialization.Encoding.Raw, serialization.PublicFormat.Raw
        )
        return base64.b64encode(raw).decode("ascii")
