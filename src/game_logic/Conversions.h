//
// Created by salom on 24.06.2025.
//

#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include <array>
#include <string>

#include "GameBoard.h"
#include "Constants.h"


GameBoard convertFENtoGameBoard(std::string const & fen); // throws exception

int convertPositionStringToInt(std::string const & enPassant);
std::string convertIntToPosition(uint8_t position);
bool isChessLetter(char letter);
bool isChessNumber(char number);
std::array<bool, 5> convertStringToCastleInformation(std::string const & castles);
std::array<uint64_t, 13> convertStringToPieceInformation(std::string const & pieces);
std::string toString(Constants::Piece piece);
std::string toString(Constants::Castle castle);
std::string longAlgebraicNotation(Move move);


#endif //CONVERSIONS_H
