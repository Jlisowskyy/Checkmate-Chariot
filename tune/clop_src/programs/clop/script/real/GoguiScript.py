#!/usr/bin/env python
#############################################################################
"""
 GoguiScript.py

 Run a game of Go via gogui tools.
 Parameters of the program to be optimized are set via GTP commands.
 This script will create a sub-directory for each game.
 Edit the option section to customize this script.

 see DummyScript.py for more info
"""
#############################################################################
import sys
import os
import shutil
import platform

#############################################################################
# Preliminaries
#############################################################################

#
# Processor and seed are passed on the command line
#
processor = sys.argv[1]
seed = int(sys.argv[2])

#
# Create empty directory for this seed, and chdir to it
#
path = "GoguiExperiment.dir/game-%07d" % seed
shutil.rmtree(path, ignore_errors = True)
os.makedirs(path)
os.chdir(path)

#############################################################################
#
# System options
# 
# This script needs gogui-adapter to set parameters via gtp commands.
# gogui-adapter version 1.2.1 does not understand "resign".
# This problem was fixed:
# http://sf.net/tracker/?func=detail&atid=489964&aid=3016960&group_id=59117
# If you wish to play games with programs that resign, use a fixed version.
# 
#############################################################################
sys_id = platform.system()
#print "sys_id == ", sys_id
if sys_id == 'Windows' or sys_id == 'CYGWIN_NT-5.1':
    gogui_location = 'c:\\Program Files\\gogui\\lib\\'
    gogui_twogtp = 'java -jar "' + gogui_location + 'gogui-twogtp.jar"'
    gogui_adapter = 'java -jar \\"' + gogui_location + 'gogui-adapter.jar\\"'
else:
    gogui_twogtp = 'gogui-twogtp'
    gogui_adapter = 'gogui-adapter'

#############################################################################
# Experiment options
#############################################################################

# board size (gogui option)
board_size = '-size 19'

# time control (gogui option)
time_control = '-time 15'

# komi (gogui option)
twogtp_komi = '-komi -30.5'

# command prefix and suffix (typically used for distributed computation)
#command_prefix = 'ssh ' + processor + ' cd ' + path + ';'
command_prefix = ''
command_suffix = ''

# program to be optimized
optimized_program = "cs19-0011-05"
#optimized_program = "g:\\mimi\\rc2\\programs\\CrazyStone\\compgcc\\bin\\CrazyStone.exe"

# setting a parameter is done with "<gtp_prefix> <parameter_name> <value>"
gtp_prefix = 'crazy-analyst MCSearch param'

# GTP commands sent to the optimized program before starting
optimized_settings = 'crazy-analyst MCSearch param Seed ' + str(seed) + '\n'
optimized_settings += """
crazy-param Log_file gtp.log
crazy-param Sentinel_file ../../sentinel
crazy-analyst MCSearch param Log_file MCSearch.log
crazy-analyst MCSearch param Processors 2
crazy-analyst MCSearch param Ponder 0
crazy-analyst MCSearch param Resign_threshold 95
"""

# (fixed) opponent program
#gnu_level = '5'
#opponent_program = 'gnugo --mode gtp --chinese-rules --positional-superko --seed ' + str(seed) + ' --level ' + gnu_level + ' --min-level ' + gnu_level + ' --max-level ' + gnu_level
#opponent_program = optimized_program
opponent_program = 'fuego'

# GTP commands sent to the opponent program before starting
opponent_settings = 'uct_param_search number_threads 2'

#############################################################################
# You should not have to edit anything beyond this point
#############################################################################

#
# Create initialization file for the program to be optimized
#
gtp_file = open('optimized_settings.gtp', 'w')
gtp_file.write(optimized_settings)

i = 4
params = []
while i < len(sys.argv):
    name = sys.argv[i - 1]
    value = sys.argv[i]
    gtp_file.write(gtp_prefix + ' ' + name + ' ' + value + '\n')
    i += 2

gtp_file.close()

#
# Create initialization file for the opponent
#
open('opponent_settings.gtp', 'w').write(opponent_settings)

#
# Protect program names with quotes
#
optimized_program = '\\"' + optimized_program + '\\"'
opponent_program = '\\"' + opponent_program + '\\"'

#
# Run one game with gogui-twogtp
#
command = command_prefix +\
 gogui_twogtp + ' ' + time_control + ' ' + board_size + ' ' + twogtp_komi +\
 ' -white "' + gogui_adapter + ' -log white-adapter.log ' + board_size +\
          ' -gtpfile optimized_settings.gtp ' + optimized_program + '"' +\
 ' -black "' + gogui_adapter + ' -log black-adapter.log ' + board_size +\
          ' -gtpfile opponent_settings.gtp ' + opponent_program + '"' +\
 ' -sgffile twogtp.sgf -games 1 -auto' + command_suffix

#print "command = ", command

os.system(command)

#
# Return game result
#
try:
    dat_file = open('twogtp.sgf.dat', 'r')
    
    for i in range(16):
        dat_file.readline()
    line = dat_file.readline()
    
    dat_file.close()
    
    if 'unexpectedly' in line: # problem
        print "Error: " + line
    elif 'W+' in line: # win
        print "W"
    elif 'B+' in line: # loss
        print "L"
    else:
        print "Error: could not determine game result"
except IOError:
    print "Error: IOError"
