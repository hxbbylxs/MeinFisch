//
// Created by salom on 24.06.2025.
//

#ifndef OUTPUT_H
#define OUTPUT_H

#include <string>
#include <chrono>

#include "GameBoard.h" // lib game_logic
#include "Move.h"

void printGameBoard(GameBoard const & board);
void printPlayerToMove(bool whiteToMove);
void printBitBoard(uint64_t number);
void printCompleteMove(Move const & move);

std::string evaluationToString(int evaluation);
void printAnalysisData(std::pair<uint32_t,int> const & move, int depth, int seldepth, std::chrono::time_point<std::chrono::system_clock> start, int nodes, std::string const & pv);

#endif //OUTPUT_H
