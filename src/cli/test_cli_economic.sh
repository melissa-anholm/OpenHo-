#!/bin/sh
set -eu

CLI="$1"
ASSETS="$2"

output="$({
    printf '%s\n' \
        'set spending 0.30 0.20 0.50' \
        'set research 1 0 0 0 0 0' \
        'set planet 7 1.0 0.60 0.40' \
        'dip 500 persistent' \
        'abandon 7' \
        'cancel-abandon 7' \
        'ready' \
        'submit' \
        'ready' \
        'process' \
        'status' \
        'quit'
} | "$CLI" --assets "$ASSETS" 2>&1)"

printf '%s\n' "$output"
printf '%s\n' "$output" | grep -F "Error: ready: submit pending economic choices first" >/dev/null
printf '%s\n' "$output" | grep -F "Economic submission accepted for the current turn." >/dev/null
printf '%s\n' "$output" | grep -F "Turn processed and autosaved." >/dev/null
printf '%s\n' "$output" | grep -F "Turn 1, year 2010" >/dev/null
