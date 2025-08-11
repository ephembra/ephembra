#!/usr/bin/env python3

import os, sys, argparse, urllib.request

parser = argparse.ArgumentParser(description="download file from URL")
parser.add_argument("url", help="URL to download")
parser.add_argument("path", help="output file path")
args = parser.parse_args()
os.makedirs(os.path.dirname(args.path), exist_ok=True)
if not os.path.exists(args.path):
    print(f"fetching {args.url}")
    urllib.request.urlretrieve(args.url, args.path)
    print(f"{args.path} written ({os.stat(args.path).st_size} bytes)")
else:
    print(f"{args.path} exists ({os.stat(args.path).st_size} bytes)")
