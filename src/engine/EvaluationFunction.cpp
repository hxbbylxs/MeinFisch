//
// Created by salom on 16.07.2025.
//
#include <cmath>

#include "EvaluationFunction.h"

#include "EvaluationConstants.h" // lib constants
#include "MoveGenerationConstants.h"

#include "GameBoard.h" //lib game_logic


// returns a value in centipawns (positive ~ good for the color to move)
int evaluate(GameBoard const & board, int alpha, int beta) {
    //lazy cutoff
    /*int fastEval = evaluateMaterialOnly(board);
    if (fastEval >= beta+LAZY_EVAL_SAFETY_MARGIN) return beta;
    if (fastEval <= alpha-LAZY_EVAL_SAFETY_MARGIN) return alpha;*/

    // mg = middle game, eg = end game
    int evaluation = 0;
    int game_phase_score = getGamePhaseScore(board);

    EvalInfo eval_info;

    eval_info.white_pieces = board.pieces[Constants::WHITE_KING] | board.pieces[Constants::WHITE_QUEEN]
                                        | board.pieces[Constants::WHITE_ROOK] | board.pieces[Constants::WHITE_BISHOP]
                                        | board.pieces[Constants::WHITE_KNIGHT] | board.pieces[Constants::WHITE_PAWN];
    eval_info.black_pieces = board.allPieces & ~eval_info.white_pieces;



    // have to be called in this particular order because attacked squares by less valuable pieces are passed by reference
    evaluateKing(board, eval_info);
    evaluatePawns(board, eval_info);
    evaluateKnights(board, eval_info);
    evaluateBishops(board, eval_info);
    evaluateRooks(board, eval_info);
    evaluateQueen(board, eval_info);

    int num_hanging_pieces = (__builtin_popcountll(eval_info.white_pieces) - __builtin_popcountll(eval_info.whites_defended_pieces));
    eval_info.mg_evaluation -= num_hanging_pieces*num_hanging_pieces;
    num_hanging_pieces = (__builtin_popcountll(eval_info.black_pieces) - __builtin_popcountll(eval_info.blacks_defended_pieces));
    eval_info.mg_evaluation -= num_hanging_pieces*num_hanging_pieces;

    // linear interpolation between middle game and end game
    evaluation = (game_phase_score*eval_info.mg_evaluation + (24-game_phase_score)*eval_info.eg_evaluation)/24;


    evaluation += board.whiteToMove ? TEMPO_BONUS : -TEMPO_BONUS;

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


void evaluateKing(GameBoard const &board, EvalInfo & eval_info) {
    int black_king_position = __builtin_ctzll(board.pieces[Constants::Piece::BLACK_KING]);
    int white_king_position = __builtin_ctzll(board.pieces[Constants::Piece::WHITE_KING]);

    // the small king zone is the 3x3 square around the king
    eval_info.white_king_zone_small = board.pieces[Constants::Piece::WHITE_KING] | kingAttackBitMasks[__builtin_ctzll(board.pieces[Constants::Piece::WHITE_KING])];
    eval_info.black_king_zone_small = board.pieces[Constants::Piece::BLACK_KING] | kingAttackBitMasks[__builtin_ctzll(board.pieces[Constants::Piece::BLACK_KING])];

    // large king zone is a 5x5 area
    eval_info.white_king_zone_large = KING_LARGE_ZONE_BITMASK[white_king_position];
    eval_info.black_king_zone_large = KING_LARGE_ZONE_BITMASK[black_king_position];

    uint64_t pawns_directly_ahead_black_king = board.pieces[Constants::Piece::BLACK_PAWN] & KING_PAWN_SHIELD_BITMASK[black_king_position];
    uint64_t pawns_directly_ahead_white_king = board.pieces[Constants::Piece::WHITE_PAWN] & KING_PAWN_SHIELD_BITMASK[white_king_position];

    int num_protectors_black = __builtin_popcountll(pawns_directly_ahead_black_king);
    int num_protectors_white = __builtin_popcountll(pawns_directly_ahead_white_king);

    // bonus for every pawn on the three squares ahead of the king
    eval_info.mg_evaluation += PAWN_SHIELD*num_protectors_white - PAWN_SHIELD*num_protectors_black;

    // penalty if the pawn in front of the king is missing
    // & 7 equals % 8 and gives the correct line
    if (!(__builtin_popcountll(LINE_BITMASKS[white_king_position & 7] & board.pieces[Constants::Piece::WHITE_PAWN]))) eval_info.mg_evaluation += PAWN_AHEAD_KING_MISSING;
    if (!(__builtin_popcountll(LINE_BITMASKS[black_king_position & 7] & board.pieces[Constants::Piece::BLACK_PAWN]))) eval_info.mg_evaluation -= PAWN_AHEAD_KING_MISSING;

    // evaluation only based on the position of the king on the board
    eval_info.mg_evaluation += PST_MG_WHITE_KING[white_king_position];
    eval_info.mg_evaluation -= PST_MG_BLACK_KING[black_king_position];
    eval_info.eg_evaluation += PST_EG_WHITE_KING[white_king_position];
    eval_info.eg_evaluation -= PST_EG_BLACK_KING[black_king_position];

    // punish open lines to the king
    uint64_t if_king_moves_like_queen = getQueenAttackBits(white_king_position,board.allPieces);
    int num_reachable_squares = __builtin_popcountll(if_king_moves_like_queen);
    eval_info.mg_evaluation -= num_reachable_squares*num_reachable_squares/5;

    uint64_t if_black_king_moves_like_queen = getQueenAttackBits(black_king_position,board.allPieces);
    num_reachable_squares = __builtin_popcountll(if_black_king_moves_like_queen);
    eval_info.mg_evaluation += num_reachable_squares*num_reachable_squares/5;

    // Squares attacked around the king are evaluated as pressure in the evaluatePiece functions
    // due to performance, although they would belong here.

    //black pieces in whites small zone
    int num_pieces_in_zone = __builtin_popcountll(eval_info.white_king_zone_small & board.pieces[Constants::BLACK_QUEEN]);
    eval_info.mg_evaluation -= num_pieces_in_zone*QUEEN_IN_SMALL_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.white_king_zone_small & board.pieces[Constants::BLACK_ROOK]);
    eval_info.mg_evaluation -= num_pieces_in_zone*ROOK_IN_SMALL_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.white_king_zone_small & board.pieces[Constants::BLACK_BISHOP]);
    eval_info.mg_evaluation -= num_pieces_in_zone*BISHOP_IN_SMALL_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.white_king_zone_small & board.pieces[Constants::BLACK_KNIGHT]);
    eval_info.mg_evaluation -= num_pieces_in_zone*KNIGHT_IN_SMALL_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.white_king_zone_small & board.pieces[Constants::BLACK_PAWN]);
    eval_info.mg_evaluation -= num_pieces_in_zone*PAWN_IN_SMALL_KING_ZONE;

    //black pieces in whites large zone
    num_pieces_in_zone = __builtin_popcountll(eval_info.white_king_zone_large & board.pieces[Constants::BLACK_QUEEN]);
    eval_info.mg_evaluation -= num_pieces_in_zone*QUEEN_IN_LARGE_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.white_king_zone_large & board.pieces[Constants::BLACK_ROOK]);
    eval_info.mg_evaluation -= num_pieces_in_zone*ROOK_IN_LARGE_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.white_king_zone_large & board.pieces[Constants::BLACK_BISHOP]);
    eval_info.mg_evaluation -= num_pieces_in_zone*BISHOP_IN_LARGE_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.white_king_zone_large & board.pieces[Constants::BLACK_KNIGHT]);
    eval_info.mg_evaluation -= num_pieces_in_zone*KNIGHT_IN_LARGE_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.white_king_zone_large & board.pieces[Constants::BLACK_PAWN]);
    eval_info.mg_evaluation -= num_pieces_in_zone*PAWN_IN_LARGE_KING_ZONE;

        //white pieces in black small zone
    num_pieces_in_zone = __builtin_popcountll(eval_info.black_king_zone_small & board.pieces[Constants::WHITE_QUEEN]);
    eval_info.mg_evaluation += num_pieces_in_zone*QUEEN_IN_SMALL_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.black_king_zone_small & board.pieces[Constants::WHITE_ROOK]);
    eval_info.mg_evaluation += num_pieces_in_zone*ROOK_IN_SMALL_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.black_king_zone_small & board.pieces[Constants::WHITE_BISHOP]);
    eval_info.mg_evaluation += num_pieces_in_zone*BISHOP_IN_SMALL_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.black_king_zone_small & board.pieces[Constants::WHITE_KNIGHT]);
    eval_info.mg_evaluation += num_pieces_in_zone*KNIGHT_IN_SMALL_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.black_king_zone_small & board.pieces[Constants::WHITE_PAWN]);
    eval_info.mg_evaluation += num_pieces_in_zone*PAWN_IN_SMALL_KING_ZONE;

    //white pieces in blacks large zone
    num_pieces_in_zone = __builtin_popcountll(eval_info.black_king_zone_large & board.pieces[Constants::WHITE_QUEEN]);
    eval_info.mg_evaluation += num_pieces_in_zone*QUEEN_IN_LARGE_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.black_king_zone_large & board.pieces[Constants::WHITE_ROOK]);
    eval_info.mg_evaluation += num_pieces_in_zone*ROOK_IN_LARGE_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.black_king_zone_large & board.pieces[Constants::WHITE_BISHOP]);
    eval_info.mg_evaluation += num_pieces_in_zone*BISHOP_IN_LARGE_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.black_king_zone_large & board.pieces[Constants::WHITE_KNIGHT]);
    eval_info.mg_evaluation += num_pieces_in_zone*KNIGHT_IN_LARGE_KING_ZONE;
    num_pieces_in_zone = __builtin_popcountll(eval_info.black_king_zone_large & board.pieces[Constants::WHITE_PAWN]);
    eval_info.mg_evaluation += num_pieces_in_zone*PAWN_IN_LARGE_KING_ZONE;
}

