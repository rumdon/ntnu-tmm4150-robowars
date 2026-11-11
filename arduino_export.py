import os, shutil
import os, shutil
from datetime import datetime

PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))
now = datetime.now()
minutes_into_year = int((now - datetime(now.year, 1, 1)).total_seconds() // 60)
TIMESTAMP = f"{now.strftime('%y')}_{minutes_into_year}"
SKETCH_NAME = "arduino_sketch_" + TIMESTAMP
OUT_DIR = os.path.join(PROJECT_DIR, "export", SKETCH_NAME)
SOURCES = ("src", "include")
EXTENSIONS = (".cpp", ".h", ".hpp", ".c", ".ino")

if os.path.isdir(OUT_DIR):
    shutil.rmtree(OUT_DIR)
os.makedirs(OUT_DIR)

for d in SOURCES:
    src_dir = os.path.join(PROJECT_DIR, d)
    if not os.path.isdir(src_dir):
        continue
    for root, _, files in os.walk(src_dir):
        for name in files:
            if not name.endswith(EXTENSIONS):
                continue
            dest_name = name
            if d == "src" and name == "main.cpp":
                dest_name = SKETCH_NAME + ".ino"
            dest = os.path.join(OUT_DIR, dest_name)
            if os.path.exists(dest):
                print(f"WARNING: name clash, overwriting {dest_name}")
            shutil.copy2(os.path.join(root, name), dest)
            print(f"{os.path.relpath(os.path.join(root, name), PROJECT_DIR)} -> {dest_name}")

print(f"\nDone: {OUT_DIR}")