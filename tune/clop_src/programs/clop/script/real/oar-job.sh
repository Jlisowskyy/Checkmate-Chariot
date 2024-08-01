#!/bin/bash
cp -f $OAR_NODEFILE ./oar-nodefile.tmp
echo $OAR_JOBID >./oar-jobid.tmp
sleep 100d
