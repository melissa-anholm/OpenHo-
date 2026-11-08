#!/bin/sh
set -eu

CLI="$1"
ASSETS="$2"

output="$({
    printf '%s\n' \
        'set_spending 0.30 0.20 0.50' \
        'set_savings 0.40' \
        'set_research_budget 0.30' \
        'set_planet_budget Colma 0.20' \
        'set_research 1 0 0 0 0 0' \
        'set_research_stream weapons 0.50' \
	        'research' \
	        'fleets' \
        'set_mining Colma 0.80' \
        'set_terraforming Colma 0.25' \
        'set_planet Colma 1.0 0.60 0.40' \
        'dip 500 persistent' \
        'abandon Colma' \
        'cancel_abandon Colma' \
        'ready' \
        'unready' \
        'dip 600 persistent' \
        'ready' \
        'process' \
        'status' \
        'quit'
} | "$CLI" --assets "$ASSETS" 2>&1)"

printf '%s\n' "$output"
printf '%s\n' "$output" | grep -F "Economic submission accepted for the current turn." >/dev/null
printf '%s\n' "$output" | grep -F "Pending spending allocation updated and normalized." >/dev/null
printf '%s\n' "$output" | grep -F "Pending planetary allocation updated and normalized." >/dev/null
printf '%s\n' "$output" | grep -F "Pending research-stream allocation updated and normalized." >/dev/null
printf '%s\n' "$output" | grep -F "Pending research-stream allocations:" >/dev/null
printf '%s\n' "$output" | grep -F "Queued fleet actions for turn 0:" >/dev/null
printf '%s\n' "$output" | grep -F "  scrap requests: (none)" >/dev/null
printf '%s\n' "$output" | grep -F "  weapons=0.5" >/dev/null
printf '%s\n' "$output" | grep -F "Pending planet development allocation updated and normalized." >/dev/null
test "$(printf '%s\n' "$output" | grep -c "Economic submission accepted for the current turn.")" -ge 2
printf '%s\n' "$output" | grep -F "Player 1 is unready." >/dev/null
printf '%s\n' "$output" | grep -F "Turn processed and autosaved." >/dev/null
printf '%s\n' "$output" | grep -F "Turn 1, year 2010" >/dev/null
