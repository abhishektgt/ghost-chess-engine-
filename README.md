# Ghost Chess Engine

Ghost is a C chess engine that communicates through the UCI protocol. It is intended to be used either from a terminal by sending UCI commands manually, or from a graphical chess GUI such as Arena, Cute Chess, Banksia, or any other UCI-compatible interface.

Estimated playing strength: around 1900-2000 Elo based on test games linked below.

## Features

- Bitboard-based board representation
- Magic bitboards for efficient sliding-piece move generation
- Negamax search with alpha-beta pruning
- Principal variation, killer move, and history move ordering
- Iterative deepening
- Principal Variation Search (PVS)
- Late Move Reduction (LMR)
- UCI protocol support

## Project Files

- `chess_engine.c` - engine source code
- `README.md` - usage and project documentation

## Requirements

This project is currently written for Windows.

You need a C compiler. On Windows, the easiest option is GCC from MSYS2/MinGW.

Check that GCC is available:

```powershell
gcc --version
```

## Build

From the project folder:

```powershell
gcc chess_engine.c -O2 -o ghost.exe
```

This creates:

```text
ghost.exe
```

You can choose another output name if you want:

```powershell
gcc chess_engine.c -O2 -o chess_engine.exe
```

## Run From Terminal

Start the engine:

```powershell
.\ghost.exe
```

The engine runs as a UCI engine. That means it waits for text commands and prints text responses. A normal terminal session looks like this:

```text
uci
isready
position startpos
go depth 6
quit
```

What each command does:

- `uci` - asks the engine to identify itself and enter UCI mode
- `isready` - checks whether the engine is ready
- `position startpos` - loads the normal chess starting position
- `position startpos moves e2e4 e7e5 g1f3` - loads the starting position and then applies moves
- `position fen <FEN>` - loads a custom FEN position
- `go depth 6` - searches the current position to depth 6
- `ucinewgame` - starts a new game
- `quit` - closes the engine

Example terminal session:

```text
uci
isready
position startpos moves e2e4 e7e5 g1f3 b8c6
go depth 5
quit
```

The engine prints search information and then a line like:

```text
bestmove <move>
```

For example:

```text
bestmove f1b5
```

## Use With A Graphical Interface

Ghost does not include its own graphical chess board. To play against it with a board, use a UCI-compatible chess GUI.

### Arena Chess GUI

Download Arena:

<http://www.playwitharena.de/>

Steps:

1. Build the engine with GCC so you have `ghost.exe`.
2. Open Arena.
3. Go to `Engines` -> `Install New Engine`.
4. Select the compiled `ghost.exe` file.
5. Choose UCI if Arena asks for the engine type.
6. Start a new game and select Ghost as one of the players.

### Other UCI GUIs

The same compiled `.exe` should also work in other UCI-compatible GUIs:

- Cute Chess
- Banksia GUI
- Lucas Chess
- Fritz/ChessBase interfaces that support UCI engines

In most GUIs, add a new UCI engine and point the GUI to the compiled `ghost.exe` file.

## Useful UCI Examples

Search from the starting position:

```text
position startpos
go depth 6
```

Search after a short opening:

```text
position startpos moves e2e4 e7e5 g1f3 b8c6 f1b5
go depth 6
```

Search a custom FEN:

```text
position fen rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2
go depth 5
```

Start a new game:

```text
ucinewgame
position startpos
go depth 6
```

## Troubleshooting

### `gcc` is not recognized

Install MSYS2 or MinGW-w64, then make sure the compiler's `bin` folder is added to your `PATH`.

### The engine opens and waits without showing a board

That is expected. This is a UCI engine, not a standalone graphical chess app. Use terminal UCI commands or load the compiled executable into a chess GUI.

### Compile errors in the current source

At the time this README was updated, compiling with:

```powershell
gcc chess_engine.c -O2 -o ghost.exe
```

reported a syntax error near the decorative `******Board********` heading in `chess_engine.c`. That heading should be turned into a comment before compiling.

If later errors mention `start_position` or `tricky_position`, define those FEN strings before they are used, for example:

```c
char start_position[] = "rn1qkbnr/ppp2ppp/3p4/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1";
```

or replace it with the standard starting FEN:

```c
char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
```

## Test Games

- <https://www.chess.com/game/computer/133732527>
- <https://www.chess.com/game/computer/133471741>
- <https://www.chess.com/game/computer/133445355>
- <https://www.chess.com/game/live/113595652923>

## Credits

- <https://www.chessprogramming.org/Main_Page>
- <https://trepo.tuni.fi/bitstream/handle/10024/140588/PodsechinIgor.pdf?sequence=2&isAllowed=y>
- <https://ameye.dev/notes/chess-engine/>
- <https://www.youtube.com/watch?v=QUNP-UjujBM&list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs>
- <https://www.youtube.com/watch?v=bGAfaepBco4&list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg>
