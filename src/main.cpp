
#include <chrono>
#include <Conversions.h>
#include <EvaluationFunction.h>

#include <iostream>
#include <Memory.h>
#include <MoveGeneration.h>
#include <Output.h>
#include <PerformanceTest.h>


#include "engineUCI.h"

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
        try {
            engine.receiveCommand(input);
        } catch (const std::bad_alloc& e) {
            std::cout << e.what() << std::endl;
        }
        catch (const std::system_error& e) {
            std::cout << e.what() << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }

    return 0;
}