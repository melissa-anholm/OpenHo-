#!/bin/sh
set -eu

CLI="$1"
ASSETS="$2"

output="$({
    printf '%s\n' \
        'players' \
        'play_as Joe 2' \
        'players' \
        'play_as Ann 3' \
        'quit'
} | "$CLI" --assets "$ASSETS" --players 4 --planets 12 --humans 2 2>&1)"

printf '%s\n' "$output"
printf '%s\n' "$output" | grep -F 'type=human, ready=no, eliminated=no' >/dev/null
printf '%s\n' "$output" | grep -F 'type=AI, ready=yes, eliminated=no' >/dev/null
printf '%s\n' "$output" | grep -F 'Now playing as Joe 2.' >/dev/null
printf '%s\n' "$output" | grep -F '* Player 2: Joe 2' >/dev/null
printf '%s\n' "$output" | grep -F 'cannot play as an AI player' >/dev/null
