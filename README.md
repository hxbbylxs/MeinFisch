# MeinFisch

A chess engine in **C++** built with **CMake**. <br><br>
<img src="https://en.meming.world/images/en/b/b1/Baby_Beats_Computer_at_Chess.jpg" alt="*Meme not loading*" width="200" align="top">

## Requirements

- C++20-compatible Compiler  
- CMake (>= 3.30)  

## Compiling

```bash
git clone https://github.com/hxbbylxs/MeinFisch.git
cd MeinFisch
mkdir build && cd build
cmake ..
cmake --build
```

## Execution

MeinFisch is UCI-compatible. To run the engine, execute the binary and use UCI commands such as:  
  
```text
ucinewgame
position startpos
go
```  
For testing and GUI support, the engine can be used with a GUI like CuteChess https://github.com/cutechess/cutechess/releases  

## Key Concepts in the Code
Basics:  
Minimax, Pruning: A really good explanation: https://www.youtube.com/watch?v=l-hh51ncgDI&t=1s    
Bitboards: https://de.wikipedia.org/wiki/Bitboard  
Magic Bitboards: https://www.chessprogramming.org/Magic_Bitboards  
Negamax: https://en.wikipedia.org/wiki/Negamax  
Transposition Table (TT): https://en.wikipedia.org/wiki/Transposition_table  
Zobrist Hashing: https://www.chessprogramming.org/Zobrist_Hashing  
Universal Chess Interface (UCI): https://page.mi.fu-berlin.de/block/uci.htm

Advanced:  
Piece Square Tables (PST): https://www.chessprogramming.org/Piece-Square_Tables  
History Heuristic: https://www.chessprogramming.org/History_Heuristic  
Killer Heuristic: https://www.chessprogramming.org/Killer_Heuristic  
Late Move Reductions (LMR): https://www.chessprogramming.org/Late_Move_Reductions  

