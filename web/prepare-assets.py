#!/usr/bin/env python3
"""Generate a compact catalogue from Fayawo's ORIGINAL car configuration assets.
No invented cars or substitute assets. Rendering mesh conversion comes later.
"""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "data" / "carsim" / "normal" / "cars"
OUTPUT = ROOT / "site" / "assets" / "cars"
OUTPUT.mkdir(parents=True, exist_ok=True)

entries = []
for config in sorted(SOURCE.glob("*.car")):
    data = config.read_bytes()
    # Car configs are text source files. Keep original file bytes unchanged.
    (OUTPUT / config.name).write_bytes(data)
    entries.append({
        "id": config.stem,
        "config": f"assets/cars/{config.name}",
        "bytes": len(data),
        "mesh_directory": f"data/cars/{config.stem}",
    })

manifest = {
    "source": "data/carsim/normal/cars",
    "asset_type": "original VDrift car configuration",
    "count": len(entries),
    "cars": entries,
}
TIRES = ROOT / "data" / "carsim" / "normal" / "tires"
TIRE_OUTPUT = OUTPUT.parent / "tires"
TIRE_OUTPUT.mkdir(parents=True, exist_ok=True)
tire_names = []
for tire in sorted(TIRES.glob("*.tire")):
    (TIRE_OUTPUT / tire.name).write_bytes(tire.read_bytes())
    tire_names.append(tire.name)
manifest["tires"] = tire_names
(OUTPUT.parent / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
if not tire_names:
    raise SystemExit("No original Fayawo tire files found")
if not entries:
    raise SystemExit("No original Fayawo car configuration files found")
print(f"Staged {len(entries)} original .car configurations")
