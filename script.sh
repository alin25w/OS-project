#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <filename.c>"
    exit 1
fi

filename=$1


gcc -Wall -o "buff.txt" "$filename" 2> buff.txt

    errors=$(grep -o 'error' buff.txt | wc -l)
    warnings=$(grep -o 'warning' buff.txt | wc -l)

    echo "$errors $warnings"

rm -f buff.txt