// modifies the values passed by reference
void evaluatePawns(GameBoard const &board, EvalInfo & eval_info) {
    int any_game_phase_evaluation = 0; // will be added to mg evaluation as well as eg evaluation
    uint64_t white_pawn_attacked_squares = 0;
    uint64_t black_pawn_attacked_squares = 0;

    forEachPiece(Constants::WHITE_PAWN,board,[&](int position, GameBoard const & game_board) {
        // PST
        eval_info.mg_evaluation += PST_MG_WHITE_PAWN[position];
        eval_info.eg_evaluation += PST_EG_WHITE_PAWN[position];

        uint64_t attacked_squares = pawnCaptureBitMask[0][position];
        white_pawn_attacked_squares |= attacked_squares; // will be needed in knight/bishop evaluation
        eval_info.whites_defended_pieces |= attacked_squares & eval_info.white_pieces;

        // pressure on king
        if (attacked_squares & eval_info.black_king_zone_large) {
            any_game_phase_evaluation += PAWN_ATTACKS_LARGE_KING_ZONE*__builtin_popcountll(eval_info.black_king_zone_large & attacked_squares);
            if (attacked_squares & eval_info.black_king_zone_small) {
                any_game_phase_evaluation += PAWN_ATTACKS_SMALL_KING_ZONE*__builtin_popcountll(eval_info.black_king_zone_small & attacked_squares);
            }
        }

        // passed, doubled, isolated pawn
        if (!(PASSED_PAWN_BITMASK[0][position] & board.pieces[Constants::Piece::BLACK_PAWN])) {
            eval_info.mg_evaluation += PAWN_PASSED_MG[position/8];
            eval_info.eg_evaluation += PAWN_PASSED_EG[position/8];
        }
        if (!(ISOLATED_PAWN_BITMASK[position&7] & board.pieces[Constants::Piece::WHITE_PAWN])) {
            any_game_phase_evaluation += PAWN_ISOLATED;
        }
        if (__builtin_popcountll(board.pieces[Constants::WHITE_PAWN] & LINE_BITMASKS[position&7]) > 1) any_game_phase_evaluation += PAWN_DOUBLED;
    });
    forEachPiece(Constants::BLACK_PAWN,board,[&](int position, GameBoard const & game_board) {
        //PST
        eval_info.mg_evaluation -= PST_MG_BLACK_PAWN[position];
        eval_info.eg_evaluation -= PST_EG_BLACK_PAWN[position];

        uint64_t attacked_squares = pawnCaptureBitMask[1][position];
        black_pawn_attacked_squares |= attacked_squares;
        eval_info.blacks_defended_pieces |= attacked_squares & eval_info.black_pieces;

        //King Pressure
        if (attacked_squares & eval_info.white_king_zone_large) {
            any_game_phase_evaluation -= PAWN_ATTACKS_LARGE_KING_ZONE*__builtin_popcountll(eval_info.white_king_zone_large & attacked_squares);
            if (attacked_squares & eval_info.white_king_zone_small) {
                any_game_phase_evaluation -= PAWN_ATTACKS_SMALL_KING_ZONE*__builtin_popcountll(eval_info.white_king_zone_small & attacked_squares);
            }
        }
        // passed, doubled, isolated
        if (!(PASSED_PAWN_BITMASK[1][position] & board.pieces[Constants::Piece::WHITE_PAWN])) {
            eval_info.mg_evaluation -= PAWN_PASSED_MG[7-(position/8)];
            eval_info.eg_evaluation -= PAWN_PASSED_EG[7-(position/8)];
        }
        if (!(ISOLATED_PAWN_BITMASK[position&7] & board.pieces[Constants::Piece::BLACK_PAWN])) {
            any_game_phase_evaluation -= PAWN_ISOLATED;
        }
        if (__builtin_popcountll(board.pieces[Constants::BLACK_PAWN] & LINE_BITMASKS[position&7]) > 1) any_game_phase_evaluation -= PAWN_DOUBLED;
    });
    eval_info.mg_evaluation += any_game_phase_evaluation;
    eval_info.eg_evaluation += any_game_phase_evaluation;

    eval_info.squares_attacked_by_less_valuable_white_pieces |= white_pawn_attacked_squares;
    eval_info.squares_attacked_by_less_valuable_black_pieces |= black_pawn_attacked_squares;
}


