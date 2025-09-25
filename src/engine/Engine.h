//
// Created by salom on 13.07.2025.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <array>
#include <utility>

#include "GameBoard.h"
#include "Move.h"



inline std::array<int,Constants::MAX_RECURSION_DEPTH> global_evaluated_positions_counter = {}; // TODO remove
inline std::array<int,Constants::MAX_RECURSION_DEPTH> evaluated_positions_in_quiscence = {}; // TODO remove


void startTimeLimit(int timeLimit);
void playerControlledTimeLimit(int ignore);

std::pair<Move,int> iterativeDeepening(GameBoard & board, int timeLimit, int max_depth);
std::pair<Move,int> getOptimalMoveNegaMax(GameBoard & board, int maxRecursionDepth);
int negaMax(GameBoard  & board, int remaining_depth, int alpha, int beta, int depth, Move previous_move, bool null_move_allowed); // searches all moves (expected bad ones lower depth)
int quiscenceSearch(GameBoard & board, int remaining_depth, int alpha, int beta, int depth); // searches only captures

std::string reconstructPV(GameBoard & board);




#endif //ENGINE_H
