//
// Created by salom on 23.08.2025.
//
#include <algorithm>

#include "Movepicking.h"

#include <Output.h>

#include "Memory.h"

#include "Constants.h" // lib constants
#include "EvaluationConstants.h"
using Constants::move_decoding_bitmasks;
using Constants::MoveDecoding;
#include "MoveGenerationConstants.h"

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
            phase = Good_Captures;
            [[fallthrough]];
        case Good_Captures:
            moves = getPseudoLegalMoves(board,board.whiteToMove,CAPTURES);
            mvv_lva_MoveOrdering(moves);
            return moves;
        case Good_Quiets:
            moves = getPseudoLegalMoves(board,board.whiteToMove,QUIETS);
            staticMoveOrdering(moves, board);
            return moves;
        default:
            return {};
    }
}


void staticMoveOrdering(std::vector<uint32_t> & pseudoLegalMoves, GameBoard const & board) {

    if (false) {
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

int static_exchange_evaluation(int square, bool attacker_is_white, GameBoard & board, Constants::Piece piece_on_square) {

    uint32_t attack_move = getCheapestAttackMove(board,attacker_is_white,square,piece_on_square);
    if (attack_move == 0) return 0;

    //Data for unmaking the move
    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    int captured_piece_value = abs(STATIC_MG_PIECE_VALUES[(move_decoding_bitmasks[MoveDecoding::CAPTURE] & attack_move) >> 10]);
    Constants::Piece piece_attacking = static_cast<Constants::Piece>(move_decoding_bitmasks[MoveDecoding::PIECE] & attack_move);
    if (attack_move & move_decoding_bitmasks[MoveDecoding::PROMOTION]) {
        piece_attacking = static_cast<Constants::Piece>((move_decoding_bitmasks[MoveDecoding::PROMOTION] & attack_move) >> 20);
    }

    board.applyPseudoLegalMove(attack_move);
    int value = std::max(0,captured_piece_value - static_exchange_evaluation(square, !attacker_is_white, board,piece_attacking));
    board.unmakeMove(attack_move, enPassant,castle_rights,plies,hash_before);
    return value;
}

int static_exchange_evaluation(uint32_t move, GameBoard & board) {
    Move mv = decodeMove(move);

    //Data for unmaking the move
    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    board.applyPseudoLegalMove(move);

    Constants::Piece piece_on_square = mv.pawn_promote_to ? mv.pawn_promote_to : mv.piece;

    int value = abs(STATIC_MG_PIECE_VALUES[mv.captured_piece]) - static_exchange_evaluation(mv.to,board.whiteToMove,board,piece_on_square);
    board.unmakeMove(move, enPassant,castle_rights,plies,hash_before);
    return value;
}

uint32_t getCheapestAttackMove(GameBoard const &board, bool attacker_is_white, int square, Constants::Piece piece_on_square) {

    uint64_t enemy_pieces = board.whiteToMove ? board.black_pieces : board.white_pieces;
    uint64_t all_pieces = board.white_pieces | board.black_pieces;

    if (!(enemy_pieces & (1ULL << square))) return 0; // no enemy piece on this square

    //Is square attacked by pawns
    uint64_t pawn_attacker = pawnCaptureBitMask[attacker_is_white][square] & (board.pieces[attacker_is_white?Constants::Piece::WHITE_PAWN:Constants::Piece::BLACK_PAWN]);
    if (pawn_attacker) {
        int from = __builtin_ctzll(pawn_attacker);
        return (attacker_is_white ? Constants::WHITE_PAWN : Constants::BLACK_PAWN) | from << 4 | piece_on_square << 10 | square << 14 | ((square/8 == 0 || square/8 == 7)? (Constants::WHITE_QUEEN+(!attacker_is_white))<<20 : 0);
    }
    uint64_t knight_attacker = knightAttackBitMasks[square] & board.pieces[attacker_is_white?Constants::Piece::WHITE_KNIGHT:Constants::Piece::BLACK_KNIGHT];
    if (knight_attacker) {
        int from = __builtin_ctzll(knight_attacker);
        return (attacker_is_white ? Constants::WHITE_KNIGHT : Constants::BLACK_KNIGHT) | from << 4 | piece_on_square << 10 | square << 14;
    }

    uint64_t bishop_attacker = getBishopAttackBits(square,all_pieces) & (board.pieces[attacker_is_white?Constants::Piece::WHITE_BISHOP:Constants::Piece::BLACK_BISHOP]);
    if (bishop_attacker) {
        int from = __builtin_ctzll(bishop_attacker);
        return (attacker_is_white ? Constants::WHITE_BISHOP : Constants::BLACK_BISHOP) | from << 4 | piece_on_square << 10 | square << 14;
    }

    uint64_t rook_attacker = getRookAttackBits(square,all_pieces) & (board.pieces[attacker_is_white?Constants::Piece::WHITE_ROOK:Constants::Piece::BLACK_ROOK]);
    if (rook_attacker) {
        int from = __builtin_ctzll(rook_attacker);
        return (attacker_is_white ? Constants::WHITE_ROOK : Constants::BLACK_ROOK) | from << 4 | piece_on_square << 10 | square << 14;
    }

    uint64_t queen_attacker = (getBishopAttackBits(square,all_pieces) | getRookAttackBits(square,all_pieces)) & (board.pieces[attacker_is_white?Constants::Piece::WHITE_QUEEN:Constants::Piece::BLACK_QUEEN]);
    if (queen_attacker) {
        int from = __builtin_ctzll(queen_attacker);
        return (attacker_is_white ? Constants::WHITE_QUEEN : Constants::BLACK_QUEEN) | from << 4 | piece_on_square << 10 | square << 14;
    }

    return 0; // no attacker left
}
