#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=__build
OUT=SYMBOLS.md

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j

"$BUILD_DIR/queue_bench" "$@"

# Extract disassembly without raw bytes or color for clean markdown
OBJDUMP="${OBJDUMP:-objdump}"
DISASM_C=$($OBJDUMP -d -C --no-show-raw-insn "$BUILD_DIR/libsymbols_c.a")
DISASM_CPP=$($OBJDUMP -d -C --no-show-raw-insn "$BUILD_DIR/libsymbols_cpp.a")

# Function pairs to compare: "label|ref_symbol|new_symbol"
PAIRS=(
    "try_prepare_push|ref_spsc_try_prepare_push|new_spsc_try_prepare_push"
    "prepare_push|ref_spsc_prepare_push|new_spsc_prepare_push"
    "commit_push|ref_spsc_commit_push|new_spsc_commit_push"
    "try_prepare_consume|ref_spsc_try_prepare_consume|new_spsc_try_prepare_consume"
    "prepare_consume|ref_spsc_prepare_consume|new_spsc_prepare_consume"
    "commit_consume|ref_spsc_commit_consume|new_spsc_commit_consume"
)

extract_func() {
    local disasm="$1"
    local sym="$2"
    # Match from "<sym>:" until the next blank line (function separator)
    echo "$disasm" | sed -n "/^[0-9a-f]* <${sym}>:/,/^$/p" | head -n -1
}

{
    echo "# Symbol Comparison: queue.h (ref) vs queuepp.hpp (new)"
    echo ""

    for pair in "${PAIRS[@]}"; do
        IFS='|' read -r label ref_sym new_sym <<< "$pair"

        echo "## ${label}"
        echo ""
        echo "### ref (queue.h)"
        echo ""
        echo '```asm'
        extract_func "$DISASM_C" "$ref_sym"
        echo '```'
        echo ""
        echo "### new (queuepp.hpp)"
        echo ""
        echo '```asm'
        extract_func "$DISASM_CPP" "$new_sym"
        echo '```'
        echo ""
    done
} > "$OUT"

echo "Wrote $OUT"
