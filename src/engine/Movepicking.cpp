//
// Created by salom on 23.08.2025.
//
#include <algorithm>

#include "Movepicking.h"

#include <iostream>
#include <cassert>

#include "Memory.h"
#include "EvaluationFunction.h"

#include "Constants.h" // lib constants
#include "EvaluationConstants.h"
#include "PieceSquareTables.h"
using Constants::move_decoding_bitmasks;
using Constants::MoveDecoding;
#include "MoveGenerationConstants.h"

#include "Move.h" // lib game_logic
#include "MoveGeneration.h"

#include "Utils.h" // lib utils



// generates next moves only when necessary (lazy move generation)
// example: no need to generate all quiet moves when a capture move already causes a cutoff
std::vector<Move> pickNextMoves(Data const &savedData,Move counter_candidate ,Move killer_candidate, GameBoard const &board, MoveGenPhase & phase) {
    std::vector<Move> moves;
    switch (phase) {
        case TTMove:
                if (savedData.evaluationFlag != EMPTY) {
                    assert(isPseudoLegalMove(savedData.bestMove,board));
                    return {savedData.bestMove};
                }
            phase = Good_Captures;
            [[fallthrough]];
        case Good_Captures:
            moves = getPseudoLegalMoves(board,board.whiteToMove,CAPTURES);
            mvv_lva_MoveOrdering(moves);
            return moves;
        case Killer:
            if (isPseudoLegalMove(killer_candidate,board)) {
                return {killer_candidate};
            }
        phase = Counter;
        [[fallthrough]];
        case Counter:
            if (isPseudoLegalMove(counter_candidate,board)) {
                return {counter_candidate};
            }
        phase = Good_Quiets;
        [[fallthrough]];
        case Good_Quiets:
            moves = getPseudoLegalMoves(board,board.whiteToMove,QUIETS);
            staticMoveOrdering(moves, board);
            return moves;
        case QCaptures:
            moves = getPseudoLegalMoves(board,board.whiteToMove,CAPTURES);
            mvv_lva_MoveOrdering(moves);
            return moves;
        case QPawns:
            moves = getPseudoLegalAdvancedPawnPushes(board,board.whiteToMove);
            return moves;
        default:
            return {};
    }
}


void staticMoveOrdering(std::vector<Move> & pseudoLegalMoves, GameBoard const & board) {

    std::sort(pseudoLegalMoves.begin(), pseudoLegalMoves.end(),[](Move a, Move b) {
        return getMoveScore(a) > getMoveScore(b);
    });
}

// 1. Queen Promo 2. history score 3. other Promo
// will be used only for quiet moves
int getMoveScore(Move move) {

    // Cutoff scores
    int score = history_move_scores[move.from()][move.to()];
    // Promo
    if (move.promotion()) {
        if (move.promotion() == Constants::WHITE_QUEEN || move.promotion() == Constants::BLACK_QUEEN) {
            score += 1'000'000;
        } else {
            score = 0;
        }
    }

    return score;
}



void mvv_lva_MoveOrdering(std::vector<Move> & pseudoLegalMoves) {
    std::sort(pseudoLegalMoves.begin(), pseudoLegalMoves.end(),[](Move a, Move b) {
        int score_a = 5*abs(STATIC_MG_PIECE_VALUES[a.capture()]) - abs(STATIC_MG_PIECE_VALUES[a.piece()]);
        int score_b = 5*abs(STATIC_MG_PIECE_VALUES[b.capture()]) - abs(STATIC_MG_PIECE_VALUES[b.piece()]);
        return score_a > score_b;
    });
}