void evaluateKnights(GameBoard const &board, EvalInfo & eval_info) {
    int any_game_phase_evaluation = 0;
    uint64_t white_knights_attacked_squares = 0;
    uint64_t black_knights_attacked_squares = 0;

    forEachPiece(Constants::WHITE_KNIGHT,board,[&](int position, GameBoard const & game_board) {

        uint64_t attacked_squares = knightAttackBitMasks[position];
        white_knights_attacked_squares |= attacked_squares;
        eval_info.whites_defended_pieces |= attacked_squares & eval_info.white_pieces;

        // mobility
        uint64_t possible_moves = ~board.allPieces & attacked_squares & ~eval_info.squares_attacked_by_less_valuable_black_pieces;
        any_game_phase_evaluation += KNIGHT_BONUS_PER_SQUARE*__builtin_popcountll(possible_moves);

        // king pressure
        if (attacked_squares & eval_info.black_king_zone_large) {
            any_game_phase_evaluation += KNIGHT_ATTACKS_LARGE_KING_ZONE*__builtin_popcountll(eval_info.black_king_zone_large & attacked_squares);
            if (attacked_squares & eval_info.black_king_zone_small) {
                any_game_phase_evaluation += KNIGHT_ATTACKS_SMALL_KING_ZONE*__builtin_popcountll(eval_info.black_king_zone_small & attacked_squares);
            }
        }
        // PST
        eval_info.mg_evaluation += PST_MG_KNIGHT[position];
        eval_info.eg_evaluation += PST_EG_KNIGHT[position];
    });
    forEachPiece(Constants::BLACK_KNIGHT,board,[&](int position, GameBoard const & game_board) {

        uint64_t attacked_squares = knightAttackBitMasks[position];
        black_knights_attacked_squares |= attacked_squares;
        eval_info.blacks_defended_pieces |= attacked_squares & eval_info.black_pieces;

        //mobility
        uint64_t possible_moves = ~board.allPieces & attacked_squares & ~eval_info.squares_attacked_by_less_valuable_white_pieces;
        any_game_phase_evaluation -= KNIGHT_BONUS_PER_SQUARE*__builtin_popcountll(possible_moves);

        // king pressure
        if (attacked_squares & eval_info.white_king_zone_large) {
            any_game_phase_evaluation -= KNIGHT_ATTACKS_LARGE_KING_ZONE*__builtin_popcountll(eval_info.white_king_zone_large & attacked_squares);
            if (attacked_squares & eval_info.white_king_zone_small) {
                any_game_phase_evaluation -= KNIGHT_ATTACKS_SMALL_KING_ZONE*__builtin_popcountll(eval_info.white_king_zone_small & attacked_squares);
            }
        }
        // PST
        eval_info.mg_evaluation -= PST_MG_KNIGHT[position];
        eval_info.eg_evaluation -= PST_EG_KNIGHT[position];
});
    eval_info.mg_evaluation += any_game_phase_evaluation;
    eval_info.eg_evaluation += any_game_phase_evaluation;

    eval_info.squares_attacked_by_less_valuable_white_pieces |= white_knights_attacked_squares;
    eval_info.squares_attacked_by_less_valuable_black_pieces |= black_knights_attacked_squares;
}


