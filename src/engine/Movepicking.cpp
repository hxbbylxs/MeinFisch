//
// Created by salom on 23.08.2025.
//
#include <algorithm>

#include "Movepicking.h"
#include "Memory.h"

#include "Constants.h" // lib constants
#include "EvaluationConstants.h"
using Constants::move_decoding_bitmasks;
using Constants::MoveDecoding;

#include "Move.h" // lib game_logic
#include "MoveGeneration.h"


// generates next moves only when necessary (lazy move generation)
// example: no need to generate all quiet moves when a capture move already causes a cutoff
std::vector<uint32_t> pickNextMoves(Data const &savedData, uint32_t killerCandidate, GameBoard const &board, MoveGenPhase & phase) {
    std::vector<uint32_t> moves;
    switch (phase) {
        case TTMove:
                if (savedData.evaluationFlag != EMPTY) {
                    return {savedData.bestMove};
                }
            phase = Killer;
            [[fallthrough]];
        case Killer:
            if (killerCandidate != savedData.bestMove && isPseudoLegalMove(killerCandidate,board)) {
                return {killerCandidate};
            }
            phase = Captures;
            [[fallthrough]];
        case Captures:
            moves = getPseudoLegalMoves(board,board.whiteToMove,CAPTURES);
            mvv_lva_MoveOrdering(moves);
            return moves;
        case Quiets:
            moves = getPseudoLegalMoves(board,board.whiteToMove,QUIETS);
            staticMoveOrdering(moves, board);
            return moves;
    }
}


void staticMoveOrdering(std::vector<uint32_t> & pseudoLegalMoves, GameBoard const & board) {

    if (__builtin_popcountll(board.allPieces) < 14) {
        std::sort(pseudoLegalMoves.begin(), pseudoLegalMoves.end(),[](uint32_t a, uint32_t b) {
     return getMoveScoreEndGame(a) > getMoveScoreEndGame(b);
 });
    } else {

        std::sort(pseudoLegalMoves.begin(), pseudoLegalMoves.end(),[](uint32_t a, uint32_t b) {
    return getMoveScoreMiddleGame(a) > getMoveScoreMiddleGame(b);
});
    }
}

// 1. Queen Promo 2. history score 3. other Promo
// will be used only for quiet moves
int getMoveScoreMiddleGame(uint32_t move) {
    Move mv = decodeMove(move);
    int score = history_move_scores[mv.from][mv.to];
    if (mv.pawn_promote_to) {
        if (mv.pawn_promote_to == Constants::WHITE_QUEEN || mv.pawn_promote_to == Constants::BLACK_QUEEN) {
            score += 1'000'000;
        } else {
            score = 0;
        }
    }
    return score;
}



// 1. Queen Promo 2. MVP 4. Other Promo
// will be used only for quiet moves
int getMoveScoreEndGame(uint32_t move) {
    Move mv = decodeMove(move);
    int score = abs(STATIC_MG_PIECE_VALUES[(mv.piece)]);
    if (mv.piece == Constants::WHITE_KING || mv.piece == Constants::BLACK_KING) score += 400;
    if (mv.pawn_promote_to) {
        if (mv.pawn_promote_to == Constants::WHITE_QUEEN || mv.pawn_promote_to == Constants::BLACK_QUEEN) {
            score += 1'000'000;
        } else {
            score = 0;
        }
    }
    return score;
}


void mvv_lva_MoveOrdering(std::vector<uint32_t> & pseudoLegalMoves) {
    std::sort(pseudoLegalMoves.begin(), pseudoLegalMoves.end(),[](uint32_t a, uint32_t b) {
        int score_a = 5*abs(STATIC_MG_PIECE_VALUES[(move_decoding_bitmasks[MoveDecoding::CAPTURE] & a) >> 10]) - abs(STATIC_MG_PIECE_VALUES[(move_decoding_bitmasks[MoveDecoding::PIECE] & a)]);
        int score_b = 5*abs(STATIC_MG_PIECE_VALUES[(move_decoding_bitmasks[MoveDecoding::CAPTURE] & b) >> 10]) - abs(STATIC_MG_PIECE_VALUES[(move_decoding_bitmasks[MoveDecoding::PIECE] & b)]);
        return score_a > score_b;
    });
}