#!/bin/bash
{
 echo set terminal x11
 echo set logscale xy
 echo set style data lines
 echo set key bottom left reverse Left

 echo "plot [10:] \\"
# echo "500*x**-0.5 lt 2,500*x**-0.6 lt 2,500*x**-0.75 lt 2,\\"

 lt=3

 for i in cpd-$1*.dat; do
  title=`echo $i | sed -e "s/cpd-\(.*\).dat/\\1/"`
  echo \"$i\" "using 3:4  lt $lt lw 2 title \"$title\",\\"
#  echo \"$i\" "using 3:8  lt $lt lw 1 with lines notitle,\\"
#  echo \"$i\" "using 3:10 lt $lt lw 1 with lines notitle,\\"
#  echo \"$i\" "using 3:12 lt $lt lw 1 with lines notitle,\\"
  let lt++
 done

 echo 1/0 notitle

} | gnuplot -persist
