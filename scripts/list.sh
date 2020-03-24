#!/usr/bin/env bash

set -eo pipefail

find . \
  -type f \
  \
  -name '*.htm' \
  -o -name '*.HTM' \
  -o -name '*.html' \
  -o -name '*.HTML' \
  -o -name '*.txt' \
  -o -name '*.TXT' \
