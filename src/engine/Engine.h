//
// Created by salom on 13.07.2025.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <array>
#include <utility>
#include <cstdint>

#include "GameBoard.h"
#include "Memory.h"

enum MoveGenPhase {
    TTMove,
    Killer,
    Captures,
    Quiets,
    Done
};


inline std::array<int,Constants::MAX_RECURSION_DEPTH> global_evaluated_positions_counter = {}; // TODO remove
inline std::array<int,Constants::MAX_RECURSION_DEPTH> evaluated_positions_in_quiscence = {}; // TODO remove

void staticMoveOrdering(std::vector<uint32_t> & pseudoLegalMoves, GameBoard const & board);
int getMoveScoreEndGame(uint32_t move);
int getMoveScoreMiddleGame(uint32_t move);
void mvv_lva_MoveOrdering(std::vector<uint32_t> & pseudoLegalMoves);

bool isCriticalMove(uint32_t move);

std::vector<uint32_t> pickNextMoves(Data const & savedData, uint32_t killerCandidate, GameBoard const & board, MoveGenPhase & phase);

void startTimeLimit(int timeLimit);
void playerControlledTimeLimit(int ignore);

std::pair<uint32_t,int> iterativeDeepening(GameBoard & board, int timeLimit);
std::pair<uint32_t,int> getOptimalMoveNegaMax(GameBoard & board, int maxRecursionDepth);
int negaMax(GameBoard  & board, int maxRecursionDepth, int alpha, int beta);
int quiscenceSearch(GameBoard & board, int maxRecursionDepth, int alpha, int beta);




#endif //ENGINE_H
