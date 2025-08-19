//
// Created by salom on 01.08.2025.
//

#ifndef PERFORMANCETEST_H
#define PERFORMANCETEST_H
#include <GameBoard.h>

void perft(int position);
void perftPruning(int position);
int testMinimax(int maxRecursionDepth, GameBoard & board);
void testCaptureMoveGen();
void testEval();
void testMoveGenPerformance();
void debugTest();
int testNegamax(int maxRecursionDepth, GameBoard & board, int alpha, int beta);




#endif //PERFORMANCETEST_H
