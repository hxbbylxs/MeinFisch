//
// Created by salom on 26.07.2025.
//

#ifndef ENGINEUCI_H
#define ENGINEUCI_H

#include "GameBoard.h"
#include <string>

class engineUCI {
    static GameBoard convertInputPositionToGameBoard(std::string const & input);
    static void calcBestMove(std::string const & go);
    static void makeMove(std::string const & move, GameBoard & gameBoard);
    public:
    static void receiveCommand(std::string const & message);
};



#endif //ENGINEUCI_H
