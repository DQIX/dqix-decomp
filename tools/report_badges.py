#!/usr/bin/env python3

import argparse
import json
from pathlib import Path

COLOR_THRESHOLDS = [(20, "red"), (40, "orange"), (60, "yellow"), (80, "yellowgreen"), (100, "green")]


def color_for(percent: float) -> str:
    for threshold, color in COLOR_THRESHOLDS:
        if percent < threshold:
            return color
    return "brightgreen"


def badge(label: str, percent: float) -> dict:
    return {
        "schemaVersion": 1,
        "label": label,
        "message": f"{percent:.2f}%",
        "color": color_for(percent),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Turn an objdiff report into shields.io endpoint badges")
    parser.add_argument("report", type=Path, help="path to report.json produced by 'ninja'")
    parser.add_argument("-o", "--out-dir", type=Path, help="write functions.json and bytes.json here")
    parser.add_argument("-l", "--label-prefix", default="", help="prefix each badge label, e.g. USA")
    args = parser.parse_args()

    prefix = f"{args.label_prefix} " if args.label_prefix else ""
    measures = json.loads(args.report.read_text())["measures"]
    badges = {
        "functions": badge(f"{prefix}functions", measures["matched_functions_percent"]),
        "bytes": badge(f"{prefix}bytes", measures["matched_code_percent"]),
    }

    if args.out_dir is not None:
        args.out_dir.mkdir(parents=True, exist_ok=True)
        for name, data in badges.items():
            (args.out_dir / f"{name}.json").write_text(json.dumps(data) + "\n")

    print(
        f"functions  {badges['functions']['message']:>8}"
        f"  ({measures['matched_functions']}/{measures['total_functions']})"
    )
    print(
        f"bytes      {badges['bytes']['message']:>8}"
        f"  ({int(measures['matched_code'])}/{int(measures['total_code'])})"
    )


if __name__ == "__main__":
    main()
