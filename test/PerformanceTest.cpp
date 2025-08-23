//
// Created by salom on 01.08.2025.
//

#include "PerformanceTest.h"

#include "Constants.h"

#include "GameBoard.h"
#include "MoveGeneration.h"

#include "Engine.h"
#include "EvaluationFunction.h"
#include "Memory.h"

#include <array>
#include <iostream>
#include <chrono>
#include <Conversions.h>

#include "EvaluationConstants.h"
#include "Movepicking.h"

std::array<int,Constants::MAX_RECURSION_DEPTH> nodes_at_depth = {};
std::array<std::string,4> testFENs = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  "
};
std::array<int,4> testDepths = {5,4,5,4};

void perft(int position) {
    nodes_at_depth = {};
    GameBoard board = convertFENtoGameBoard(testFENs[position-1]);
    int maxDepth = testDepths[position-1];
    auto start = std::chrono::high_resolution_clock::now();
    int eval = testMinimax(maxDepth,board);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    for (int i = maxDepth; i >= 0; i--) {
        std::cout << "depth: " << maxDepth - i << " number of nodes: " << nodes_at_depth[i] << std::endl;
    }
    std::cout << "Total time: " << duration.count() << " ms" << std::endl;
    std::cout << "Evaluation: " << eval << std::endl;
}

void debugTest() {
    std::string criticalFEN = "6k1/rppqbrpp/p3Nn2/3pBP2/2nP4/PPNQP2P/2P5/R4RK1 b - - 0 19";
    GameBoard board = convertFENtoGameBoard(criticalFEN);
    for (int i = 1; i <= 7; i++) {

        int result = testNegamax(i,board,-145,-130);
        std::cout << "depth: " << i << " cp: " << result << std::endl;
    }

}

int testMinimax(int maxRecursionDepth, GameBoard & board) {
    nodes_at_depth[maxRecursionDepth]++;
    if (maxRecursionDepth <= 0) return quiscenceSearch(board,maxRecursionDepth,-CHECKMATE_VALUE,CHECKMATE_VALUE,0);
    auto psm = getPseudoLegalMoves(board, board.whiteToMove,ALL);

    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    int max = -CHECKMATE_VALUE;

    for (uint32_t move : psm) {
        if (!isLegalMove(move, board)) continue;
        board.applyPseudoLegalMove(move);
        max = std::max(max,-testMinimax(maxRecursionDepth-1, board));
        board.unmakeMove(move,enPassant,castle_rights,plies,hash_before);
    }
    return max;
}

int testNegamax(int maxRecursionDepth, GameBoard &board, int alpha, int beta) {
    nodes_at_depth[maxRecursionDepth]++;
    if (maxRecursionDepth <= 0) return quiscenceSearch(board,maxRecursionDepth,alpha,beta,0);
    auto psm = getPseudoLegalMoves(board, board.whiteToMove,ALL);
    mvv_lva_MoveOrdering(psm);

    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    int max = -CHECKMATE_VALUE;

    for (uint32_t move : psm) {
        if (!isLegalMove(move, board)) continue;
        board.applyPseudoLegalMove(move);
        max = std::max(max,-testNegamax(maxRecursionDepth-1, board, -beta,-alpha));
        board.unmakeMove(move,enPassant,castle_rights,plies,hash_before);
    }
    return max;
}


void perftPruning(int position) {
    clearTT();
    global_evaluated_positions_counter = {};
    evaluated_positions_in_quiscence = {};
    GameBoard board = convertFENtoGameBoard(testFENs[position-1]);
    int maxDepth = position == 2 ? 5 : 8;
    auto start = std::chrono::high_resolution_clock::now();
    auto eval = getOptimalMoveNegaMax(board,maxDepth);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    for (int i = maxDepth; i >= 0; i--) {
        std::cout << "depth: " << maxDepth - i << " number of nodes: " << global_evaluated_positions_counter[i] << std::endl;
    }
    std::cout << "in Quiescence: " << std::endl;
    for (int i = 0; evaluated_positions_in_quiscence[i] > 0; i++) {
        std::cout << "depth: " << i << " number of nodes: " << evaluated_positions_in_quiscence[i] << std::endl;
    }

    std::cout << "Total time: " << duration.count() << " ms" << std::endl;
    std::cout << "Evaluation: " << eval.second << std::endl;
}

void testCaptureMoveGen() {
    GameBoard board = convertFENtoGameBoard(testFENs[1]);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        auto psm = getPseudoLegalMoves(board,true,CAPTURES);
        auto psm2 = getPseudoLegalMoves(board,false,CAPTURES);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Total time: " << duration.count() << " ms" << std::endl;
}

void testEval() {
    GameBoard board = convertFENtoGameBoard(testFENs[1]);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        int a = evaluate(board,-CHECKMATE_VALUE,CHECKMATE_VALUE);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Total time: " << duration.count() << " ms" << std::endl;
}

void testMoveGenPerformance() {    GameBoard board = convertFENtoGameBoard(testFENs[1]);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000000; i++) {
        auto psm = getPseudoLegalMoves(board,true,CAPTURES);
        auto psm2 = getPseudoLegalMoves(board,true,QUIETS);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Total time (1mio Move Gens): " << duration.count() << " ms" << std::endl;
}