void evaluateBishops(GameBoard const &board, EvalInfo & eval_info) {
    int any_game_phase_evaluation = 0;
    uint64_t white_bishops_attacked_squares = 0;
    uint64_t black_bishops_attacked_squares = 0;

    forEachPiece(Constants::WHITE_BISHOP,board,[&](int position, GameBoard const & game_board) {

        uint64_t attacked_squares = getBishopAttackBits(position,board.allPieces);
        white_bishops_attacked_squares |= attacked_squares;
        eval_info.whites_defended_pieces |= attacked_squares & eval_info.white_pieces;

        // mobility
        uint64_t possible_moves = ~board.allPieces & attacked_squares & ~eval_info.squares_attacked_by_less_valuable_black_pieces;
        any_game_phase_evaluation += BISHOP_BONUS_PER_SQUARE*__builtin_popcountll(possible_moves);
        any_game_phase_evaluation += BISHOP_BONUS_PER_CENTER_SQUARE*__builtin_popcountll(PST_CENTER_SQUARE & possible_moves);

        //TODO xray on rook/queen/king

        // king pressure
        if (attacked_squares & eval_info.black_king_zone_large) {
            any_game_phase_evaluation += BISHOP_ATTACKS_LARGE_KING_ZONE*__builtin_popcountll(eval_info.black_king_zone_large & attacked_squares);
            if (attacked_squares & eval_info.black_king_zone_small) {
                any_game_phase_evaluation += BISHOP_ATTACKS_SMALL_KING_ZONE*__builtin_popcountll(eval_info.black_king_zone_small & attacked_squares);
            }
        }

        // PST
        eval_info.mg_evaluation += PST_MG_BISHOP[position];
        eval_info.eg_evaluation += PST_EG_BISHOP[position];
    });
    forEachPiece(Constants::BLACK_BISHOP,board,[&](int position, GameBoard const & game_board) {

        uint64_t attacked_squares = getBishopAttackBits(position,board.allPieces);
        black_bishops_attacked_squares |= attacked_squares;
        eval_info.blacks_defended_pieces |= attacked_squares & eval_info.black_pieces;

        // mobility
        uint64_t possible_moves = ~board.allPieces & attacked_squares & ~eval_info.squares_attacked_by_less_valuable_white_pieces;
        any_game_phase_evaluation -= BISHOP_BONUS_PER_SQUARE*__builtin_popcountll(possible_moves);
        any_game_phase_evaluation -= BISHOP_BONUS_PER_CENTER_SQUARE*__builtin_popcountll(PST_CENTER_SQUARE & possible_moves);

        //TODO xray

        // king pressure
        if (attacked_squares & eval_info.white_king_zone_large) {
            any_game_phase_evaluation -= BISHOP_ATTACKS_LARGE_KING_ZONE*__builtin_popcountll(eval_info.white_king_zone_large & attacked_squares);
            if (attacked_squares & eval_info.white_king_zone_small) {
                any_game_phase_evaluation -= BISHOP_ATTACKS_SMALL_KING_ZONE*__builtin_popcountll(eval_info.white_king_zone_small & attacked_squares);
            }
        }

        // PST
        eval_info.mg_evaluation -= PST_MG_BISHOP[position];
        eval_info.eg_evaluation -= PST_EG_BISHOP[position];
    });

    eval_info.mg_evaluation += any_game_phase_evaluation;
    eval_info.eg_evaluation += any_game_phase_evaluation;

    if (__builtin_popcountll(board.pieces[Constants::Piece::WHITE_BISHOP]) >= 2) eval_info.eg_evaluation += BISHOP_PAIR;
    if (__builtin_popcountll(board.pieces[Constants::Piece::BLACK_BISHOP]) >= 2) eval_info.eg_evaluation -= BISHOP_PAIR;

    eval_info.squares_attacked_by_less_valuable_white_pieces |= white_bishops_attacked_squares;
    eval_info.squares_attacked_by_less_valuable_black_pieces |= black_bishops_attacked_squares;
}


