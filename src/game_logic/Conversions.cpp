//
// Created by salom on 24.06.2025.
//

#include <array>
using std::array;
#include <stdexcept>
#include <algorithm>
#include <sstream>

#include "Conversions.h"
#include "Constants.h"
#include "MoveGeneration.h"


GameBoard convertFENtoGameBoard(std::string const & fen) {

    std::istringstream iss(fen);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {
        tokens.push_back(token);
    }
    array<uint64_t, 13> pieceInformation = convertStringToPieceInformation(tokens[0]);
    bool white = tokens[1] == "w";
    array<bool, 5> castleInformation = convertStringToCastleInformation(tokens[2]);
    int enPass = convertPositionStringToInt(tokens[3]);
    uint8_t ply = std::stoi(tokens[4]);
    uint8_t move = std::stoi(tokens[5]);

    return {pieceInformation, white, castleInformation, enPass, ply, move};
}

int convertPositionStringToInt(std::string const & position) {
    if (position.length() == 2 && isChessLetter(position[0]) && isChessNumber(position[1])) {
        return std::tolower(position[0])-'a' + 56-8*(position[1]-'1');
    }
    return -1;
}
std::string convertIntToPosition(uint8_t position) {
    auto file = static_cast<char>('a' + (position % 8));
    auto rank = static_cast<char>('8' - (position / 8));
    return {file, rank};
}

bool isChessLetter(char letter) {
    return std::tolower(letter)-'a' >= 0 && std::tolower(letter)-'a' < 8;
}
bool isChessNumber(char number) {
    return number - '1' >= 0 && number - '1' < 8;
}

array<bool, 5> convertStringToCastleInformation(std::string const & castles) {
    array<bool, 5> castleInformation = {false, false, false, false,false};
    using Constants::Castle;
    if (castles.find('Q') != std::string::npos) castleInformation[Castle::WHITE_QUEEN_SIDE_CASTLE] = true;
    if (castles.find('K') != std::string::npos) castleInformation[Castle::WHITE_KING_SIDE_CASTLE] = true;
    if (castles.find('q') != std::string::npos) castleInformation[Castle::BLACK_QUEEN_SIDE_CASTLE] = true;
    if (castles.find('k') != std::string::npos) castleInformation[Castle::BLACK_KING_SIDE_CASTLE] = true;
    return castleInformation;
}

array<uint64_t, 13> convertStringToPieceInformation(std::string const & pieces) {
    array<uint64_t, 13> pieceInformation = {};
    int i = 0;
    for (char p : pieces) {
        auto it = std::ranges::find(Constants::PIECE_LETTERS.begin(),Constants::PIECE_LETTERS.end(),p);
        if ( it != Constants::PIECE_LETTERS.end()) {
            pieceInformation[it-Constants::PIECE_LETTERS.begin()] |= (1ULL<<i++);
        } else if (p == '/') {
            //nothing
        } else if (isChessNumber(p)) {
            i += p - '0';
        } else {
            throw std::invalid_argument("Invalid character in FEN");
        }
    }
    return pieceInformation;
}

std::string toString(Constants::Piece piece) {
    switch (piece) {
        case Constants::Piece::WHITE_PAWN: return "White Pawn";
        case Constants::Piece::WHITE_QUEEN: return "White Queen";
        case Constants::Piece::BLACK_PAWN: return "Black Pawn";
        case Constants::Piece::BLACK_QUEEN: return "Black Queen";
        case Constants::Piece::WHITE_KING: return "White King";
        case Constants::Piece::BLACK_KING: return "Black King";
        case Constants::Piece::WHITE_ROOK: return "White Rook";
        case Constants::Piece::BLACK_ROOK: return "Black Rook";
        case Constants::Piece::WHITE_BISHOP: return "White Bishop";
        case Constants::Piece::BLACK_BISHOP: return "Black Bishop";
        case Constants::Piece::WHITE_KNIGHT: return "White Knight";
        case Constants::Piece::BLACK_KNIGHT: return "Black Knight";
        default: return "-";
    }
}

std::string toString(Constants::Castle castle) {
    switch (castle) {
        case Constants::Castle::NO_CASTLE: return "-";
        case Constants::Castle::WHITE_QUEEN_SIDE_CASTLE:
        case Constants::Castle::BLACK_QUEEN_SIDE_CASTLE: return "O-O-O";
        case Constants::Castle::WHITE_KING_SIDE_CASTLE:
        case Constants::Castle::BLACK_KING_SIDE_CASTLE: return "O-O";
    }
    return "-";
}


std::string longAlgebraicNotation(Move move) {
    std::string result = convertIntToPosition(move.from()) + convertIntToPosition(move.to());
    if (move.promotion()) {
        result += (Constants::PROMOTION_STRING[move.promotion()]);
    }
    return result;
}

Move longAlgebraicNotationToMove(std::string const & move, GameBoard const & board) {
    unsigned from = std::tolower(move[0])-'a' + 56-8*(move[1]-'1');
    unsigned to = std::tolower(move[2])-'a' + 56-8*(move[3]-'1');

    // promotion
    Constants::Piece pawn_promotion = Constants::Piece::NONE;
    if (move.length() == 5) {
        if (move[4] == 'q') {
            pawn_promotion = board.whiteToMove ? Constants::Piece::WHITE_QUEEN : Constants::Piece::BLACK_QUEEN;
        } else if (move[4] == 'r') {
            pawn_promotion = board.whiteToMove ? Constants::Piece::WHITE_ROOK : Constants::Piece::BLACK_ROOK;
        } else if (move[4] == 'b') {
            pawn_promotion = board.whiteToMove ? Constants::Piece::WHITE_BISHOP : Constants::Piece::BLACK_BISHOP;
        } else if (move[4] == 'n') {
            pawn_promotion = board.whiteToMove ? Constants::Piece::WHITE_KNIGHT : Constants::Piece::BLACK_KNIGHT;
        }
    }

    Constants::Piece piece = getPieceAt(board,from,board.whiteToMove);
    Constants::Piece capture = getPieceAt(board,to,!board.whiteToMove);

    // en passant
    if (piece == Constants::Piece::WHITE_PAWN && to == board.enPassant) capture = Constants::Piece::BLACK_PAWN;
    else if (piece == Constants::BLACK_PAWN && to == board.enPassant) capture = Constants::Piece::WHITE_PAWN;

    // castle
    Constants::Castle castle = Constants::Castle::NO_CASTLE;
    if (piece == Constants::WHITE_KING && to == 62 && from == 60) castle = Constants::Castle::WHITE_KING_SIDE_CASTLE;
    else if (piece == Constants::WHITE_KING && to == 58 && from == 60) castle = Constants::Castle::WHITE_QUEEN_SIDE_CASTLE;
    else if (piece == Constants::BLACK_KING && to == 6 && from == 4) castle = Constants::Castle::BLACK_KING_SIDE_CASTLE;
    else if (piece == Constants::BLACK_KING && to == 2 && from == 4) castle = Constants::Castle::BLACK_QUEEN_SIDE_CASTLE;

    return {piece,from,capture,to,pawn_promotion,castle};
}
