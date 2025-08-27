
#include <chrono>
#include <iostream>
#include <Memory.h>
#include <Output.h>
#include <PerformanceTest.h>


#include "EngineUCI.h"

#include "Constants.h"
#include "EvaluationConstants.h"
#include "MoveGenerationConstants.h"

#include "Test.h"

int main() {

    initializeZobristHashValues();
    initializeSliderAttackBitMask(); //TODO at compiletime
    initializeHistoryHeuristic();


    engineUCI engine = engineUCI();
    std::string input;

    while (input != "quit") {
        std::getline(std::cin, input);
        engine.receiveCommand(input);
    }

    return 0;
}