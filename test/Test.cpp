//
// Created by salom on 20.07.2025.
//

#include "Test.h"

#include "GameBoard.h" //lib game_logic
#include "Conversions.h"
#include "MoveGeneration.h"

#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <Engine.h>
#include <Output.h>
#include <Memory.h>

#include "Movepicking.h"

//4k2r/Ppq5/8/R2bpP2/n5P1/3r1B1N/1pP1Q3/R3K3 w Qk e6 0 1
// testing only this particular FEN
void testMoveGeneration() {
    GameBoard board = convertFENtoGameBoard("4k2r/Ppq5/8/R2bpP2/n5P1/3r1B1N/1pP1Q3/R3K3 w Qk e6 0 1");
    std::vector<uint32_t> whiteMoves = getPseudoLegalMoves(board, true,ALL);
    std::vector<uint32_t> blackMoves = getPseudoLegalMoves(board, false,ALL);

    std:: vector<uint32_t> whiteCorrectSolution = { 9437313, 7340161, 5243009, 3145857,
                                                    330193, 344529, 492129, 713505,
                                                    688929, 557857, 449237, 590549,
                                                    885461, 1032917, 262535, 409991,
                                                    426375, 448903, 528775, 529287,
                                                    656263, 787335, 934791, 951175,
                                                    967559, 461641, 590665, 713545,
                                                    721737, 836425, 869193, 885577,
                                                    901961, 967497, 1000265, 492275,
                                                    606963, 869107, 1016563, 836555,
                                                    869323, 967627, 1000395, 17728459};

    std::vector<uint32_t> blackCorrectSolution = {  278674, 409746, 590274, 11411218,
                                                    9314066, 7216914, 5119762, 11420434,
                                                    9323282, 7226130, 5128978, 98742,
                                                    213430, 295350, 328118, 557494,
                                                    590262, 672182, 742838, 786870,
                                                    82040, 98424, 245880, 376952,
                                                    508024, 639096, 773240, 574136,
                                                    656056, 672440, 688824, 721592,
                                                    743096, 836280, 967352, 16554,
                                                    32938, 49322, 180394, 196778,
                                                    213162, 229546, 245930, 278698,
                                                    295082, 311466, 400554, 426154,
                                                    557226, 688298, 820394, 279044,
                                                    426500, 688644, 49228, 81996,
                                                    180300, 196684, 213068, 67207244};
    if (whiteCorrectSolution.size() != whiteMoves.size()) {
        std::cout << "Test failed: move generation" << std::endl;
        return;
    }
    for (uint32_t move : whiteMoves) {
        auto it = find(whiteCorrectSolution.begin(), whiteCorrectSolution.end(), move);
        if (it == whiteCorrectSolution.end()) {
            std::cout << "Test failed: move generation" << std::endl;
            return;
        }
    }
    if (whiteCorrectSolution.size() != whiteMoves.size()) {
        std::cout << "Test failed: move generation" << std::endl;
        return;
    }
    for (uint32_t move : blackMoves) {
        auto it = find(blackCorrectSolution.begin(), blackCorrectSolution.end(), move);
        if (it == blackCorrectSolution.end()) {
            std::cout << "Test failed: move generation" << std::endl;
            return;
        }
    }
    std::cout << "Test passed: move generation" << std::endl;
}



//3rk3/4q3/8/8/6nb/4P3/5P2/R3K2R w KQq - 0 1
void testLegalMoveCheck() {

    GameBoard board = convertFENtoGameBoard("3rk3/4q3/8/8/6nb/4P3/5P2/R3K2R w KQq - 0 1");

    std::vector<uint32_t> expectedIllegal = {738129,607057,836555,967627,17728459};
    std::vector<uint32_t> expectedLegal = {852939,1000395,34571211,771063};

    for (auto move : expectedLegal) {
        if (!isLegalMove(move,board)) {
            std::cout << "Test failed: legal move" << std::endl;
            return;
        }
    }
    for (auto move : expectedIllegal) {
        if (isLegalMove(move,board)) {
            std::cout << "Test failed: illegal move" << std::endl;
            return;
        }
    }
    std::cout << "Test passed: legal move detection" << std::endl;

}

//r1b1r1k1/1p1nb3/2pp2PQ/q1nPp3/p1P1P3/2N2P2/PP6/1K1R1BNR b - - 0 18
void testBlackBoxEngine() {

    GameBoard board = convertFENtoGameBoard("r1b1r1k1/1p1nb3/2pp2PQ/q1nPp3/p1P1P3/2N2P2/PP6/1K1R1BNR b - - 0 18");
    int expected = 99980;
    auto actual = iterativeDeepening(board,5,30);
    if (actual.second != expected) {
        std::cout << "Test failed: checkmate detection" << std::endl;
        return;
    }
    std::cout << "Test passed: checkmate detection" << std::endl;
}


//4k2r/Ppq5/8/R2bpP2/n5P1/3r1B1N/1pP1Q3/R3K3 w Qk e6 0 1 before move
void testZobristHashUpdate() {

    GameBoard board = convertFENtoGameBoard("4k2r/Ppq5/8/R2bpP2/n5P1/3r1B1N/1pP1Q3/R3K3 w Qk e6 0 1");
    uint64_t zobristHash_before = board.zobristHash;

    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant_ = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    auto psm = getPseudoLegalMoves(board,true,ALL);
    for (auto move : psm) {
        board.zobristHash = zobristHash_before;
        board.applyPseudoLegalMove(move);
        board.unmakeMove(move,enPassant_,castle_rights,plies,hash_before);
        if (board.zobristHash != zobristHash_before) {
            std::cout << "Test failed: zobrist hash update" << std::endl;
            return;
        }
    }
    std::cout << "Test passed: zobrist hash update" << std::endl;
}

//4k2r/Ppq5/8/R2bpP2/n5P1/3r1B1N/1pP1Q3/R3K3 w Qk e6 0 1 before move
void testUnmakeMove() {

    GameBoard board = convertFENtoGameBoard("4k2r/Ppq5/8/R2bpP2/n5P1/3r1B1N/1pP1Q3/R3K3 w Qk e6 0 1");
    GameBoard board_before = board;

    getOptimalMoveNegaMax(board,4);
    if (!(board == board_before)) {
        std::cout << "Test failed: unmaking moves" << std::endl;
        return;
    }

    std::cout << "Test passed: unmaking moves" << std::endl;
}

void testSEE() {
    GameBoard board = convertFENtoGameBoard("rnbqkb1r/pppppp1p/5n2/6p1/5P2/7N/PPPPP1PP/RNBQKB1R w KQkq - 0 1");
    printGameBoard(board);
    auto psm = getPseudoLegalMoves(board,board.whiteToMove,ALL);
    for (auto move : psm) {
        printCompleteMove(decodeMove(move));
        std::cout << "SEE: " << static_exchange_evaluation(move,board) << std::endl;
    }
}
