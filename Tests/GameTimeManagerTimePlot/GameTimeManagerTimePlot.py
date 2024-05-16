import os
import subprocess
import multiprocessing
import matplotlib.pyplot as plt


class ChessEngine:
    def __init__(self, engine_path='Checkmate-Chariot', build=True):
        self.engine_path = engine_path
        self.process = None
        if build:
            self.build_engine()
        self.start_engine()

    def build_engine(self):
        os.chdir('../../')
        if not os.path.exists(self.engine_path):
            print('Checkmate-Chariot not found, compiling...')
            os.system('chmod +x clean-cmake.sh')
            os.system('./clean-cmake.sh')
            os.system('cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release')
            os.system(f'make -j {multiprocessing.cpu_count()}')

    def start_engine(self):
        self.process = subprocess.Popen(
            ["./" + self.engine_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

    def close_engine(self):
        if self.process:
            self.process.stdin.close()
            self.process.stdout.close()
            self.process.stderr.close()
            self.process.wait()

    def calculate_time_per_move(self, game_moves, initial_time_ms):
        time_per_move_history = []
        played_moves_string = ""
        time_for_game_ms = initial_time_ms

        for i, move in enumerate(game_moves):
            played_moves_string += move + " "
            if i % 2 == 1:
                continue
            position_command_string = f'position startpos moves {played_moves_string}\n'
            self.process.stdin.write(position_command_string)
            self.process.stdin.flush()
            calculate_time_per_move_command_full = f'ctpm btime {time_for_game_ms}\n'
            self.process.stdin.write(calculate_time_per_move_command_full)
            self.process.stdin.flush()

            output = self.process.stdout.readline()
            time_allocated = output.split(":")[1].strip()
            time_per_move_history.append((i, int(time_allocated)))
            print(f"Time allocated for move {move}: {time_allocated} ms")

            time_for_game_ms -= int(time_allocated)

        print(f'Time left: {time_for_game_ms} ms')
        return time_per_move_history


def plot_time_per_move(time_per_move_history):
    x = [int(i) for i, _ in time_per_move_history]
    y = [int(time_allocated) for _, time_allocated in time_per_move_history]

    plt.plot(x, y)
    plt.xlabel('Move number')
    plt.ylabel('Time allocated (ms)')
    plt.title('Time allocated for each move')
    plt.show()


# Example of how to use the encapsulated code:
if __name__ == "__main__":
    game_moves = "g1f3 d7d5 g2g3 g7g6 f1g2 f8g7 e1g1 e7e5 c2c4 d5d4 b1a3 g8e7 d2d3 e8g8 b2b4 a7a5 b4b5 b8d7 a3c2 f8e8 c1a3 e7f5 a1b1 d7b6 b1c1 g7h6 c1a1 b6a4 d1e1 a8a7 f3d2 f5d6 e2e4 a7a8 f2f3 b7b6 f3f4 e5f4 e4e5 c8b7 g2b7 d6b7 g3f4 b7c5 a3c5 a4c5 d2e4 c5d3 e1g3 d3f4 f1f4 e8e5 e4f6 g8g7 f4e4 e5e4 f6e4 g7g8 a1d1 h6g7 g3f3 a5a4 g1g2 c7c5 b5c6 a8c8 c2b4 g7e5 e4g3 d8e7 b4d5 e7d8 d1f1 c8c6 f3f7 g8h8 f1e1".split(" ")
    initial_time_ms = 10000  # 10 seconds

    engine = ChessEngine()
    try:
        time_per_move_history = engine.calculate_time_per_move(game_moves, initial_time_ms)
        plot_time_per_move(time_per_move_history)
    finally:
        engine.close_engine()
