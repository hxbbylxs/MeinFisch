//
// Created by salom on 16.07.2025.
//

#ifndef EVALUATIONFUNCTION_H
#define EVALUATIONFUNCTION_H
#include <cstdint>
#include "GameBoard.h"

struct EvalInfo {
    int mg_evaluation = 0;
    int eg_evaluation = 0;
    uint64_t white_pieces = 0;
    uint64_t black_pieces = 0;
    uint64_t white_king_zone_small = 0;
    uint64_t black_king_zone_small = 0;
    uint64_t white_king_zone_large = 0;
    uint64_t black_king_zone_large = 0;
    uint64_t whites_defended_pieces = 0;
    uint64_t blacks_defended_pieces = 0;
    uint64_t squares_attacked_by_less_valuable_white_pieces = 0;
    uint64_t squares_attacked_by_less_valuable_black_pieces = 0;
};

int evaluate(GameBoard const & board, int alpha, int beta);
int getGamePhaseScore(GameBoard const & board);

void evaluateKing(GameBoard const & board, EvalInfo & eval_info);

void evaluatePawns(GameBoard const & board, EvalInfo & eval_info);
void evaluateKnights(GameBoard const & board, EvalInfo & eval_info);
void evaluateBishops(GameBoard const & board, EvalInfo & eval_info);
void evaluateRooks(GameBoard const & board, EvalInfo & eval_info);
void evaluateQueen(GameBoard const & board, EvalInfo & eval_info);



int evaluateSpaceControl(GameBoard const & board);

int evaluateMaterialOnly(GameBoard const & board);



#endif //EVALUATIONFUNCTION_H
