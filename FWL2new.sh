#!/bin/bash

mkdir -p new

for FILE in "$@"
do
    OUT=new/$(basename "$FILE")
    zcat "$FILE" | grep -ve "^#" | grep . | head -n2 | gzip > $OUT
    echo $OUT
done