void evaluateRooks(GameBoard const &board, EvalInfo & eval_info) {

    uint64_t pawns = board.pieces[Constants::Piece::WHITE_PAWN] | board.pieces[Constants::Piece::BLACK_PAWN];
    int any_game_phase_evaluation = 0;
    uint64_t white_rooks_attacked_squares = 0;
    uint64_t black_rooks_attacked_squares = 0;

    forEachPiece(Constants::WHITE_ROOK,board,[&](int position, GameBoard const & game_board) {

        // open lines
        int num_pawns_on_line = __builtin_popcountll(LINE_BITMASKS[position & 7] & pawns);
        if (num_pawns_on_line == 0) any_game_phase_evaluation += ROOK_ON_OPEN_LINE;
        if (num_pawns_on_line == 1) any_game_phase_evaluation += ROOK_ON_HALF_OPEN_LINE;

        // controlled squares
        uint64_t attacked_squares = getRookAttackBits(position,board.allPieces);
        eval_info.whites_defended_pieces |= eval_info.white_pieces & attacked_squares;
        white_rooks_attacked_squares |= attacked_squares;

        // pressure on king
        if (attacked_squares & eval_info.black_king_zone_large) {
            any_game_phase_evaluation += ROOK_ATTACKS_LARGE_KING_ZONE*__builtin_popcountll(eval_info.black_king_zone_large & attacked_squares);
            if (attacked_squares & eval_info.black_king_zone_small) {
                any_game_phase_evaluation += ROOK_ATTACKS_SMALL_KING_ZONE*__builtin_popcountll(eval_info.black_king_zone_small & attacked_squares);
            }
        }

        // TODO xray

        // PST
        eval_info.mg_evaluation += PST_MG_WHITE_ROOK[position];
        eval_info.eg_evaluation += PST_EG_WHITE_ROOK[position];
    });

    forEachPiece(Constants::BLACK_ROOK,board,[&](int position, GameBoard const & game_board) {

        // open lines
        int num_pawns_on_line = __builtin_popcountll(LINE_BITMASKS[position & 7] & pawns);
        if (num_pawns_on_line == 0) any_game_phase_evaluation -= ROOK_ON_OPEN_LINE;
        if (num_pawns_on_line == 1) any_game_phase_evaluation -= ROOK_ON_HALF_OPEN_LINE;

        // controlled squares
        uint64_t attacked_squares = getRookAttackBits(position,board.allPieces);
        eval_info.blacks_defended_pieces |= eval_info.black_pieces & attacked_squares;
        black_rooks_attacked_squares |= attacked_squares;

        // pressure on king
        if (attacked_squares & eval_info.white_king_zone_large) {
            any_game_phase_evaluation -= ROOK_ATTACKS_LARGE_KING_ZONE*__builtin_popcountll(eval_info.white_king_zone_large & attacked_squares);
            if (attacked_squares & eval_info.white_king_zone_small) {
                any_game_phase_evaluation -= ROOK_ATTACKS_SMALL_KING_ZONE*__builtin_popcountll(eval_info.white_king_zone_small & attacked_squares);
            }
        }

        // TODO xray

        // PST
        eval_info.mg_evaluation -= PST_MG_WHITE_ROOK[position];
        eval_info.eg_evaluation -= PST_EG_WHITE_ROOK[position];
    });

    eval_info.squares_attacked_by_less_valuable_white_pieces |= white_rooks_attacked_squares;
    eval_info.squares_attacked_by_less_valuable_black_pieces |= black_rooks_attacked_squares;

    if (__builtin_popcountll(board.pieces[Constants::WHITE_ROOK] & white_rooks_attacked_squares)) {
        any_game_phase_evaluation += ROOKS_CONNECTED;
    }
    if (__builtin_popcountll(board.pieces[Constants::BLACK_ROOK] & black_rooks_attacked_squares)) {
        any_game_phase_evaluation -= ROOKS_CONNECTED;
    }

    eval_info.mg_evaluation += any_game_phase_evaluation;
    eval_info.eg_evaluation += any_game_phase_evaluation;
}



