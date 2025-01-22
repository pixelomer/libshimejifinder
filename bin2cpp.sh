#!/usr/bin/env bash

var="$(basename "${2%.*}")"
echo "#include <cstddef>" > "$2"
echo "static const char ${var}[] = " >> "$2"
hexdump -v -e '16/1 "_x%02X" "\n"' < "$1" | sed 's/_/\\/g; s/\\x  //g; s/.*/    "&"/' >> "$2"
echo ";" >> "$2"
echo -n "static const size_t ${var}_len = " >> "$2"
wc -c < "$1" >> "$2"
echo ";" >> "$2"