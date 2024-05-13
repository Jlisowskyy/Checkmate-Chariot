import multiprocessing
import os
import subprocess
import matplotlib.pyplot as plt

timeForGameMs = 10000 # 10 seconds
gameMoves = "g1f3 d7d5 g2g3 g7g6 f1g2 f8g7 e1g1 e7e5 c2c4 d5d4 b1a3 g8e7 d2d3 e8g8 b2b4 a7a5 b4b5 b8d7 a3c2 f8e8 c1a3 e7f5 a1b1 d7b6 b1c1 g7h6 c1a1 b6a4 d1e1 a8a7 f3d2 f5d6 e2e4 a7a8 f2f3 b7b6 f3f4 e5f4 e4e5 c8b7 g2b7 d6b7 g3f4 b7c5 a3c5 a4c5 d2e4 c5d3 e1g3 d3f4 f1f4 e8e5 e4f6 g8g7 f4e4 e5e4 f6e4 g7g8 a1d1 h6g7 g3f3 a5a4 g1g2 c7c5 b5c6 a8c8 c2b4 g7e5 e4g3 d8e7 b4d5 e7d8 d1f1 c8c6 f3f7 g8h8 f1e1".split(" ")
calculateTimePerMoveCommand = 'ctpm'
timePerMoveHistory = []

# check if Checkmate-Chariot exists
os.chdir('../../')
if not os.path.exists('Checkmate-Chariot'):
    # Compile Checkmate-Chariot
    print('Checkmate-Chariot not found, compiling...')
    os.system('chmod +x clean-cmake.sh')
    os.system('./clean-cmake.sh')
    os.system('cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release')
    os.system(f'make -j {multiprocessing.cpu_count()}')

# list files in the current directory

# run Checkmate-Chariot as a subprocess
process = subprocess.Popen(["./Checkmate-Chariot"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)


# simulate a game
playedMovesString = ""
for i, move in enumerate(gameMoves):
    # send the move to the engine
    playedMovesString += move + " "
    if i % 2 == 1:
        continue
    positionCommandString=f'position startpos moves {playedMovesString}\n'
    process.stdin.write(positionCommandString)
    process.stdin.flush()
    # make the engine calculate the time allocated for the next move
    calculateTimePerMoveCommandFull = f'{calculateTimePerMoveCommand} btime {timeForGameMs}\n'
    process.stdin.write(calculateTimePerMoveCommandFull)
    process.stdin.flush()

    # read the output of the engine
    output = process.stdout.readline()
    # get the part after : and remove the newline character
    timeAllocated = output.split(":")[1].strip()
    timePerMoveHistory.append( (i, timeAllocated) )
    print(f"Time allocated for move {move}: {timeAllocated} ms")

    # wait for the time allocated
    timeForGameMs -= int(timeAllocated)

# close the process
process.stdin.close()
process.stdout.close()
process.stderr.close()
process.wait()

# plot the time allocated for each move
# get the x and y values
x = [int(i) for i, _ in timePerMoveHistory]
y = [int(timeAllocated) for _, timeAllocated in timePerMoveHistory]

# plot the values
plt.plot(x, y)
plt.xlabel('Move number')
plt.ylabel('Time allocated (ms)')
plt.title('Time allocated for each move')
plt.show()