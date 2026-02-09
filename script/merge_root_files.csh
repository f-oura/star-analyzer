#!/bin/tcsh
# Script to merge ROOT files with the same JOBID
# Usage: merge_root_files.csh <path_to_root_file>
# Example: merge_root_files.csh /path/to/output_phi_analysis_12345_0.root
# This will merge all files matching: /path/to/output_phi_analysis_12345_*.root
# Output: /path/to/output_phi_analysis_12345_merge.root

# Check if argument is provided
if ( "$1" == "" ) then
    echo "ERROR: No input file specified"
    echo "Usage: merge_root_files.csh <path_to_root_file>"
    echo "Example: merge_root_files.csh /path/to/X_prefix_Y_JOBID_SUBID.root"
    exit 1
endif

set INPUT_FILE = "$1"

# Check if input file exists
if ( ! -f "$INPUT_FILE" ) then
    echo "ERROR: Input file does not exist: $INPUT_FILE"
    exit 1
endif

# Extract directory and filename
set FILE_DIR = `dirname "$INPUT_FILE"`
set FILE_NAME = `basename "$INPUT_FILE"`

# Check if file has .root extension
set FILE_EXT = `echo "$FILE_NAME" | sed 's/.*\.//'`
if ( "$FILE_EXT" != "root" ) then
    echo "ERROR: Input file must have .root extension: $INPUT_FILE"
    exit 1
endif

# Extract base pattern (X_..._Y_JOBID) by removing _SUBID.root
# Remove .root extension first
set FILE_BASE = `echo "$FILE_NAME" | sed 's/\.root$//'`
# Remove last underscore and everything after it (this is SUBID)
set PATTERN_BASE = `echo "$FILE_BASE" | sed 's/_[^_]*$//'`

if ( "$PATTERN_BASE" == "" ) then
    echo "ERROR: Could not extract pattern from filename: $FILE_NAME"
    echo "Expected format: X_..._Y_JOBID_SUBID.root"
    exit 1
endif

# Construct search pattern and output filename
set SEARCH_PATTERN = "${PATTERN_BASE}_*.root"
set OUTPUT_FILE = "${FILE_DIR}/${PATTERN_BASE}_merge.root"

echo "=========================================="
echo "Merging ROOT files"
echo "Input pattern: ${FILE_DIR}/${SEARCH_PATTERN}"
echo "Output file: $OUTPUT_FILE"
echo "=========================================="

# Find all matching files
set MATCHING_FILES = `find "$FILE_DIR" -maxdepth 1 -name "$SEARCH_PATTERN" -type f | sort`

# Check if any files found
if ( "$MATCHING_FILES" == "" ) then
    echo "ERROR: No files found matching pattern: ${FILE_DIR}/${SEARCH_PATTERN}"
    exit 1
endif

# Count files
set FILE_COUNT = `echo "$MATCHING_FILES" | wc -l`
echo "Found $FILE_COUNT file(s) to merge:"
foreach file ( $MATCHING_FILES )
    echo "  - $file"
end

# Check if output file already exists
if ( -f "$OUTPUT_FILE" ) then
    echo ""
    echo "WARNING: Output file already exists: $OUTPUT_FILE"
    echo "It will be overwritten."
    echo ""
endif

# Check if hadd command is available
which hadd >& /dev/null
if ( $status != 0 ) then
    echo "ERROR: hadd command not found. Please setup ROOT environment."
    exit 1
endif

# Merge files using hadd
echo ""
echo "Merging files..."
hadd -f "$OUTPUT_FILE" $MATCHING_FILES
set HADD_STATUS = $status

if ( $HADD_STATUS != 0 ) then
    echo "ERROR: hadd failed with status $HADD_STATUS"
    exit 1
endif

# Check if output file was created successfully
if ( ! -f "$OUTPUT_FILE" ) then
    echo "ERROR: Output file was not created: $OUTPUT_FILE"
    exit 1
endif

# Get file sizes
set OUTPUT_SIZE = `ls -lh "$OUTPUT_FILE" | awk '{print $5}'`
echo ""
echo "=========================================="
echo "Merge completed successfully!"
echo "Output file: $OUTPUT_FILE"
echo "Output size: $OUTPUT_SIZE"
echo "=========================================="

exit 0
