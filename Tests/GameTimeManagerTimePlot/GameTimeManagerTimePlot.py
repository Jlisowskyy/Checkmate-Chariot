import os
import subprocess
import multiprocessing
import matplotlib.pyplot as plt
import chess
import chess.pgn
from io import StringIO
from enum import Enum


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

    def calculate_time_per_move(self, game_moves, color, initial_time_ms, increment_ms=0):
        time_str = 'wtime' if color == Game.PlayerColor.WHITE else 'btime'
        increment_str = 'winc' if color == Game.PlayerColor.WHITE else 'binc'

        time_per_move_history = []
        played_moves_string = ""
        time_for_game_ms = initial_time_ms

        for i, move in enumerate(game_moves):
            played_moves_string += move + " "

            if color == Game.PlayerColor.WHITE and i % 2 == 0:
                continue
            if color == Game.PlayerColor.BLACK and i % 2 == 1:
                continue

            position_command_string = f'position startpos moves {played_moves_string}\n'
            self.process.stdin.write(position_command_string)
            self.process.stdin.flush()

            calculate_time_per_move_command = (
                f'ctpm {time_str} {time_for_game_ms} {increment_str} {increment_ms}\n'
                if increment_ms != 0
                else f'ctpm {time_str} {time_for_game_ms}\n'
            )
            self.process.stdin.write(calculate_time_per_move_command)
            self.process.stdin.flush()

            output = self.process.stdout.readline()
            time_allocated = int(output.split(":")[1].strip())
            time_per_move_history.append((i, time_allocated))

            time_for_game_ms -= time_allocated - increment_ms
            print(f"Time allocated for move {move}: {time_allocated} ms | Time left: {time_for_game_ms} ms")

            if time_for_game_ms < 0:
                print("Time limit exceeded | Moves left: ", len(game_moves[i:]))
                break

        return time_per_move_history

    def plot_time_per_move(self, game_name, time_per_move_history):
        x = [i for i, _ in time_per_move_history]
        y = [time_allocated for _, time_allocated in time_per_move_history]

        plt.plot(x, y)
        plt.xlabel('Move number')
        plt.ylabel('Time allocated (ms)')
        plt.title(game_name)
        plt.show()

    def pgn_str_to_uci_str(self, pgn):
        pgn_io = StringIO(pgn)
        game = chess.pgn.read_game(pgn_io)
        board = game.board()
        uci_moves = [move.uci() for move in game.mainline_moves()]
        return ' '.join(uci_moves)


class Game:
    class PlayerColor(Enum):
        WHITE = True
        BLACK = False

    def __init__(self, name, game_moves, color, initial_time_ms, increment_ms=0):
        self.name = name
        self.game_moves = game_moves.split()
        self.color = color
        self.initial_time_ms = initial_time_ms
        self.increment_ms = increment_ms

    def calculate_time_per_move(self, engine):
        return engine.calculate_time_per_move(self.game_moves, self.color, self.initial_time_ms, self.increment_ms)

    def plot_time_per_move(self, engine, time_per_move_history):
        engine.plot_time_per_move(self.name, time_per_move_history)


def main():
    engine = ChessEngine()

    games = [
        Game(
            "40 moves game",
            "g1f3 d7d5 g2g3 g7g6 f1g2 f8g7 e1g1 e7e5 c2c4 d5d4 b1a3 g8e7 d2d3 e8g8 b2b4 a7a5 b4b5 b8d7 a3c2 f8e8 c1a3 e7f5 a1b1 d7b6 b1c1 g7h6 c1a1 b6a4 d1e1 a8a7 f3d2 f5d6 e2e4 a7a8 f2f3 b7b6 f3f4 e5f4 e4e5 c8b7 g2b7 d6b7 g3f4 b7c5 a3c5 a4c5 d2e4 c5d3 e1g3 d3f4 f1f4 e8e5 e4f6 g8g7 f4e4 e5e4 f6e4 g7g8 a1d1 h6g7 g3f3 a5a4 g1g2 c7c5 b5c6 a8c8 c2b4 g7e5 e4g3 d8e7 b4d5 e7d8 d1f1 c8c6 f3f7 g8h8 f1e1",
            Game.PlayerColor.BLACK, 10000, 0
        ),
        Game(
            "90 moves game",
            engine.pgn_str_to_uci_str(
                "1. Nc3 d5 2. d4 Nf6 3. Nf3 c6 4. Bf4 e6 5. e3 Bd6 6. Bxd6 Qxd6 7. Bd3 Nbd7 8. e4 dxe4 9. Bxe4 O-O 10. Bd3 Rd8 11. O-O c5 12. Nb5 Qb6 13. a4 cxd4 14. a5 Qc5 15. Nbxd4 h6 16. Nb5 Qe7 17. Nc7 Rb8 18. Qd2 a6 19. Rad1 Qc5 20. Qc3 Qxc3 21. bxc3 b5 22. c4 bxc4 23. Bxc4 Bb7 24. Bxa6 Rdc8 25. Bxb7 Rxb7 26. Nxe6 fxe6 27. Ra1 Ra8 28. Rfe1 Ra6 29. h4 Rb4 30. Ree3 Kf8 31. g3 Ke7 32. Ree1 Rb5 33. Nd4 Rc5 34. f4 Rc3 35. Kh2 Rc4 36. Rad1 Rc5 37. Ree2 Rd5 38. c4 Rc5 39. Rde1 Nf8 40. Rc1 Rd6 41. Nb3 Rcc6 42. Rd2 Rxd2+ 43. Nxd2 N8d7 44. Kg2 Rd6 45. Nf3 Ra6 46. Ra1 Kd6 47. Nd2 Nc5 48. Kf3 Kc7 49. Ke3 Rd6 50. Ra3 h5 51. Ke2 Ra6 52. Ke3 Rd6 53. Ke2 g6 54. Nf3 Kb7 55. Ne5 Ka6 56. Nxg6 Nfe4 57. Ne5 Nd2 58. Ke3 Ndb3 59. Kf3 Rd2 60. g4 Nd4+ 61. Ke3 Rd1 62. Nd3 Ndc2+ 63. Kf3 Nxd3 64. Rc3 Nd4+ 65. Ke3 Nxf4 66. Kxf4 Ne2+ 67. Ke5 Nxc3 68. gxh5 Rh1 69. Kf6 Rxh5 70. Kg6 Rh1 71. h6 Kxa5 72. h7 e4 73. c5 Kb5 74. Kg7 Rxh7+ 75. Kxh7 Kxc5 76. Kh6 e4 77. Kh5 e3 78. Kg4 e2 79. Kf5 Kc4 80. Kg6 Kb3 81. Kh7 Nd5 82. Kh8 e1=Q 83. Kh7 Qg3 84. Kh8 Ne3 85. Kh7 Qg5 86. Kh8 Qf6+ 87. Kg8 Kc2 88. Kh7 Nf5 89. Kg8 Qg7#"
            ),
            Game.PlayerColor.WHITE, 1000 * 60 * 5, 3000
        ),
    ]

    try:
        for game in games:
            time_per_move_history = game.calculate_time_per_move(engine)
            game.plot_time_per_move(engine, time_per_move_history)
    finally:
        engine.close_engine()


if __name__ == "__main__":
    main()
