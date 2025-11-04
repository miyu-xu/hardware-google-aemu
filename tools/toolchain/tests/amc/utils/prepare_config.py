import argparse
import os
from aemu.util import find_aosp_root

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--template", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    with open(args.template, 'r') as f:
        content = f.read()

    aosp_root = find_aosp_root()
    content = content.replace("%AOSP_ROOT%", aosp_root)

    with open(args.output, 'w') as f:
        f.write(content)

if __name__ == "__main__":
    main()
