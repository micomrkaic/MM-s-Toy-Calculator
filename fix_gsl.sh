#!/bin/bash

GSL_DIR="/opt/homebrew/Cellar/gsl/2.8/include/gsl"

echo "Scanning for invalid UTF-8 files in: $GSL_DIR"

find "$GSL_DIR" -type f -name "*.h" | while read -r file; do
  # Check for invalid UTF-8 using iconv
  if ! iconv -f UTF-8 -t UTF-8 "$file" >/dev/null 2>&1; then
    echo "Fixing: $file"
    # Convert from ISO-8859-1 (Latin-1) to UTF-8
    iconv -f ISO-8859-1 -t UTF-8 "$file" -o "${file}.fixed" && mv "${file}.fixed" "$file"
  fi
done

echo "Done. All headers are now UTF-8 encoded."
