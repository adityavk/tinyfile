#!/bin/bash

for filename in "$2"/*; do
    if [ -d "$filename" ]; then
        continue
    fi
    OUT=`"$1" --state ASYNC --file "$filename"`
    echo "$filename","$OUT"
done