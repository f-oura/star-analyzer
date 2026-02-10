#!/bin/bash
# Generate joblist XML from mainconf and analysis info.
# Usage: ./script/generate_joblist.sh MAINCONF_PATH
# Example: ./script/generate_joblist.sh config/mainconf/main_lambda.yaml

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if [ -z "${1:-}" ]; then
    echo "Usage: $0 MAINCONF_PATH" >&2
    echo "  MAINCONF_PATH  e.g. config/mainconf/main_lambda.yaml" >&2
    echo "  Output: job/joblist/joblist_<runMacro>.xml (e.g. joblist_run_ana_Lambda.xml, from analysis info runMacro)" >&2
    exit 1
fi

cd "$PROJECT_ROOT" && python script/analysis_info_helper.py --generate-joblist "$1" || exit 1
