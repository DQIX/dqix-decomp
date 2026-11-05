import os
import subprocess
import sys

output, command = sys.argv[1], sys.argv[2:]
result = subprocess.run(command)
if result.returncode == 0:
    os.makedirs(os.path.dirname(output) or ".", exist_ok=True)
    with open(output, "a"):
        pass
    os.utime(output)
sys.exit(result.returncode)
