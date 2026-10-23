#!/usr/bin/env python3
"""Import an existing, prepared local asset pack. Never downloads artwork."""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import stat
import sys
import tempfile
import uuid

ROOT = Path(__file__).resolve().parents[1]
PREFIXES = {"native": "iphone-duo-3ds-native", "web": "iphone-duo-3ds-prototype"}
IMAGE_SUFFIXES = {".png", ".jpg", ".jpeg", ".webp", ".gif", ".svg"}


def relative_path(value):
    if not isinstance(value, str) or not value or "\\" in value or "\0" in value:
        raise ValueError("Invalid asset path")
    path = PurePosixPath(value)
    if path.is_absolute() or any(part in ("", ".", "..") for part in value.split("/")):
        raise ValueError(f"Unsafe asset path: {value}")
    return path


def safe_path(root, relative):
    """Reject symlinks within a caller-selected root, including the root itself."""
    root = Path(root).expanduser().absolute()
    if root.is_symlink():
        raise ValueError(f"Symlink root is not allowed: {root}")
    root = root.resolve()
    path = root
    for part in relative_path(str(relative)).parts:
        path = path / part
        if path.is_symlink():
            raise ValueError(f"Symlink asset path is not allowed: {path}")
    if not path.resolve().is_relative_to(root):
        raise ValueError(f"Asset escapes its root: {relative}")
    return path


def load_manifest(path):
    raw = json.loads(Path(path).read_text(encoding="utf-8"))
    if raw.get("version") != 1 or not isinstance(raw.get("files"), list) or not raw["files"]:
        raise ValueError("Expected nonempty asset manifest version 1")
    entries, seen = [], set()
    for item in raw["files"]:
        source = item.get("source")
        if source not in PREFIXES:
            raise ValueError("Asset source must be native or web")
        rel = relative_path(item.get("path"))
        target = relative_path(item.get("target"))
        if target != PurePosixPath(PREFIXES[source]) / rel:
            raise ValueError(f"Asset target/source mismatch: {target}")
        allowed = ((source == "native" and (str(rel) == "icon.png" or
                    (rel.parts[0] == "gfx" and rel.suffix.lower() == ".png"))) or
                   (source == "web" and rel.parts[0] == "assets" and
                    (rel.suffix.lower() in IMAGE_SUFFIXES or
                     str(rel) == "assets/sf-symbols/symbols-data.js")))
        if not allowed:
            raise ValueError(f"File is not an allowed visual asset: {rel}")
        if str(target) in seen:
            raise ValueError(f"Duplicate asset target: {target}")
        if not re.fullmatch(r"[0-9a-f]{64}", item.get("sha256", "")):
            raise ValueError(f"Invalid SHA256 for {target}")
        if type(item.get("size")) is not int or item["size"] < 0:
            raise ValueError(f"Invalid byte size for {target}")
        seen.add(str(target))
        entries.append(item)
    return entries


def digest(path):
    flags = os.O_RDONLY | getattr(os, "O_NOFOLLOW", 0)
    fd = os.open(path, flags)
    with os.fdopen(fd, "rb") as stream:
        if not stat.S_ISREG(os.fstat(stream.fileno()).st_mode):
            raise ValueError(f"Not a regular asset file: {path}")
        result = hashlib.sha256()
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            result.update(block)
        return result.hexdigest()


def matches(path, entry):
    return path.is_file() and path.stat().st_size == entry["size"] and digest(path) == entry["sha256"]


def import_assets(root, manifest, sources, replace=False):
    entries = load_manifest(manifest)
    selected = [entry for entry in entries if sources.get(entry["source"]) is not None]
    if not selected:
        raise ValueError("Provide --native and/or --web with an existing prepared project directory")
    pending, unchanged = [], 0
    # Validate every selected input and every conflict before writing anything.
    for entry in selected:
        src = safe_path(sources[entry["source"]], entry["path"])
        dst = safe_path(root, entry["target"])
        if not matches(src, entry):
            raise ValueError(f"Missing or mismatched source asset: {src}")
        if dst.exists():
            if not dst.is_file():
                raise ValueError(f"Destination is not a file: {dst}")
            if matches(dst, entry):
                unchanged += 1
                continue
            if not replace:
                raise ValueError(f"Different destination already exists: {dst}; use --replace to back it up")
        pending.append((entry, src, dst))
    backup = None
    if any(dst.exists() for _, _, dst in pending):
        stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        backup = safe_path(root, f".asset-backups/{stamp}-{uuid.uuid4().hex[:8]}")
        backup.mkdir(parents=True, exist_ok=False)
        for entry, _, dst in pending:
            if dst.exists():
                saved = safe_path(backup, entry["target"])
                saved.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(dst, saved)
                if digest(saved) != digest(dst):
                    raise ValueError(f"Backup verification failed: {dst}")
    for entry, src, dst in pending:
        safe_path(root, entry["target"])
        dst.parent.mkdir(parents=True, exist_ok=True)
        descriptor, temp_name = tempfile.mkstemp(prefix=".asset-import-", dir=dst.parent)
        temp = Path(temp_name)
        try:
            with os.fdopen(descriptor, "wb") as output, open(src, "rb") as input_file:
                shutil.copyfileobj(input_file, output)
                output.flush()
                os.fsync(output.fileno())
            if not matches(temp, entry):
                raise ValueError(f"Source changed while importing: {src}")
            safe_path(root, entry["target"])
            os.replace(temp, dst)
            if not matches(dst, entry):
                raise ValueError(f"Imported asset verification failed: {dst}")
        finally:
            temp.unlink(missing_ok=True)
    return {"imported": len(pending), "unchanged": unchanged, "backup": str(backup) if backup else None}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--native", type=Path, help="Prepared native project directory containing gfx/")
    parser.add_argument("--web", type=Path, help="Prepared browser project directory containing assets/")
    parser.add_argument("--root", type=Path, default=ROOT, help="Destination repository root")
    parser.add_argument("--manifest", type=Path, help="Manifest; defaults to ROOT/asset-manifest.json")
    parser.add_argument("--replace", action="store_true", help="Back up and replace differing existing assets")
    args = parser.parse_args()
    try:
        result = import_assets(args.root, args.manifest or args.root / "asset-manifest.json",
                               {"native": args.native, "web": args.web}, args.replace)
    except (ValueError, OSError, KeyError, TypeError) as error:
        print(f"Asset import stopped: {error}", file=sys.stderr)
        return 2
    print(f"Imported {result['imported']} assets; {result['unchanged']} already matched.")
    if result["backup"]:
        print(f"Previous files backed up at: {result['backup']}")
    print("Run python3 tools/check_assets.py to check the complete local asset set.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
