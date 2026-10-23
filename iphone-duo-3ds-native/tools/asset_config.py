"""Portable asset input conventions; no machine-specific source paths."""
import os
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def asset_dir(name):
    return Path(os.environ.get('DUO_ASSET_ROOT',str(ROOT/'asset-input')))/name
