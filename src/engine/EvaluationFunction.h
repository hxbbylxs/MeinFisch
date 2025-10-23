//
// Created by salom on 16.07.2025.
//

#ifndef EVALUATIONFUNCTION_H
#define EVALUATIONFUNCTION_H


#include "GameBoard.h"

int evaluate(GameBoard const & board);
int getGamePhaseScore(GameBoard const & board);

struct Accumulator {
    int mg_evaluation = 0;
    int eg_evaluation = 0;
    int game_phase_score = 24;
    void reset_to(GameBoard const & board);
    void update(Move move, int en_passant, bool unmake);
};

inline Accumulator pst_evaluation_accumulator;


#endif //EVALUATIONFUNCTION_H