int static_exchange_evaluation(unsigned square, bool attacker_is_white, GameBoard & board, Constants::Piece piece_on_square) {

    Move attack_move = getCheapestAttackMove(board,attacker_is_white,square,piece_on_square);
    if (attack_move.value == 0) return 0;

    //Data for unmaking the move
    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    int captured_piece_value = abs(STATIC_MG_PIECE_VALUES[attack_move.capture()]);
    auto piece_attacking = static_cast<Constants::Piece>(attack_move.piece());
    if (attack_move.promotion()) {
        piece_attacking = static_cast<Constants::Piece>(attack_move.promotion());
    }

    board.applyPseudoLegalMove(attack_move);
    int value = std::max(0,captured_piece_value - static_exchange_evaluation(square, !attacker_is_white, board,piece_attacking));
    board.unmakeMove(attack_move, enPassant,castle_rights,plies,hash_before);
    return value;
}

int static_exchange_evaluation(Move move, GameBoard & board) {

    //Data for unmaking the move
    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    board.applyPseudoLegalMove(move);

    auto piece_on_square = static_cast<Constants::Piece>(move.promotion() ? move.promotion() : move.piece());

    int value = abs(STATIC_MG_PIECE_VALUES[move.capture()]) - static_exchange_evaluation(move.to(),board.whiteToMove,board,piece_on_square);
    board.unmakeMove(move, enPassant,castle_rights,plies,hash_before);
    return value;
}

Move getCheapestAttackMove(GameBoard const &board, bool attacker_is_white, unsigned square, Constants::Piece piece_on_square) {

    uint64_t enemy_pieces = board.whiteToMove ? board.black_pieces : board.white_pieces;
    uint64_t all_pieces = board.white_pieces | board.black_pieces;

    if (!(enemy_pieces & (1ULL << square))) return 0; // no enemy piece on this square

    //Is square attacked by pawns
    uint64_t pawn_attacker = pawnCaptureBitMask[attacker_is_white][square] & (board.pieces[attacker_is_white?Constants::Piece::WHITE_PAWN:Constants::Piece::BLACK_PAWN]);
    if (pawn_attacker) {
        int from = counttzll(pawn_attacker);
        return (attacker_is_white ? Constants::WHITE_PAWN : Constants::BLACK_PAWN) | from << 4 | piece_on_square << 10 | square << 14 | ((square/8 == 0 || square/8 == 7)? (Constants::WHITE_QUEEN+(!attacker_is_white))<<20 : 0);
    }
    uint64_t knight_attacker = knightAttackBitMasks[square] & board.pieces[attacker_is_white?Constants::Piece::WHITE_KNIGHT:Constants::Piece::BLACK_KNIGHT];
    if (knight_attacker) {
        int from = counttzll(knight_attacker);
        return (attacker_is_white ? Constants::WHITE_KNIGHT : Constants::BLACK_KNIGHT) | from << 4 | piece_on_square << 10 | square << 14;
    }

    uint64_t bishop_attacker = getBishopAttackBits(square,all_pieces) & (board.pieces[attacker_is_white?Constants::Piece::WHITE_BISHOP:Constants::Piece::BLACK_BISHOP]);
    if (bishop_attacker) {
        int from = counttzll(bishop_attacker);
        return (attacker_is_white ? Constants::WHITE_BISHOP : Constants::BLACK_BISHOP) | from << 4 | piece_on_square << 10 | square << 14;
    }

    uint64_t rook_attacker = getRookAttackBits(square,all_pieces) & (board.pieces[attacker_is_white?Constants::Piece::WHITE_ROOK:Constants::Piece::BLACK_ROOK]);
    if (rook_attacker) {
        int from = counttzll(rook_attacker);
        return (attacker_is_white ? Constants::WHITE_ROOK : Constants::BLACK_ROOK) | from << 4 | piece_on_square << 10 | square << 14;
    }

    uint64_t queen_attacker = (getBishopAttackBits(square,all_pieces) | getRookAttackBits(square,all_pieces)) & (board.pieces[attacker_is_white?Constants::Piece::WHITE_QUEEN:Constants::Piece::BLACK_QUEEN]);
    if (queen_attacker) {
        int from = counttzll(queen_attacker);
        return (attacker_is_white ? Constants::WHITE_QUEEN : Constants::BLACK_QUEEN) | from << 4 | piece_on_square << 10 | square << 14;
    }

    return 0; // no attacker left
}
