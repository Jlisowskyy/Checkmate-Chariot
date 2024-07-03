#!/bin/bash

# Run the command in the background and get its PID
SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
PID=$$
LOGFILE="gdb_output_${PID}_$(date +%Y%m%d%H%M%S).log"

EXECUTABLE=$1
shift
ARGS=$*

unset DEBUGINFOD_URLS

cd "${SCRIPT_DIR}" || exit 1

# Create a GDB script with useful commands
cat <<EOF > gdb_commands.txt
# Enable pagination to avoid pausing output
set pagination off

# Display backtrace if program crashes
handle SIGABRT stop
define hook-stop
    thread apply all bt
end

# Log to given file
set logging on
set logging file $LOGFILE

# Run the executable with arguments
run $ARGS

# Exit GDB after the program finishes
quit

EOF

# Start GDB with the executable and the script
gdb -x gdb_commands.txt --args "$EXECUTABLE" "$ARGS"