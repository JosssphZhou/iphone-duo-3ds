"""Security and data-preservation checks for the local asset importer."""
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from import_assets import import_assets
from check_assets import check_assets


class AssetImportTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.base = Path(self.temp.name)
        self.root = self.base / "repo"
        self.source = self.base / "native"
        self.root.mkdir()
        (self.source / "gfx").mkdir(parents=True)
        (self.source / "gfx/tile.png").write_bytes(b"known visual content")
        (self.source / "layout.bin").write_bytes(b"private save")
        self.data = (self.source / "gfx/tile.png").read_bytes()
        self.entry = {"source": "native", "path": "gfx/tile.png",
                      "target": "iphone-duo-3ds-native/gfx/tile.png",
                      "sha256": hashlib.sha256(self.data).hexdigest(), "size": len(self.data)}
        self.manifest = self.root / "asset-manifest.json"
        self.write_manifest([self.entry])

    def tearDown(self):
        self.temp.cleanup()

    def write_manifest(self, entries):
        self.manifest.write_text(json.dumps({"version": 1, "files": entries}))

    def run_import(self, replace=False):
        return import_assets(self.root, self.manifest, {"native": self.source}, replace)

    def test_import_is_idempotent_and_never_copies_save(self):
        self.assertEqual(self.run_import()["imported"], 1)
        self.assertEqual(self.run_import()["unchanged"], 1)
        self.assertFalse(any(self.root.rglob("layout.bin")))
        self.assertEqual((self.source / "layout.bin").read_bytes(), b"private save")

    def test_hash_failure_prevents_all_writes(self):
        bad = dict(self.entry, path="gfx/missing.png", target="iphone-duo-3ds-native/gfx/missing.png")
        self.write_manifest([self.entry, bad])
        with self.assertRaises(ValueError):
            self.run_import()
        self.assertFalse((self.root / self.entry["target"]).exists())

    def test_existing_file_requires_replace_and_is_backed_up(self):
        dest = self.root / self.entry["target"]
        dest.parent.mkdir(parents=True)
        dest.write_bytes(b"existing artwork")
        with self.assertRaises(ValueError):
            self.run_import()
        self.assertEqual(dest.read_bytes(), b"existing artwork")
        result = self.run_import(replace=True)
        self.assertEqual(dest.read_bytes(), self.data)
        self.assertEqual((Path(result["backup"]) / self.entry["target"]).read_bytes(), b"existing artwork")

    def test_rejects_traversal_mismatched_target_and_private_files(self):
        for change in [
            {"path": "gfx/../../escape.png"},
            {"path": "/tmp/escape.png"},
            {"target": "../escape.png"},
            {"path": "layout.bin", "target": "iphone-duo-3ds-native/layout.bin"},
            {"path": "gfx/a.py", "target": "iphone-duo-3ds-native/gfx/a.py"},
            {"target": "iphone-duo-3ds-native/source/main.c"},
        ]:
            with self.subTest(change=change):
                self.write_manifest([dict(self.entry, **change)])
                with self.assertRaises(ValueError):
                    self.run_import()

    def test_rejects_source_and_destination_symlinks(self):
        original = self.source / "gfx/tile.png"
        original.rename(self.source / "real.png")
        original.symlink_to(self.source / "real.png")
        with self.assertRaises(ValueError):
            self.run_import()
        original.unlink()
        original.write_bytes(self.data)
        dest = self.root / "iphone-duo-3ds-native"
        dest.symlink_to(self.source, target_is_directory=True)
        with self.assertRaises(ValueError):
            self.run_import()

    def test_symbols_carrier_is_explicitly_allowed_but_other_js_is_not(self):
        web = self.base / "web"
        symbols = web / "assets/sf-symbols/symbols-data.js"
        symbols.parent.mkdir(parents=True)
        symbols.write_bytes(b"const symbols = {};")
        entry = {"source": "web", "path": "assets/sf-symbols/symbols-data.js",
                 "target": "iphone-duo-3ds-prototype/assets/sf-symbols/symbols-data.js",
                 "sha256": hashlib.sha256(symbols.read_bytes()).hexdigest(), "size": symbols.stat().st_size}
        self.write_manifest([entry])
        self.assertEqual(import_assets(self.root, self.manifest, {"web": web})["imported"], 1)
        self.write_manifest([dict(entry, path="assets/other.js", target="iphone-duo-3ds-prototype/assets/other.js")])
        with self.assertRaises(ValueError):
            import_assets(self.root, self.manifest, {"web": web})

    def test_checker_reports_missing_and_unlisted_atlas_dependency(self):
        gfx = self.root / "iphone-duo-3ds-native/gfx"
        gfx.mkdir(parents=True)
        (gfx / "ui.t3s").write_text("--atlas\ntile.png\nextra.png\n")
        issues = check_assets(self.root, self.manifest)
        self.assertTrue(any("tile.png" in issue for issue in issues))
        self.assertTrue(any("unlisted" in issue for issue in issues))
        (gfx / "ui.t3s").write_text("--atlas\ntile.png\n")
        self.run_import()
        self.assertEqual(check_assets(self.root, self.manifest), [])


if __name__ == "__main__":
    unittest.main()
