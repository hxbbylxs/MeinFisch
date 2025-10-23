//
// Created by salom on 13.07.2025.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <array>
#include <utility>
#include <atomic>

#include "GameBoard.h"
#include "Move.h"
#include "Movepicking.h"

enum class NodeType {
    PV_NODE,
    CUT_NODE,
    ALL_NODE
};
inline std::atomic<bool> stop_search(false);
inline std::atomic<bool> search_ongoing(false);

inline std::atomic<bool> lmr_active(true);
inline std::atomic<bool> nmp_active(true);
inline std::atomic<bool> razoring_active(true);

inline std::array<int,2*Constants::MAX_RECURSION_DEPTH> nodes_at_depth = {};
inline std::array<int,2*Constants::MAX_RECURSION_DEPTH> qnodes_at_depth = {};
inline std::array<int,100> cutoffs_at_move_number_cutnodes = {};
inline std::array<int,100> cutoffs_at_move_number_allnodes = {};
inline std::array<int,NUM_MOVE_GEN_PHASES> cutoffs_at_move_gen_phase = {};
inline std::array<int,2*Constants::MAX_RECURSION_DEPTH> pv_changes_at_depth = {};
inline int total_researches = 0;
inline int nmp_prunes = 0;
inline int razoring_prunes = 0;
inline int exact_tt_hits = 0;
inline int bound_tt_hits = 0;
inline int qsearch_tt_hits = 0;

void startTimeLimit(int timeLimit);
void playerControlledTimeLimit(int ignore);

void iterativeDeepening(GameBoard & board, int timeLimit, int max_depth);
std::pair<Move,int> getOptimalMoveNegaMax(GameBoard & board, int remaining_depth);
int negaMax(GameBoard  & board, int remaining_depth, int alpha, int beta, int depth, Move previous_move, bool null_move_allowed, NodeType node_type); // searches all moves (expected bad ones lower depth)
int quiscenceSearch(GameBoard & board, int remaining_depth, int alpha, int beta, int depth); // searches only captures

std::string reconstructPV(GameBoard & board);




#endif //ENGINE_H
