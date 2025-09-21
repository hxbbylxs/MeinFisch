//
// Created by salom on 17.07.2025.
//

#ifndef MEMORY_H
#define MEMORY_H

#include <array>
#include <cstdint>

#include "GameBoard.h" // lib game_logic

// Global Game Board

inline GameBoard global_board;

//Transposition Table
enum Evaluation_Flag {
    EMPTY,
    LOWER_BOUND,
    UPPER_BOUND,
    EXACT
};

struct Data {
    uint64_t zobristHash;
    Evaluation_Flag evaluationFlag;
    int depth;
    int evaluation;
    uint32_t bestMove;
    uint8_t age;
    Data(uint64_t zobristHash, Evaluation_Flag evaluation_flag, int depth, int evaluation,     uint32_t bestMove )
        : zobristHash(zobristHash), evaluationFlag(evaluation_flag), depth(depth), evaluation(evaluation), bestMove(bestMove),age(depth) {}
    Data()
        : zobristHash(0), evaluationFlag(EMPTY), depth(-1), evaluation(0), bestMove(0), age(10)
    {
    }
};

inline std::array<Data,Constants::TTSIZE> transposition_table = {};

void tryMakeNewEntry(Evaluation_Flag evaluation_flag, int depth, int evaluation,uint32_t bestMove, GameBoard const & board);
bool newEntryIsBetter(int depth, int index);
Data& getData(uint64_t hash);

int updateReturnValue(int evaluation);
int updateAlphaBetaValue(int alphabeta);

void clearTT();

// Killer Moves
inline std::array<uint32_t,Constants::MAX_RECURSION_DEPTH> killer_moves = {};

// Counter Moves
inline std::array<std::array<uint32_t,Constants::NUM_SQUARES>,Constants::NUM_SQUARES> counter_moves = {};

// History Heuristic
// assigns a score to a move based on [from][to] for move ordering
// when a move causes a cutoff the value at [from][to] is increased
inline std::array<std::array<int,Constants::NUM_SQUARES>,Constants::NUM_SQUARES> history_move_scores = {};

void increaseMoveScore(uint32_t move, int depth);
void decreaseAllMoveScores();
void initializeHistoryHeuristic();
int getMaxHistoryScore();



#endif //MEMORY_H
