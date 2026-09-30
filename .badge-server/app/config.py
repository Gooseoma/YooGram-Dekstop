import os
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Settings:
    admin_password: str
    session_secret: str
    data_dir: Path
    cookie_secure: bool = True

    @property
    def db_path(self) -> Path:
        return self.data_dir / "badges.db"

    @property
    def key_path(self) -> Path:
        return self.data_dir / "signing_key.pem"


def load_settings() -> Settings:
    password = os.environ.get("BADGE_ADMIN_PASSWORD", "")
    secret = os.environ.get("BADGE_SESSION_SECRET", "")
    if len(password) < 12:
        raise RuntimeError("BADGE_ADMIN_PASSWORD must be at least 12 characters")
    if len(secret) < 32:
        raise RuntimeError("BADGE_SESSION_SECRET must be at least 32 characters")
    return Settings(
        admin_password=password,
        session_secret=secret,
        data_dir=Path(os.environ.get("BADGE_DATA_DIR", "./data")),
        cookie_secure=os.environ.get("BADGE_COOKIE_SECURE", "1") != "0",
    )
