//
// Created by salom on 16.07.2025.
//

#ifndef EVALUATIONFUNCTION_H
#define EVALUATIONFUNCTION_H
#include <cstdint>
#include "GameBoard.h"

int evaluate(GameBoard const & board, int alpha, int beta);
int getGamePhaseScore(GameBoard const & board);

void evaluatePawns(GameBoard const & board, int & mg_evaluation, int & eg_evaluation, uint64_t white_king_zone, uint64_t black_king_zone,uint64_t & whitePawnAttackedSquares, uint64_t & blackPawnAttackedSquares, int & num_attackers_white_king_zone, int & num_attackers_black_king_zone);
void evaluateKnights(GameBoard const & board, int & mg_evaluation, int & eg_evaluation, uint64_t white_king_zone, uint64_t black_king_zone,uint64_t whitePawnAttackedSquares, uint64_t blackPawnAttackedSquares, int & num_attackers_white_king_zone, int & num_attackers_black_king_zone);
void evaluateBishops(GameBoard const & board, int & mg_evaluation, int & eg_evaluation, uint64_t white_king_zone, uint64_t black_king_zone,uint64_t whitePawnAttackedSquares, uint64_t blackPawnAttackedSquares, int & num_attackers_white_king_zone, int & num_attackers_black_king_zone);
void evaluateRooks(GameBoard const & board, int & mg_evaluation, int & eg_evaluation, uint64_t white_king_zone, uint64_t black_king_zone, int & num_attackers_white_king_zone, int & num_attackers_black_king_zone);
void evaluateQueen(GameBoard const & board, int & mg_evaluation, int & eg_evaluation, uint64_t white_king_zone, uint64_t black_king_zone, int & num_attackers_white_king_zone, int & num_attackers_black_king_zone);

void evaluateKing(GameBoard const & board, int & mg_evaluation, int & eg_evaluation);

int evaluateSpaceControl(GameBoard const & board);

int evaluateMaterialOnly(GameBoard const & board);



#endif //EVALUATIONFUNCTION_H
