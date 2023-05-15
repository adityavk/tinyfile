#!/bin/bash

if [ -n "$4" ]; then
    OUTDIR="$4"
else
    OUTDIR="output"
fi

for filename in "$2"/*; do
    if [ -d "$filename" ]; then
        continue
    fi
    echo $OUTDIR
    "$1" --state SYNC --file "$filename" --out_dir "$OUTDIR"
    "$1" --state ASYNC --file "$filename" --out_dir "$OUTDIR"
    "$1" --state SYNC --file "$filename" --no_service --sms_size "$3" --out_dir "$OUTDIR"
done

for filename in "$2"/cantrbry/*; do
    echo $OUTDIR
    "$1" --state SYNC --file "$filename" --out_dir "$OUTDIR"
    "$1" --state ASYNC --file "$filename" --out_dir "$OUTDIR"
    "$1" --state SYNC --file "$filename" --no_service --sms_size "$3" --out_dir "$OUTDIR"
done