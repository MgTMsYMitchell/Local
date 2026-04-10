#!/usr/bin/env bash
# Rune System — Linux / Termux dependency check
# Usage:  bash scripts/check-deps.sh

set -euo pipefail

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}=== Rune System Dependency Check ===${NC}"
echo ""

all_ok=true

check() {
    local name="$1"; shift
    if out=$("$@" 2>&1 | head -1); then
        printf "${GREEN}[OK]      %-14s %s${NC}\n" "$name" "$out"
    else
        printf "${RED}[MISSING] %s${NC}\n" "$name" >&2
        all_ok=false
    fi
}

check git     git     --version
check cmake   cmake   --version
check ninja   ninja   --version
check node    node    --version
check npm     npm     --version
check curl    curl    --version
check sqlite3 sqlite3 --version
check zstd    zstd    --version

# C++ compiler
if command -v g++ &>/dev/null; then
    v=$(g++ --version 2>&1 | head -1)
    printf "${GREEN}[OK]      %-14s %s${NC}\n" "g++" "$v"
elif command -v clang++ &>/dev/null; then
    v=$(clang++ --version 2>&1 | head -1)
    printf "${GREEN}[OK]      %-14s %s${NC}\n" "clang++" "$v"
else
    printf "${RED}[MISSING] C++ compiler (g++ or clang++)${NC}\n" >&2
    all_ok=false
fi

echo ""
if $all_ok; then
    echo -e "${CYAN}All dependencies satisfied. Ready to build.${NC}"
else
    echo -e "${YELLOW}Some dependencies are missing.${NC}"
    echo ""
    echo "Install on Termux:"
    echo "  pkg install git cmake ninja nodejs sqlite zstd curl clang"
    echo ""
    echo "Install on Ubuntu/Debian:"
    echo "  apt install git cmake ninja-build nodejs npm sqlite3 zstd curl g++ make"
    exit 1
fi
