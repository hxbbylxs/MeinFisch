//
// Created by salom on 26.07.2025.
//
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "engineUCI.h"

#include "MoveGeneration.h"
#include "Memory.h"
#include "Output.h"
#include "Engine.h"
#include "Conversions.h"

void engineUCI::receiveCommand(std::string const & message) {
    if (message == "uci") {
        std::cout << "id name MeinFisch" << std::endl;
        std::cout << "id author hxbbylxs" << std::endl;
        std::cout << "uciok" << std::endl;
    } else if (message == "ucinewgame") {
        initializeHistoryHeuristic();
        killer_moves = {};
        counter_moves = {};
        clearTT();
        global_board = GameBoard();
    } else if (message == "isready") {
        std::cout << "readyok" << std::endl;
    } else if (message.rfind("position",0) == 0) {
        global_board = convertInputPositionToGameBoard(message);
    } else if (message.rfind("go",0) == 0) {
        std::cout << calcBestMove(message) << std::endl;
    } else if (message == "stop") {

    }
}

GameBoard engineUCI::convertInputPositionToGameBoard(std::string const & input) {
    std::istringstream iss(input);
    std::string token;
    std::vector<std::string> tokens;

    // Zerlege message in Tokens
    while (iss >> token) {
        tokens.push_back(token);
    }

    GameBoard board;
    size_t idx = 1; // skip "position"

    if (tokens[idx] == "startpos") {
        board = GameBoard(); // standard start position
        idx++;

    } else if (tokens[idx] == "fen") {
        if (idx + 6 >= tokens.size()) {
            std::cerr << "Error: incomplete FEN" << std::endl;
            return {};
        }

        // Füge die nächsten 6 Tokens zur FEN-Zeichenkette zusammen
        std::ostringstream fenStream;
        for (size_t i = 1; i <= 6; ++i) {
            fenStream << tokens[idx + i];
            if (i < 6) fenStream << " ";
        }

        std::string fen = fenStream.str();

        board = convertFENtoGameBoard(fen);
        idx += 7;
    } else {
        std::cerr << "Error: unknown position type" << std::endl;
        return {};
    }

    // Check for "moves"
    if (idx < tokens.size() && tokens[idx] == "moves") {
        idx++;
        while (idx < tokens.size()) {
            makeMove(tokens[idx],board);
            idx++;
        }
    }

    return board;
}

std::string engineUCI::calcBestMove(std::string const & go) {
    std::istringstream iss(go);
    std::string token;

    // Karte zur Speicherung aller Werte wie wtime, btime usw.
    std::unordered_map<std::string, int> uciParams;

    // "go" steht am Anfang → überspringen
    iss >> token;

    // Lese Paare wie "wtime 300000"
    while (iss >> token) {
        int value = 0;
        if (iss >> value) {
            uciParams[token] = value;
        }
    }

    // Extrahiere Zeitangaben, falls vorhanden
    int wtime = uciParams.contains("wtime") ? uciParams["wtime"] : 9000000;
    int btime = uciParams.contains("btime") ? uciParams["btime"] : 9000000;
    int winc  = uciParams.contains("winc")  ? uciParams["winc"]  : 0;
    int binc  = uciParams.contains("binc")  ? uciParams["binc"]  : 0;
    int movestogo = uciParams.contains("movestogo") ? uciParams["movestogo"] : -1;
    int max_depth = uciParams.contains("depth") ? uciParams["depth"] : Constants::MAX_RECURSION_DEPTH;


    int estimated_remaining_moves = std::max(10,60-global_board.moves);
    int searchTime = static_cast<int>(std::min(15000.0,global_board.whiteToMove ? (2.0*wtime/estimated_remaining_moves + 0.8*winc) : (2.0*btime/estimated_remaining_moves + 0.8*binc)));

    auto res = iterativeDeepening(global_board, searchTime, max_depth);

    return "bestmove "+longAlgebraicNotation(res.first);
}

void engineUCI::makeMove(std::string const & move, GameBoard & gameBoard) {
    unsigned from = std::tolower(move[0])-'a' + 56-8*(move[1]-'1');
    unsigned to = std::tolower(move[2])-'a' + 56-8*(move[3]-'1');
    Constants::Piece pawn_promotion = Constants::Piece::NONE;
    if (move.length() == 5) {
        if (move[4] == 'q') {
            pawn_promotion = gameBoard.whiteToMove ? Constants::Piece::WHITE_QUEEN : Constants::Piece::BLACK_QUEEN;
        } else if (move[4] == 'r') {
            pawn_promotion = gameBoard.whiteToMove ? Constants::Piece::WHITE_ROOK : Constants::Piece::BLACK_ROOK;
        } else if (move[4] == 'b') {
            pawn_promotion = gameBoard.whiteToMove ? Constants::Piece::WHITE_BISHOP : Constants::Piece::BLACK_BISHOP;
        } else if (move[4] == 'n') {
            pawn_promotion = gameBoard.whiteToMove ? Constants::Piece::WHITE_KNIGHT : Constants::Piece::BLACK_KNIGHT;
        }
    }
    Constants::Piece piece = getPieceAt(gameBoard,from,gameBoard.whiteToMove);
    Constants::Piece capture = getPieceAt(gameBoard,to,!gameBoard.whiteToMove);
    if (piece == Constants::Piece::WHITE_PAWN && to == gameBoard.enPassant) capture = Constants::Piece::BLACK_PAWN;
    else if (piece == Constants::BLACK_PAWN && to == gameBoard.enPassant) capture = Constants::Piece::WHITE_PAWN;
    Constants::Castle castle = Constants::Castle::NO_CASTLE;
    if (piece == Constants::WHITE_KING && to == 62 && from == 60) castle = Constants::Castle::WHITE_KING_SIDE_CASTLE;
    else if (piece == Constants::WHITE_KING && to == 58 && from == 60) castle = Constants::Castle::WHITE_QUEEN_SIDE_CASTLE;
    else if (piece == Constants::BLACK_KING && to == 6 && from == 4) castle = Constants::Castle::BLACK_KING_SIDE_CASTLE;
    else if (piece == Constants::BLACK_KING && to == 2 && from == 4) castle = Constants::Castle::BLACK_QUEEN_SIDE_CASTLE;

    Move mv = {piece,from,capture,to,pawn_promotion,castle};

    gameBoard.applyPseudoLegalMove(mv);
}
