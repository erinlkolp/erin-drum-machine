#!/usr/bin/env python3
"""Strip trailing commas from JUCE-generated moduleinfo.json so Bitwig can parse it."""
import json
import re
import sys

path = sys.argv[1]
with open(path) as f:
    text = f.read()
fixed = re.sub(r",\s*([}\]])", r"\1", text)
with open(path, "w") as f:
    json.dump(json.loads(fixed), f, indent=2)
