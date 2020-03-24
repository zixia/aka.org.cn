#!/usr/bin/env bash

set -eo pipefail

if [ -z "$1" ]; then
  >&2 echo "Need a file as arg"
  exit 1
fi

echo "Converting gbk to utf8: $1"

iconv -f GBK -t UTF-8 < "$1" > "$1.bak"
mv "$1.bak" "$1"
