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
        self.process.stdin.write('ucinewgame\n')
        self.process.stdin.flush()

        time_str = 'wtime' if color == Game.PlayerColor.WHITE else 'btime'
        increment_str = 'winc' if color == Game.PlayerColor.WHITE else 'binc'

        time_per_move_history = []
        played_moves_string = ""
        time_for_game_ms = initial_time_ms
        game_time_with_increment = initial_time_ms

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
            time_per_move_history.append((i/2, time_allocated))

            time_for_game_ms -= time_allocated - increment_ms
            game_time_with_increment += increment_ms
            print(f"Time allocated for move {move}: {time_allocated} ms | Time left: {time_for_game_ms} ms")

            if time_for_game_ms < 0:
                print("Time limit exceeded | Moves left: ", len(game_moves[i:])/2)
                break

        print(f"Time utilization: {(initial_time_ms - time_for_game_ms) / initial_time_ms}")

        # validate that the sum of times in time_per_move_history is less than or equal to initial_time_ms
        print(f"Total time allocated: {sum([time for _, time in time_per_move_history])} | Total time per game: {game_time_with_increment}")

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
            "88 moves game",
            engine.pgn_str_to_uci_str(
                "1. Nc3 d5 2. d4 Nf6 3. Nf3 c6 4. Bf4 e6 5. e3 Bd6 6. Bxd6 Qxd6 7. Bd3 Nbd7 8. e4 dxe4 9. Bxe4 O-O 10. Bd3 Rd8 11. O-O c5 12. Nb5 Qb6 13. a4 cxd4 14. a5 Qc5 15. Nbxd4 h6 16. Nb5 Qe7 17. Nc7 Rb8 18. Qd2 a6 19. Rad1 Qc5 20. Qc3 Qxc3 21. bxc3 b5 22. c4 bxc4 23. Bxc4 Bb7 24. Bxa6 Rdc8 25. Bxb7 Rxb7 26. Nxe6 fxe6 27. Ra1 Ra8 28. Rfe1 Ra6 29. h4 Rb4 30. Ree3 Kf8 31. g3 Ke7 32. Ree1 Rb5 33. Nd4 Rc5 34. f4 Rc3 35. Kh2 Rc4 36. Rad1 Rc5 37. Ree2 Rd5 38. c4 Rc5 39. Rde1 Nf8 40. Rc1 Rd6 41. Nb3 Rcc6 42. Rd2 Rxd2+ 43. Nxd2 N8d7 44. Kg2 Rd6 45. Nf3 Ra6 46. Ra1 Kd6 47. Nd2 Nc5 48. Kf3 Kc7 49. Ke3 Rd6 50. Ra3 h5 51. Ke2 Ra6 52. Ke3 Rd6 53. Ke2 g6 54. Nf3 Kb7 55. Ne5 Ka6 56. Nxg6 Nfe4 57. Ne5 Nd2 58. Ke3 Ndb3 59. Kf3 Rd2 60. g4 Nd4+ 61. Ke3 Rd1 62. Nd3 Ndc2+ 63. Kf3 Nxd3 64. Rc3 Nd4+ 65. Ke3 Nxf4 66. Kxf4 Ne2+ 67. Ke5 Nxc3 68. gxh5 Rh1 69. Kf6 Rxh5 70. Kg6 Rh1 71. h6 Kxa5 72. h7 e4 73. c5 Kb5 74. Kg7 Rxh7+ 75. Kxh7 Kxc5 76. Kh6 e4 77. Kh5 e3 78. Kg4 e2 79. Kf5 Kc4 80. Kg6 Kb3 81. Kh7 Nd5 82. Kh8 e1=Q 83. Kh7 Qg3 84. Kh8 Ne3 85. Kh7 Qg5 86. Kh8 Qf6+ 87. Kg8 Kc2 88. Kh7 Nf5 89. Kg8 Qg7#"
            ),
            Game.PlayerColor.WHITE, 1000 * 60 * 5, 3000
        ),
        Game(
            "129 moves game",
            engine.pgn_str_to_uci_str(
                """1. e4 d5 2. e5 c5 3. e6 f6 4. d3 a6 5. a3 b6 6. b3 c4 7. c3 d4 8. f3 f5 9. g3 g6 10. h3 h6 11. h4 h5 12. g4 g5 13. f4 Nd7 14. b4 b5 15. a4 a5 16. dxc4 dxc3 17. bxa5 bxa4 18. fxg5 fxg4 19. Nh3 Ngf6 20. Nf4 Nh7 21. Nd5 Nhf6 22. Nb6 Ne4 23. Nd5 Nf2 24. Ne3 Nd3+ 25. Ke2 Nb2 26. Nd5 Nxc4 27. Nxe7 Nce5 28. Nd5 Nc4 29. Ne3 Nce5 30. Nc4 Nd3 31. Ne5 Nf2 32. Nf3 Nd3 33. Ne5 Nb4 34. Nc6 Nd3 35. Ne5 Nb2 36. Nc4 Nd3 37. Nb6 Nf4+ 38. Ke1 Ng2+ 39. Ke2 Nxh4 40. Nd5 Nc5 41. Nf4 Nd3 42. e7 Nc5 43. Ng2 Ne6 44. Ne1 Nf4+ 45. Kf2 Nd5 46. Nc2 Nb4 47. Nca3 Nd5 48. Nc4 Nf4 49. Kg1 Nd3 50. Ne5 Nc5 51. Nxg4 Nd3 52. Ne5 Nc5 53. Nd3 Ne4 54. Nf2 Nd2 55. Ne4 Nc4 56. Nf6+ Kf7 57. Ne4 Nb6 58. Nc5 Nc4 59. Ne4 Nd2 60. Ng3 Ne4 61. Nf5 Nc5 62. Nd6+ Kg6 63. Ne4 Nd3 64. Nf2 Nf4 65. Nd3 Nd5 66. Nb4 Ne3 67. Nc2 Ng4 68. Nd4 Nf6 69. Nc6 Ne4 70. Nd4 Nf2 71. Nc2 Ne4 72. Nb4 Nxg5 73. Nd5 Ngf3+ 74. Kf2 Nf5 75. Ne3 N3d4 76. Nc4 Ne6 77. Nb6 Ned4 78. Nd5 Nd6 79. Nb4 Nc2 80. Nd3 Nc4 81. Be3 Nd4 82. Ke1 Ne2 83. Ne5+ Kf6 84. Nxc4 Nf4 85. Nb6 Nd5 86. Nxa4 Nb6 87. Nc5 Nc4 88. Ne4+ Kg6 89. Nexc3 Nxa5 90. Nd5 Nc4 91. Rxa8 Nb6 92. Nb4 Nxa8 93. Nc6 Nb6 94. Nc3 Kf5 95. Qxd8 Nd7 96. Bc5 Nxc5 97. Rh4 Kg5 98. Ne5 Rh6 99. Ne4+ Kxh4 100. Bd3 Nxd3+ 101. Kf1 Be6 102. Qb6 Bc4 103. Nf3+ Kh3 104. Nfg5+ Kh2 105. Nf6 Rg6 106. Nge4 Rg5 107. Nd5 Nf4+ 108. Ke1 Re5 109. Nf6 Nd5 110. Qc5 Bb5 111. Ng4+ Kh1 112. Kd2 Nb4 113. Nd6 Nd5 114. Nf6 Nf4 115. Nd5 Nd3 116. Nf4 Nb4 117. Ne6 Nc6 118. Nd4 Nb4 119. Nc6 Na6 120. Qd5+ Re4 121. Nb4 Nc5 122. Nc4 Kg2 123. Qg5+ Rg4 124. Ke3 h4 125. Ne5 h3 126. Ng6 h2 127. e8=Q h1=Q 128. Nf4+ Kf1 129. Nc6 Nd3"""
            ),
            Game.PlayerColor.WHITE, 1000 * 60 * 5, 3000
        )
    ]

    try:
        for game in games:
            time_per_move_history = game.calculate_time_per_move(engine)
            game.plot_time_per_move(engine, time_per_move_history)
    finally:
        engine.close_engine()


if __name__ == "__main__":
    main()
