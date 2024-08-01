#!/bin/bash
 cat cpd.gpi

 echo "plot [10:] \\"
 lt=1
 for i in cpd-*.dat; do
  title=`echo $i | sed -e "s/cpd-\(.*\).dat/\\1/"`
  echo \"$i\" "using 3:4  lt $lt lw 3 title \"$title\",\\"
#  echo \"$i\" "using 3:8  lt $lt lw 1 with lines notitle,\\"
#  echo \"$i\" "using 3:10 lt $lt lw 1 with lines notitle,\\"
#  echo \"$i\" "using 3:12 lt $lt lw 1 with lines notitle,\\"
  let lt++
 done
 echo 220*x**\(-0.50\) lt $lt lw 1 with lines,\
      700*x**\(-0.75\) lw 1 with lines
