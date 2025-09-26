//
// Created by salom on 23.08.2025.
//

#ifndef MOVEPICKING_H
#define MOVEPICKING_H

#include <vector>

#include "GameBoard.h" // lib game logic
#include "Move.h"

#include "Memory.h"

enum MoveGenPhase {
    TTMove,
    Good_Captures,
    Killer,
    Counter,
    Good_Quiets,
    Bad_Moves,
    Done,

    QCaptures,
    QPawns,
    QDone
};
inline constexpr int NUM_MOVE_GEN_PHASES = 7;

std::vector<Move> pickNextMoves(Data const & savedData, Move counter_candidate, Move killer_candidate, GameBoard const & board, MoveGenPhase & phase);

void staticMoveOrdering(std::vector<Move> & pseudoLegalMoves, GameBoard const & board);
int getMoveScoreEndGame(Move move);
int getMoveScoreMiddleGame(Move move);
void mvv_lva_MoveOrdering(std::vector<Move> & pseudoLegalMoves);

int static_exchange_evaluation(unsigned square, bool attacker_is_white, GameBoard & board, Constants::Piece piece_on_square);
int static_exchange_evaluation(Move move, GameBoard & board);

Move getCheapestAttackMove(GameBoard const & board, bool attacker_is_white, unsigned square, Constants::Piece piece_on_square);


#endif //MOVEPICKING_H
