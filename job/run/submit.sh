#!/bin/bash
# Submit from job/run/. Replaces __PROJECT_ROOT__ in the XML with the project root.
# Usage: cd job/run && ./submit.sh [joblist.xml]
# Example: ./submit.sh ../joblist/joblist_run_anaLambda.xml

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

"$SCRIPT_DIR/setup_symlinks.sh"

PROJECT_ROOT="$(cd .. && cd .. && pwd)"
TEMPLATE="${1:-../joblist/joblist_run_anaLambda.xml}"
OUTPUT="$(basename "$TEMPLATE")"

if [[ ! -f "$TEMPLATE" ]]; then
  echo "ERROR: Template not found: $TEMPLATE" >&2
  exit 1
fi

sed "s|__PROJECT_ROOT__|$PROJECT_ROOT|g" "$TEMPLATE" > "$OUTPUT"
echo "Submitting with PROJECT_ROOT=$PROJECT_ROOT"
exec star-submit "$OUTPUT"
