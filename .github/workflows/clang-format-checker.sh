#!/bin/bash
FILES=$(cat $GITHUB_ENV | grep 'cpp_files<<EOF' -A 1 | tail -n 1)
FORMATTING_ISSUES=0
for FILE in $FILES; do
  DIFF_OUTPUT=$(clang-format -style=file "$FILE" | diff "$FILE" -)
  if [ -z "$DIFF_OUTPUT" ]; then
    echo "::notice::OK: $FILE is properly formatted."
  else
    echo "::error file=$FILE::NOT OK: $FILE needs formatting."
    echo "$DIFF_OUTPUT"
    FORMATTING_ISSUES=1
  fi
done
if [ "$FORMATTING_ISSUES" -ne 0 ]; then
  exit 1
fi
