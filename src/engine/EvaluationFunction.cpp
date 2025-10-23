//
// Created by salom on 16.07.2025.
//


#include "EvaluationFunction.h"

#include "EvaluationConstants.h"
#include "PieceSquareTables.h"

#include "GameBoard.h" //lib game_logic
#include "MoveGenerationConstants.h"
#include "Network.h"

#include "Utils.h" // lib utils

// returns a heuristic value of how good the position is
// large and positive ~ good for the side to move
// uses PeSTO or
int evaluate(GameBoard const & board) {
    if (network_weights.loaded_ok) {
        return nnue.evaluate(board.whiteToMove);
    }
    pst_evaluation_accumulator.reset_to(board);
    int gps = pst_evaluation_accumulator.game_phase_score;
    int mg_evaluation = pst_evaluation_accumulator.mg_evaluation;
    int eg_evaluation = pst_evaluation_accumulator.eg_evaluation;
    int evaluation = (gps*mg_evaluation + (24-gps)*eg_evaluation)/24;
    return board.whiteToMove ? evaluation : -evaluation;
}

//measurement for game phase (middle game: 24, late end game: 0)
int getGamePhaseScore(GameBoard const &board) {
    int score = 0;
    score += 4*popcountll(board.pieces[Constants::WHITE_QUEEN] | board.pieces[Constants::BLACK_QUEEN]);
    score += 2*popcountll(board.pieces[Constants::WHITE_ROOK] | board.pieces[Constants::BLACK_ROOK]);
    score += popcountll(board.pieces[Constants::WHITE_KNIGHT] | board.pieces[Constants::BLACK_KNIGHT]);
    score += popcountll(board.pieces[Constants::WHITE_BISHOP] | board.pieces[Constants::BLACK_BISHOP]);
    return score;
}

void Accumulator::reset_to(GameBoard const &board) {
    mg_evaluation = 0;
    eg_evaluation = 0;
    bool piece_is_white = true;
    for (unsigned piece_type = 1; piece_type < 13; ++piece_type) {
        int sign = piece_is_white ? 1 : -1;
        forEachPiece(static_cast<Constants::Piece>(piece_type),board,[&](unsigned position, GameBoard const & game_board) {

            // PST
            mg_evaluation += sign * MG_PST[piece_type][position];
            eg_evaluation += sign * EG_PST[piece_type][position];

        });
        piece_is_white = !piece_is_white;
    }
    game_phase_score = getGamePhaseScore(board);
}

void Accumulator::update(Move move, int en_passant, bool unmake) {
    bool piece_is_white = move.piece() % 2 == 1;
    int piece_sign = piece_is_white ? 1 : -1;
    int unmake_sign = unmake ? -1 : 1;

    // remove piece from square
    mg_evaluation -= piece_sign * unmake_sign * MG_PST[move.piece()][move.from()];
    eg_evaluation -= piece_sign * unmake_sign * EG_PST[move.piece()][move.from()];

    // place piece on target square
    auto piece= move.piece();
    if (move.promotion()) {
        piece = move.promotion();
        game_phase_score += unmake_sign * GAME_PHASE_SCORE_WEIGHTS[piece];
    }
    mg_evaluation += piece_sign * unmake_sign * MG_PST[piece][move.to()];
    eg_evaluation += piece_sign * unmake_sign * EG_PST[piece][move.to()];

    // remove captured piece
    if (move.capture()) {
        int offset = (move.to() == en_passant ? (piece_is_white ? Constants::SOUTH : Constants::NORTH) : 0);
        mg_evaluation -= piece_sign * unmake_sign * MG_PST[move.capture()][move.to()+offset];
        eg_evaluation -= piece_sign * unmake_sign * EG_PST[move.capture()][move.to()+offset];

        game_phase_score -= unmake_sign * GAME_PHASE_SCORE_WEIGHTS[move.capture()];
    }
    // move rook in case of castling
    if (move.castle()) {
        piece = piece_is_white ? Constants::WHITE_ROOK : Constants::BLACK_ROOK;
        unsigned from = counttzll(castle_rook_positions[move.castle()][unmake]);
        unsigned to = counttzll(castle_rook_positions[move.castle()][!unmake]);

        // no unmake sign because the positions from and to are already swapped
        mg_evaluation -= piece_sign * MG_PST[piece][from];
        eg_evaluation -= piece_sign * EG_PST[piece][from];

        mg_evaluation += piece_sign * MG_PST[piece][to];
        eg_evaluation += piece_sign * EG_PST[piece][to];
    }
}