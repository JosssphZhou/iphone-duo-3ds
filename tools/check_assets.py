#!/usr/bin/env python3
"""Check local visual resources against the release manifest and native atlases."""
import argparse
from pathlib import Path
import shlex
import sys

from import_assets import ROOT, load_manifest, matches, safe_path


def check_assets(root, manifest):
    entries = load_manifest(manifest)
    indexed = {entry["target"]: entry for entry in entries}
    issues = []
    for entry in entries:
        path = safe_path(root, entry["target"])
        if not matches(path, entry):
            issues.append(f"Missing or mismatched: {entry['target']}")
    gfx = safe_path(root, "iphone-duo-3ds-native/gfx")
    atlases = sorted(gfx.glob("*.t3s"))
    if not atlases:
        issues.append("No native atlas definitions found; copy the source tree first.")
    for atlas in atlases:
        if atlas.is_symlink():
            raise ValueError(f"Symlink atlas is not allowed: {atlas}")
        for line in atlas.read_text(encoding="utf-8").splitlines():
            tokens = shlex.split(line, comments=True)
            if not tokens or tokens[0].startswith("-"):
                continue
            if len(tokens) != 1:
                raise ValueError(f"Unexpected atlas entry in {atlas.name}: {line}")
            image = safe_path(gfx, tokens[0])
            target = image.relative_to(Path(root).resolve()).as_posix()
            if target not in indexed:
                issues.append(f"Atlas {atlas.name} requires an unlisted asset: {target}")
    return issues


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--manifest", type=Path)
    args = parser.parse_args()
    try:
        issues = check_assets(args.root, args.manifest or args.root / "asset-manifest.json")
    except (ValueError, OSError, KeyError, TypeError) as error:
        print(f"Asset check stopped: {error}", file=sys.stderr)
        return 2
    if issues:
        print(f"Local visual assets are not ready ({len(issues)} issues).", file=sys.stderr)
        for issue in issues[:20]:
            print(f"  {issue}", file=sys.stderr)
        if len(issues) > 20:
            print(f"  ... and {len(issues) - 20} more.", file=sys.stderr)
        print("Bundled artwork is missing or modified. Restore the listed files from Git, or see README.md for the optional import workflow.", file=sys.stderr)
        return 2
    print("All manifest assets and native atlas references are ready.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
