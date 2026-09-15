#!/usr/bin/env bash
set -euo pipefail

elf=${1:?usage: check_eeprom_layout.sh path/to/hr20.elf}
nm_bin=${AVR_NM:-avr-nm}

symbol_addr() {
    local symbol=$1
    local raw
    raw=$($nm_bin -n "$elf" | awk -v symbol="$symbol" '$3 == symbol {print $1; exit}')
    if [[ -z "$raw" ]]; then
        echo "missing EEPROM symbol: $symbol" >&2
        return 1
    fi
    printf '%04x' $(( (16#$raw) & 0xffff ))
}

check_symbol() {
    local symbol=$1 expected=$2 actual
    actual=$(symbol_addr "$symbol")
    if [[ "$actual" != "$expected" ]]; then
        echo "EEPROM layout mismatch: $symbol expected 0x$expected, got 0x$actual" >&2
        return 1
    fi
    echo "EEPROM $symbol = 0x$actual"
}

check_symbol ee_layout       0003
check_symbol ee_timers       0004
check_symbol ee_reserved2_60 0084
check_symbol ee_config       00c0

echo "EEPROM layout OK"
