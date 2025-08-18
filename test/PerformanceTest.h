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




#endif //PERFORMANCETEST_H
