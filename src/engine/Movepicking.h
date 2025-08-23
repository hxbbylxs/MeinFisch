//
// Created by salom on 23.08.2025.
//

#ifndef MOVEPICKING_H
#define MOVEPICKING_H

#include <vector>
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

std::vector<uint32_t> pickNextMoves(Data const & savedData, uint32_t killerCandidate, GameBoard const & board, MoveGenPhase & phase);

void staticMoveOrdering(std::vector<uint32_t> & pseudoLegalMoves, GameBoard const & board);
int getMoveScoreEndGame(uint32_t move);
int getMoveScoreMiddleGame(uint32_t move);
void mvv_lva_MoveOrdering(std::vector<uint32_t> & pseudoLegalMoves);



#endif //MOVEPICKING_H
