#!/bin/bash
# Run ana_Lambda.C - Lambda (V0 p+ pi-) analysis with StLambdaMaker
# Usage: Run from project root: ./script/run_ana_Lambda.sh
#        ./script/run_ana_Lambda.sh [inputFile] [outputFile] [jobid] [nEvents] [configPath]
# Default: auau19 list, lambda_auau19 output, main_lambda.yaml

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT" || exit 1

source "$SCRIPT_DIR/setup.sh"
export LD_LIBRARY_PATH="$PROJECT_ROOT/lib:$LD_LIBRARY_PATH"

INPUT_FILE="${1:-config/picoDstList/auau19GeV_lambda.list}"
OUTPUT_FILE="${2:-rootfile/lambda_auau19/lambda_auau19_ana_Lambda.root}"
JOBID="${3:-0}"
NEVENTS="${4:--1}"
CONFIG_PATH="${5:-}"

mkdir -p "$(dirname "$OUTPUT_FILE")"

echo "=== ana_Lambda.C ==="
echo "Input:   $INPUT_FILE"
echo "Output:  $OUTPUT_FILE"
echo "JobID:   $JOBID"
echo "nEvents: $NEVENTS"
echo "Config:  ${CONFIG_PATH:-config/mainconf/main_lambda.yaml (default)}"
echo "================================"

if [ -n "$CONFIG_PATH" ]; then
  root4star -b -q "analysis/run_ana_Lambda.C(\"$INPUT_FILE\",\"$OUTPUT_FILE\",\"$JOBID\",$NEVENTS,\"$CONFIG_PATH\")"
else
  root4star -b -q "analysis/run_ana_Lambda.C(\"$INPUT_FILE\",\"$OUTPUT_FILE\",\"$JOBID\",$NEVENTS)"
fi
