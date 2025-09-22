//
// Created by salom on 17.07.2025.
//

#include "Memory.h"

#include "EvaluationConstants.h"

#include <iostream>


// adds calculated data about a position to the transposition table (TT)
void tryMakeNewEntry(Evaluation_Flag evaluation_flag, int depth, int evaluation, Move bestMove, GameBoard const &board) {
    unsigned index = board.zobristHash & ((Constants::TTSIZE)-1);
    if (newEntryIsBetter(depth,index)) {
        transposition_table[index] = {board.zobristHash,evaluation_flag, depth, evaluation,bestMove};
    }
}

// compares the quality of an existing TT entry with the new position
bool newEntryIsBetter(int depth, unsigned index) {
    transposition_table[index].age--;
    bool isDeeper = depth >= transposition_table[index].depth;
    bool isOld = transposition_table[index].age <= 0;
    return  isDeeper || isOld;
}

//caller has to check if evaluation_flag != empty (dummy)
Data& getData(uint64_t hash) {
    unsigned index = hash & ((Constants::TTSIZE)-1);
    if (transposition_table[index].zobristHash == hash) {
        return transposition_table[index];
    }
    static Data dummy;
    return dummy;
}

//adjusting mate evaluations for higher layers in the search tree
//               /
//     mate in 6 plies here
//             ^
//            /
//           /
//  mate in 5 plies here
//
int updateReturnValue(int evaluation) {
    if (abs(evaluation) < CHECKMATE_VALUE-1000) return evaluation; // eval is not checkmate
    return evaluation > 0 ? evaluation-1 : evaluation+1; // eval is checkmate
}

// reversing updateReturnValue when going down in the search tree
int updateAlphaBetaValue(int alphaBeta) {
    if (alphaBeta >= CHECKMATE_VALUE-1000) return alphaBeta+1;
    if (alphaBeta <= -CHECKMATE_VALUE+1000) return alphaBeta-1;
    return alphaBeta;
}

void clearTT() {
    transposition_table.fill(Data());
}

// History Heuristic

// score is increased when a move causes a cutoff
void increaseMoveScore(Move move, int depth) {
    history_move_scores[move.from()][move.to()] = std::min(history_move_scores[move.from()][move.to()] + depth * depth,100'000);
}

void initializeHistoryHeuristic() {
    history_move_scores = {};
    history_move_scores[1][18] = 100; //b8c6
    history_move_scores[1][16] = -10; //b8a6
    history_move_scores[6][21] = 150; //g8f6
    history_move_scores[6][23] = -10; //g8h6
    history_move_scores[11][27] = 160; //d7d5
    history_move_scores[12][28] = 160; //e7e5
    history_move_scores[57][42] = 100; //b1c3
    history_move_scores[57][40] = -10; //b1a3
    history_move_scores[62][45] = 150; //g1f3
    history_move_scores[62][47] = -10; //g1h3
    history_move_scores[51][35] = 160; //d2d4
    history_move_scores[52][36] = 160; //e2e4
}

// in the history heuristic
// because old values might have too high scores
void decreaseAllMoveScores() {
    for (int from = 0; from < Constants::NUM_SQUARES; from++) {
        for (int to = 0; to < Constants::NUM_SQUARES; to++) {
            history_move_scores[from][to] >>= 1;
        }
    }
}


// for debugging
int getMaxHistoryScore() {
    int max = -1000;
    for (int from = 0; from < Constants::NUM_SQUARES; from++) {
        for (int to = 0; to < Constants::NUM_SQUARES; to++) {
            if (history_move_scores[from][to] > max) {
                max = history_move_scores[from][to];
            }
        }
    }
    return max;
}