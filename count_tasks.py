Import("env")
import re, glob, os

count = 0
for d in ("src", "include"):
    pattern = os.path.join(env["PROJECT_DIR"], d, "**", "*")
    for path in glob.glob(pattern, recursive=True):
        if os.path.basename(path) in ("scheduler.h", "scheduler.cpp"):
            continue
        if path.endswith((".cpp", ".h", ".ino")):
            with open(path, encoding="utf-8", errors="ignore") as f:
                text = f.read()
            text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)  # block comments
            text = re.sub(r"//.*", "", text)                    # line comments
            count += len(re.findall(r"\baddTask\s*\(", text))

env.Append(CPPDEFINES=[("SCHEDULER_SIZE", max(count, 1))])