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
    Good_Captures,
    Killer,
    Counter,
    Good_Quiets,
    Bad_Moves,
    Done
};
inline constexpr int NUM_MOVE_GEN_PHASES = 7;

std::vector<uint32_t> pickNextMoves(Data const & savedData, uint32_t counter_candidate, uint32_t killer_candidate, GameBoard const & board, MoveGenPhase & phase);

void staticMoveOrdering(std::vector<uint32_t> & pseudoLegalMoves, GameBoard const & board);
int getMoveScoreEndGame(uint32_t move);
int getMoveScoreMiddleGame(uint32_t move);
void mvv_lva_MoveOrdering(std::vector<uint32_t> & pseudoLegalMoves);

int static_exchange_evaluation(int square, bool attacker_is_white, GameBoard & board, Constants::Piece piece_on_square);
int static_exchange_evaluation(uint32_t move, GameBoard & board);

uint32_t getCheapestAttackMove(GameBoard const & board, bool attacker_is_white, int square, Constants::Piece piece_on_square);


#endif //MOVEPICKING_H
