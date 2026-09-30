import re
import sqlite3
import time
from dataclasses import dataclass
from pathlib import Path

KINDS = ("emoji", "label")
_COLOR = re.compile(r"^#[0-9a-fA-F]{6}$")

MAX_EMOJI = 16
MAX_LABEL = 24
MAX_TOOLTIP = 64


class ValidationError(ValueError):
    pass


@dataclass(frozen=True)
class Grant:
    tg_id: int
    kind: str
    emoji: str
    label: str
    color: str
    tooltip: str
    expires_at: int | None
    created_at: int


def validate(tg_id, kind, emoji, label, color, tooltip) -> tuple:
    try:
        tg_id = int(str(tg_id).strip())
    except ValueError:
        raise ValidationError("ID must be a number")
    if tg_id <= 0:
        raise ValidationError("ID must be positive")
    if kind not in KINDS:
        raise ValidationError("Unknown badge kind")
    emoji, label, tooltip = emoji.strip(), label.strip(), tooltip.strip()
    color = color.strip() or "#2a9df4"
    if not _COLOR.match(color):
        raise ValidationError("Color must look like #rrggbb")
    if kind == "emoji":
        if not emoji:
            raise ValidationError("Emoji is required")
        label = ""
    else:
        if not label:
            raise ValidationError("Label text is required")
        emoji = ""
    if len(emoji) > MAX_EMOJI:
        raise ValidationError("Emoji is too long")
    if len(label) > MAX_LABEL:
        raise ValidationError(f"Label is limited to {MAX_LABEL} characters")
    if len(tooltip) > MAX_TOOLTIP:
        raise ValidationError(f"Tooltip is limited to {MAX_TOOLTIP} characters")
    return tg_id, kind, emoji, label, color.lower(), tooltip


class Store:
    def __init__(self, path: Path):
        path.parent.mkdir(parents=True, exist_ok=True)
        self._db = sqlite3.connect(path, check_same_thread=False, isolation_level=None)
        self._db.row_factory = sqlite3.Row
        self._db.executescript(
            """
            CREATE TABLE IF NOT EXISTS grants (
                tg_id INTEGER PRIMARY KEY,
                kind TEXT NOT NULL,
                emoji TEXT NOT NULL,
                label TEXT NOT NULL,
                color TEXT NOT NULL,
                tooltip TEXT NOT NULL,
                expires_at INTEGER,
                created_at INTEGER NOT NULL
            );
            CREATE TABLE IF NOT EXISTS meta (
                key TEXT PRIMARY KEY,
                value INTEGER NOT NULL
            );
            INSERT OR IGNORE INTO meta (key, value) VALUES ('version', 1);
            """
        )

    def _bump(self) -> None:
        self._db.execute("UPDATE meta SET value = value + 1 WHERE key = 'version'")

    def _purge_expired(self) -> None:
        cur = self._db.execute(
            "DELETE FROM grants WHERE expires_at IS NOT NULL AND expires_at <= ?",
            (int(time.time()),),
        )
        if cur.rowcount:
            self._bump()

    def version(self) -> int:
        self._purge_expired()
        return self._db.execute(
            "SELECT value FROM meta WHERE key = 'version'"
        ).fetchone()[0]

    def grant(self, tg_id, kind, emoji, label, color, tooltip, expires_at=None) -> None:
        self._db.execute(
            """
            INSERT INTO grants
                (tg_id, kind, emoji, label, color, tooltip, expires_at, created_at)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            ON CONFLICT(tg_id) DO UPDATE SET
                kind = excluded.kind, emoji = excluded.emoji,
                label = excluded.label, color = excluded.color,
                tooltip = excluded.tooltip, expires_at = excluded.expires_at
            """,
            (tg_id, kind, emoji, label, color, tooltip, expires_at, int(time.time())),
        )
        self._bump()

    def revoke(self, tg_id: int) -> bool:
        cur = self._db.execute("DELETE FROM grants WHERE tg_id = ?", (tg_id,))
        if cur.rowcount:
            self._bump()
        return bool(cur.rowcount)

    def all(self) -> list[Grant]:
        self._purge_expired()
        rows = self._db.execute("SELECT * FROM grants ORDER BY created_at DESC, tg_id")
        return [Grant(**dict(r)) for r in rows]