void evaluateQueen(GameBoard const &board, EvalInfo & eval_info) {
    int any_game_phase_evaluation = 0;

    forEachPiece(Constants::WHITE_QUEEN,board,[&](int position, GameBoard const & game_board) {


        uint64_t attacked_squares = getQueenAttackBits(position, board.allPieces);
        eval_info.whites_defended_pieces |= attacked_squares & eval_info.white_pieces;

        // pressure on king
        if (attacked_squares & eval_info.black_king_zone_large) {
            any_game_phase_evaluation += QUEEN_ATTACKS_LARGE_KING_ZONE*__builtin_popcountll(eval_info.black_king_zone_large & attacked_squares);
            if (attacked_squares & eval_info.black_king_zone_small) {
                any_game_phase_evaluation += QUEEN_ATTACKS_SMALL_KING_ZONE*__builtin_popcountll(eval_info.black_king_zone_small & attacked_squares);
            }
        }

        //attack on queen
        if ((1ULL << position) & eval_info.squares_attacked_by_less_valuable_black_pieces) {
            uint64_t safe_squares = attacked_squares & ~eval_info.white_pieces & ~eval_info.squares_attacked_by_less_valuable_black_pieces;
            int num_safe_squares = __builtin_popcountll(safe_squares);
            if (num_safe_squares < 5) {
                eval_info.eg_evaluation -= (5-num_safe_squares)*(5-num_safe_squares);
            }
        }

        // PST (not needed for eg, every square is worth the same)
        eval_info.mg_evaluation += PST_MG_WHITE_QUEEN[position];
        eval_info.eg_evaluation += STATIC_EG_PIECE_VALUES[Constants::WHITE_QUEEN];
    });
    forEachPiece(Constants::BLACK_QUEEN,board,[&](int position, GameBoard const & game_board) {


        uint64_t attacked_squares = getQueenAttackBits(position, board.allPieces);
        eval_info.blacks_defended_pieces |= attacked_squares & eval_info.black_pieces;

        // pressure on king
        if (attacked_squares & eval_info.white_king_zone_large) {
            any_game_phase_evaluation -= QUEEN_ATTACKS_LARGE_KING_ZONE*__builtin_popcountll(eval_info.white_king_zone_large & attacked_squares);
            if (attacked_squares & eval_info.white_king_zone_small) {
                any_game_phase_evaluation -= QUEEN_ATTACKS_SMALL_KING_ZONE*__builtin_popcountll(eval_info.white_king_zone_small & attacked_squares);
            }
        }

        //attack on queen
        if ((1ULL << position) & eval_info.squares_attacked_by_less_valuable_white_pieces) {
            uint64_t safe_squares = attacked_squares & ~eval_info.black_pieces & ~eval_info.squares_attacked_by_less_valuable_white_pieces;
            int num_safe_squares = __builtin_popcountll(safe_squares);
            if (num_safe_squares < 5) {
                eval_info.eg_evaluation += (5-num_safe_squares)*(5-num_safe_squares);
            }
        }

        // PST
        eval_info.mg_evaluation -= PST_MG_BLACK_QUEEN[position];
        eval_info.eg_evaluation += STATIC_EG_PIECE_VALUES[Constants::BLACK_QUEEN]; // + is correct here! because the value is already negative
    });
    eval_info.mg_evaluation += any_game_phase_evaluation;
    eval_info.eg_evaluation += any_game_phase_evaluation;
}



int evaluateMaterialOnly(GameBoard const &board) {
    int evaluation = 0;
    for (int i = 1; i < 11; i++) {
        evaluation += STATIC_MG_PIECE_VALUES[i] * __builtin_popcountll(board.pieces[i]);
    }
    return board.whiteToMove ? evaluation : -evaluation;
}
