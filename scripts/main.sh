#!/usr/bin/env bash

# set -eo pipefail

pushd docs/

IFS=$'\n'
for f in $(../scripts/list.sh)
do
  ../scripts/convert.sh "$f"
done

popd
