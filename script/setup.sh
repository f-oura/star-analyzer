#!/bin/bash
# Source from project root: source script/setup.sh
# Sets MAINCONF default and runs starver with libraryTag from analysis info (mainconf -> analysis YAML).
# Export MAINCONF to use another mainconf (e.g. MAINCONF=config/mainconf/main_phi.yaml source script/setup.sh).

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
MAINCONF="${MAINCONF:-config/mainconf/main_lambda.yaml}"

LIBRARY_TAG=$(cd "$PROJECT_ROOT" && python script/analysis_info_helper.py --library-tag --mainconf "$MAINCONF") || exit 1
starver "$LIBRARY_TAG"
