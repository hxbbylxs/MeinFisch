//
// Created by salom on 16.07.2025.
//

#include "EvaluationFunction.h"
#include "PieceSquareTables.h"

#include "GameBoard.h" //lib game_logic

// returns a heuristic value of how good the position is
// large and positive ~ good for the side to move
// uses PeSTO
int evaluate(GameBoard const & board) {

    int game_phase_score = getGamePhaseScore(board); // used for interpolation between middle game and end game

    int mg_evaluation = 0;
    int eg_evaluation = 0;

    for (Constants::Piece piece_type : Constants::piece_decoding) {
        if (piece_type == Constants::NONE) continue;
        bool piece_is_white = (piece_type % 2 == 1);
        int sign = piece_is_white ? 1 : -1;
        forEachPiece(piece_type,board,[&](unsigned position, GameBoard const & game_board) {

            // PST
            mg_evaluation += sign * MG_PST[piece_type][position];
            eg_evaluation += sign * EG_PST[piece_type][position];

        });
    }

    // linear interpolation between middle game and end game
    int evaluation = (game_phase_score*mg_evaluation + (24-game_phase_score)*eg_evaluation)/24;

    // insufficient mating material (does not cover all cases)
    bool white_can_mate = game_phase_score > 1 || board.pieces[Constants::WHITE_PAWN];
    bool black_can_mate = game_phase_score > 1 || board.pieces[Constants::BLACK_PAWN];
    if (!white_can_mate) evaluation = std::min(0, evaluation);
    if (!black_can_mate) evaluation = std::max(0, evaluation);

    return board.whiteToMove ? evaluation : -evaluation;
}

//measurement for game phase (middle game: 24, late end game: 0)
int getGamePhaseScore(GameBoard const &board) {
    int score = 0;
    score += 4*__builtin_popcountll(board.pieces[Constants::WHITE_QUEEN] | board.pieces[Constants::BLACK_QUEEN]);
    score += 2*__builtin_popcountll(board.pieces[Constants::WHITE_ROOK] | board.pieces[Constants::BLACK_ROOK]);
    score += __builtin_popcountll(board.pieces[Constants::WHITE_KNIGHT] | board.pieces[Constants::BLACK_KNIGHT]);
    score += __builtin_popcountll(board.pieces[Constants::WHITE_BISHOP] | board.pieces[Constants::BLACK_BISHOP]);
    return score;
}