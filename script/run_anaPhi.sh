#!/bin/bash
# Run anaPhi.C - phi analysis with StPhiMaker
# Usage: Run from project root: ./script/run_anaPhi.sh
#        ./script/run_anaPhi.sh [inputFile] [outputFile] [jobid] [nEvents] [configPath]
# Default: main_auau19_anaPhi.yaml

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT" || exit 1

source "$SCRIPT_DIR/setup.sh" config/mainconf/main_auau19_anaPhi.yaml
export LD_LIBRARY_PATH="$PROJECT_ROOT/lib:$LD_LIBRARY_PATH"

INPUT_FILE="${1:-config/picoDstList/auau19GeV.list}"
OUTPUT_FILE="${2:-rootfile/auau19_anaPhi_temp/auau19_anaPhi_temp.root}"
JOBID="${3:-0}"
NEVENTS="${4:--1}"
CONFIG_PATH="${5:-}"

mkdir -p "$(dirname "$OUTPUT_FILE")"

echo "=== anaPhi.C ==="
echo "Input:   $INPUT_FILE"
echo "Output:  $OUTPUT_FILE"
echo "JobID:   $JOBID"
echo "nEvents: $NEVENTS"
echo "Config:  ${CONFIG_PATH:-config/mainconf/main_auau19_anaPhi.yaml (default)}"
echo "================================"

if [ -n "$CONFIG_PATH" ]; then
  root4star -b -q "analysis/run_anaPhi.C(\"$INPUT_FILE\",\"$OUTPUT_FILE\",\"$JOBID\",$NEVENTS,\"$CONFIG_PATH\")"
else
  root4star -b -q "analysis/run_anaPhi.C(\"$INPUT_FILE\",\"$OUTPUT_FILE\",\"$JOBID\",$NEVENTS)"
fi
