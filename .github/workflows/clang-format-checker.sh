#!/bin/bash
# clang-format-checker.sh

# Files are passed as arguments to the script
FILES="$@"

echo "clang-format version: $(clang-format --version)"
FORMATTING_ISSUES=0
for FILE in $FILES; do
  DIFF_OUTPUT=$(clang-format -style=file "$FILE" | diff "$FILE" -)
  if [ -z "$DIFF_OUTPUT" ]; then
    echo "::notice::OK: $FILE is properly formatted."
  else
    echo "::error file=$FILE::NOT OK: $FILE needs formatting."
    echo "Diff for $FILE:"
    echo "$DIFF_OUTPUT"
    FORMATTING_ISSUES=1
  fi
done

# Exit with 1 if there were formatting issues.
if [ "$FORMATTING_ISSUES" -ne 0 ]; then
  exit 1
fi

