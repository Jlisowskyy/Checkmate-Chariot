#!/bin/bash
grep check-pythonappend $1.i | sort | uniq >i.tmp
grep check-pythonappend $1.py | sort | uniq >py.tmp
diff i.tmp py.tmp || { echo "pythonappend error"; exit 1; }

a=`grep -c '%pythonappend' $1.i`
b=`grep -c 'check-pythonappend' $1.i`
if [[ $a != $b ]]; then
 echo "%pythonappend = $a"
 echo "check-pythonappend = $b"
 echo "Error: every %pythonappend should be checked"
 exit 1
fi
