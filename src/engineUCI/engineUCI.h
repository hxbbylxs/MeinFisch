//
// Created by salom on 26.07.2025.
//

#ifndef ENGINEUCI_H
#define ENGINEUCI_H

#include "GameBoard.h"
#include <string>

class engineUCI {
    GameBoard convertInputPositionToGameBoard(std::string const & input);
    std::string calcBestMove(std::string const & go);
    void makeMove(std::string const & move, GameBoard & gameBoard);
    public:
    void receiveCommand(std::string const & message);
};



#endif //ENGINEUCI_H
