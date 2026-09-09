#!/usr/bin/env bash

file="$1"
width="$2"
height="$3"
chafa -f sixel -s "${width}x${height}" "$file"
