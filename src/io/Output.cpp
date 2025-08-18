//
// Created by salom on 24.06.2025.
//
#include <chrono>
#include <iostream>
using std::cout;
using std::endl;
#include <format>

#include "Output.h"

#include "Constants.h" //lib constants
#include "EvaluationConstants.h"

#include "GameBoard.h" // lib game_logic
#include "Move.h"
#include "Conversions.h"

void printGameBoard(GameBoard const & board){
    using Constants::Piece;
    uint64_t allPieces = 0;
    for (uint64_t piece : board.pieces) allPieces |= piece;
    for (int i= 0; i < 64; i++) {
        if (i%8 == 0) cout << "+---+---+---+---+---+---+---+---+" << endl;
        cout << "| ";
        if (allPieces & (1ULL << i)) {
            if (board.pieces[Piece::WHITE_PAWN] & (1ULL << i)) cout << "P ";
            if (board.pieces[Piece::WHITE_BISHOP] & (1ULL << i)) cout << "B ";
            if (board.pieces[Piece::WHITE_KNIGHT] & (1ULL << i)) cout << "N ";
            if (board.pieces[Piece::WHITE_ROOK] & (1ULL << i)) cout << "R ";
            if (board.pieces[Piece::WHITE_QUEEN] & (1ULL << i)) cout << "Q ";
            if (board.pieces[Piece::WHITE_KING] & (1ULL << i)) cout << "K ";
            if (board.pieces[Piece::BLACK_PAWN] & (1ULL << i)) cout << "p ";
            if (board.pieces[Piece::BLACK_BISHOP] & (1ULL << i)) cout << "b ";
            if (board.pieces[Piece::BLACK_KNIGHT] & (1ULL << i)) cout << "n ";
            if (board.pieces[Piece::BLACK_ROOK] & (1ULL << i)) cout << "r ";
            if (board.pieces[Piece::BLACK_QUEEN] & (1ULL << i)) cout << "q ";
            if (board.pieces[Piece::BLACK_KING] & (1ULL << i)) cout << "k ";
        } else {
            cout << "  ";
        }
        if (i%8 == 7) cout << "| " << (63-i)/8 +1 << endl; // end of row
    }
    cout << "+---+---+---+---+---+---+---+---+" << endl;
    cout << "  a   b   c   d   e   f   g   h  " << endl;
}

void printPlayerToMove(bool whiteToMove) {
    std::cout << (whiteToMove ? "White" : "Black") << " to move" << std::endl;
}

// prints the bit representation of the game board or any 64bit unsigned
// used for debugging
void printBitBoard(uint64_t number) {
    for (int i= 0; i < 64; i++) {
        cout << ((number & (1ULL << i))?1:0);
        if (i%8 == 7) cout << endl;
    }
    cout << endl;
}

// prints detailed information about a move
// IMPORTANT: the check flag is not set when generating a move. I might want to discard it because it makes more sense
//              to only test if the move is legal when I actually need it as it is an expensive operation
void printCompleteMove(Move const & move) {
    cout << endl;
    cout << "Piece: " << toString(move.piece) << endl;
    cout << "From: " << convertIntToPosition(move.from) << endl;
    cout << "To: " << convertIntToPosition(move.to) << endl;
    cout << "Capture: " << toString(move.captured_piece) << endl;
    cout << "Promotion: " << toString(move.pawn_promote_to) << endl;
    cout << "Castle: " << toString(move.castle) << endl;
    cout << "Check: " << (move.check?"true":"false") << endl;
}


std::string evaluationToString(int evaluation) {
    if (abs(evaluation) > CHECKMATE_VALUE - 1000) {
        std::string result = "mate ";
        if (evaluation < 0) result += "-";
        result += std::to_string((CHECKMATE_VALUE-abs(evaluation)+1)/2);
        return result;
    }
    return "cp " + std::to_string(evaluation);
}

void printAnalysisData(std::pair<uint32_t,int> const & move, int depth, int seldepth, std::chrono::time_point<std::chrono::system_clock> start, int nodes) {
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
    int time = duration.count();
    auto const & [mv,evaluation] = move;
    cout << "info depth " << depth << " seldepth " << seldepth << " score " << evaluationToString(evaluation)
            << "  pv "   << convertIntToPosition((mv&Constants::move_decoding_bitmasks[Constants::MoveDecoding::FROM])>>4)
                            << convertIntToPosition((mv&Constants::move_decoding_bitmasks[Constants::MoveDecoding::TO])>>14) << " time " << time << " nodes " << nodes << " nps " << (time != 0 ? (nodes/time)*1000 : 0) <<endl;
}



