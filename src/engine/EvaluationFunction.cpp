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
    int mg_evaluation = 0;
    int eg_evaluation = 0;

    // the king zone is the 3x3 square around the king
    uint64_t white_king_zone = board.pieces[Constants::Piece::WHITE_KING] | kingAttackBitMasks[__builtin_ctzll(board.pieces[Constants::Piece::WHITE_KING])];
    uint64_t black_king_zone = board.pieces[Constants::Piece::BLACK_KING] | kingAttackBitMasks[__builtin_ctzll(board.pieces[Constants::Piece::BLACK_KING])];

    int num_attackers_white_king_zone = 0;
    int num_attackers_black_king_zone = 0;
    // does king help attacking?
    if (white_king_zone & black_king_zone) {
        num_attackers_white_king_zone++;
        num_attackers_black_king_zone++;
    }

    uint64_t whitePawnAttackedSquares = 0;
    uint64_t blackPawnAttackedSquares = 0;

    evaluatePawns(board, mg_evaluation, eg_evaluation, white_king_zone, black_king_zone,whitePawnAttackedSquares, blackPawnAttackedSquares, num_attackers_white_king_zone,num_attackers_black_king_zone);
    evaluateKnights(board, mg_evaluation, eg_evaluation, white_king_zone, black_king_zone,whitePawnAttackedSquares, blackPawnAttackedSquares, num_attackers_white_king_zone,num_attackers_black_king_zone);
    evaluateBishops(board, mg_evaluation, eg_evaluation, white_king_zone, black_king_zone,whitePawnAttackedSquares, blackPawnAttackedSquares, num_attackers_white_king_zone,num_attackers_black_king_zone);
    evaluateRooks(board, mg_evaluation, eg_evaluation, white_king_zone, black_king_zone, num_attackers_white_king_zone,num_attackers_black_king_zone);
    evaluateQueen(board, mg_evaluation, eg_evaluation, white_king_zone, black_king_zone, num_attackers_white_king_zone,num_attackers_black_king_zone);
    evaluateKing(board, mg_evaluation,eg_evaluation);


    // linear interpolation between middle game and end game
    evaluation = (game_phase_score*mg_evaluation + (24-game_phase_score)*eg_evaluation)/24;

    // exponential danger when many pieces attack the king
    evaluation += pow(KING_ZONE_ATTACKER_EXP_BASE,num_attackers_black_king_zone);
    evaluation -= pow(KING_ZONE_ATTACKER_EXP_BASE,num_attackers_white_king_zone);

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


void evaluateKing(GameBoard const &board, int & mg_evaluation, int & eg_evaluation) {
    int black_king_position = __builtin_ctzll(board.pieces[Constants::Piece::BLACK_KING]);
    int white_king_position = __builtin_ctzll(board.pieces[Constants::Piece::WHITE_KING]);
    uint64_t pawns_ahead_black_king = KING_PAWN_SHIELD_BITMASK[black_king_position];
    uint64_t pawns_ahead_white_king = KING_PAWN_SHIELD_BITMASK[white_king_position];

    int num_protectors_black = __builtin_popcountll(board.pieces[Constants::Piece::BLACK_PAWN] & pawns_ahead_black_king);
    int num_protectors_white = __builtin_popcountll(board.pieces[Constants::Piece::WHITE_PAWN] & pawns_ahead_white_king);

    // bonus for every pawn on the three squares ahead of the king
    mg_evaluation += PAWN_SHIELD*num_protectors_white - PAWN_SHIELD*num_protectors_black;

    // penalty if the pawn in front of the king is missing
    // & 7 equals % 8 and gives the correct line
    if (!(__builtin_popcountll(LINE_BITMASKS[white_king_position & 7] & board.pieces[Constants::Piece::WHITE_PAWN]))) mg_evaluation += PAWN_AHEAD_KING_MISSING;
    if (!(__builtin_popcountll(LINE_BITMASKS[black_king_position & 7] & board.pieces[Constants::Piece::BLACK_PAWN]))) mg_evaluation -= PAWN_AHEAD_KING_MISSING;

    // evaluation only based on the position of the king on the board
    mg_evaluation += PST_MG_WHITE_KING[white_king_position];
    mg_evaluation -= PST_MG_BLACK_KING[black_king_position];
    eg_evaluation += PST_EG_WHITE_KING[white_king_position];
    eg_evaluation -= PST_EG_BLACK_KING[black_king_position];

    // Squares attacked around the king are evaluated as pressure in the evaluatePiece functions
    // due to performance, although they would belong here.

}

// modifies the values passed by reference
void evaluatePawns(GameBoard const &board, int & mg_evaluation, int & eg_evaluation, uint64_t white_king_zone, uint64_t black_king_zone,uint64_t & whitePawnAttackedSquares, uint64_t & blackPawnAttackedSquares, int & num_attackers_white_king_zone, int & num_attackers_black_king_zone) {
    int anyGamePhaseEvaluation = 0; // will be added to mg evaluation as well as eg evaluation

    forEachPiece(Constants::WHITE_PAWN,board,[&](int position, GameBoard const & game_board) {
        // PST
        mg_evaluation += PST_MG_WHITE_PAWN[position];
        eg_evaluation += PST_EG_WHITE_PAWN[position];

        uint64_t attacked_squares = pawnCaptureBitMask[0][position];
        whitePawnAttackedSquares |= attacked_squares; // will be needed in knight/bishop evaluation

        // counts all pieces diagonally in front of the pawn (defends own piece or attacks enemy piece)
        anyGamePhaseEvaluation += PAWN_PROTECTS_OWN_PIECE*__builtin_popcountll(board.allPieces & attacked_squares);

        // pressure on king
        if (attacked_squares & black_king_zone) {
            num_attackers_black_king_zone++;
            anyGamePhaseEvaluation += PAWN_ATTACKS_KING_ZONE*__builtin_popcountll(black_king_zone & attacked_squares);
        }

        // passed, doubled, isolated pawn
        if (!(PASSED_PAWN_BITMASK[0][position] & board.pieces[Constants::Piece::BLACK_PAWN])) {
            mg_evaluation += PAWN_PASSED_MG[position/8];
            eg_evaluation += PAWN_PASSED_EG[position/8];
        }
        if (!(ISOLATED_PAWN_BITMASK[position&7] & board.pieces[Constants::Piece::WHITE_PAWN])) {
            anyGamePhaseEvaluation += PAWN_ISOLATED;
        }
        if (__builtin_popcountll(board.pieces[Constants::WHITE_PAWN] & LINE_BITMASKS[position&7]) > 1) anyGamePhaseEvaluation += PAWN_DOUBLED;
    });
    forEachPiece(Constants::BLACK_PAWN,board,[&](int position, GameBoard const & game_board) {
        //PST
        mg_evaluation -= PST_MG_BLACK_PAWN[position];
        eg_evaluation -= PST_EG_BLACK_PAWN[position];

        uint64_t attacked_squares = pawnCaptureBitMask[1][position];
        blackPawnAttackedSquares |= attacked_squares;

        //Structure
        anyGamePhaseEvaluation -= PAWN_PROTECTS_OWN_PIECE*__builtin_popcountll(board.allPieces & attacked_squares);

        //King Pressure
        if (attacked_squares & white_king_zone) {
            num_attackers_white_king_zone++;
            anyGamePhaseEvaluation -= PAWN_ATTACKS_KING_ZONE*__builtin_popcountll(white_king_zone & attacked_squares);
        }

        // passed, doubled, isolated
        if (!(PASSED_PAWN_BITMASK[1][position] & board.pieces[Constants::Piece::WHITE_PAWN])) {
            mg_evaluation -= PAWN_PASSED_MG[8-(position/8)];
            eg_evaluation -= PAWN_PASSED_EG[8-(position/8)];
        }
        if (!(ISOLATED_PAWN_BITMASK[position&7] & board.pieces[Constants::Piece::BLACK_PAWN])) {
            anyGamePhaseEvaluation -= PAWN_ISOLATED;
        }
        if (__builtin_popcountll(board.pieces[Constants::BLACK_PAWN] & LINE_BITMASKS[position&7]) > 1) anyGamePhaseEvaluation -= PAWN_DOUBLED;
    });
    mg_evaluation += anyGamePhaseEvaluation;
    eg_evaluation += anyGamePhaseEvaluation;
}


void evaluateKnights(GameBoard const &board, int & mg_evaluation, int & eg_evaluation, uint64_t white_king_zone, uint64_t black_king_zone,uint64_t whitePawnAttackedSquares, uint64_t blackPawnAttackedSquares, int & num_attackers_white_king_zone, int & num_attackers_black_king_zone) {
    int anyGamePhaseEvaluation = 0;

    forEachPiece(Constants::WHITE_KNIGHT,board,[&](int position, GameBoard const & game_board) {
        uint64_t attacked_squares = knightAttackBitMasks[position];

        // mobility
        uint64_t possible_moves = ~board.allPieces & attacked_squares & ~blackPawnAttackedSquares;
        anyGamePhaseEvaluation += KNIGHT_BONUS_PER_SQUARE*__builtin_popcountll(possible_moves);

        // king pressure
        if (attacked_squares & black_king_zone) {
            num_attackers_black_king_zone++;
            anyGamePhaseEvaluation += KNIGHT_ATTACKS_KING_ZONE*__builtin_popcountll(black_king_zone & attacked_squares);
        }
        // PST
        mg_evaluation += PST_MG_KNIGHT[position];
        eg_evaluation += PST_EG_KNIGHT[position];
    });
    forEachPiece(Constants::BLACK_KNIGHT,board,[&](int position, GameBoard const & game_board) {
        uint64_t attacked_squares = knightAttackBitMasks[position];

        //mobility
        uint64_t possible_moves = ~board.allPieces & attacked_squares & ~whitePawnAttackedSquares;
        anyGamePhaseEvaluation -= KNIGHT_BONUS_PER_SQUARE*__builtin_popcountll(possible_moves);

        // king pressure
        if (attacked_squares & white_king_zone) {
            num_attackers_white_king_zone++;
            anyGamePhaseEvaluation -= KNIGHT_ATTACKS_KING_ZONE*__builtin_popcountll(black_king_zone & attacked_squares);
        }
        // PST
        mg_evaluation -= PST_MG_KNIGHT[position];
        eg_evaluation -= PST_EG_KNIGHT[position];
});
    mg_evaluation += anyGamePhaseEvaluation;
    eg_evaluation += anyGamePhaseEvaluation;
}


void evaluateBishops(GameBoard const &board, int & mg_evaluation, int & eg_evaluation, uint64_t white_king_zone, uint64_t black_king_zone,uint64_t whitePawnAttackedSquares, uint64_t blackPawnAttackedSquares, int & num_attackers_white_king_zone, int & num_attackers_black_king_zone) {
    int anyGamePhaseEvaluation = 0;
    forEachPiece(Constants::WHITE_BISHOP,board,[&](int position, GameBoard const & game_board) {
        uint64_t attacked_squares = getBishopAttackBits(position,board.allPieces); // includes "attack" on own pieces

        // mobility
        uint64_t possible_moves = ~board.allPieces & attacked_squares & ~blackPawnAttackedSquares;
        anyGamePhaseEvaluation += BISHOP_BONUS_PER_SQUARE*__builtin_popcountll(possible_moves);
        anyGamePhaseEvaluation += BISHOP_BONUS_PER_CENTER_SQUARE*__builtin_popcountll(PST_CENTER_SQUARE & possible_moves);

        //TODO xray on rook/queen/king

        // king pressure
        if (attacked_squares & black_king_zone) {
            num_attackers_black_king_zone++;
            anyGamePhaseEvaluation += BISHOP_ATTACKS_KING_ZONE*__builtin_popcountll(black_king_zone & attacked_squares);
        }

        // PST
        mg_evaluation += PST_MG_BISHOP[position];
        eg_evaluation += PST_EG_BISHOP[position];
    });
    forEachPiece(Constants::BLACK_BISHOP,board,[&](int position, GameBoard const & game_board) {
        uint64_t attacked_squares = getBishopAttackBits(position,board.allPieces); // includes "attack" on own pieces

        // mobility
        uint64_t possible_moves = ~board.allPieces & attacked_squares & ~whitePawnAttackedSquares;
        anyGamePhaseEvaluation -= BISHOP_BONUS_PER_SQUARE*__builtin_popcountll(possible_moves);
        anyGamePhaseEvaluation -= BISHOP_BONUS_PER_CENTER_SQUARE*__builtin_popcountll(PST_CENTER_SQUARE & possible_moves);

        //TODO xray

        // king pressure
        if (attacked_squares & white_king_zone) {
            num_attackers_white_king_zone++;
            anyGamePhaseEvaluation -= BISHOP_ATTACKS_KING_ZONE*__builtin_popcountll(white_king_zone & attacked_squares);
        }

        // PST
        mg_evaluation -= PST_MG_BISHOP[position];
        eg_evaluation -= PST_EG_BISHOP[position];
    });
    mg_evaluation += anyGamePhaseEvaluation;
    eg_evaluation += anyGamePhaseEvaluation;
    if (__builtin_popcountll(board.pieces[Constants::Piece::WHITE_BISHOP]) >= 2) eg_evaluation += BISHOP_PAIR;
    if (__builtin_popcountll(board.pieces[Constants::Piece::BLACK_BISHOP]) >= 2) eg_evaluation -= BISHOP_PAIR;
}


void evaluateRooks(GameBoard const &board, int & mg_evaluation, int & eg_evaluation, uint64_t white_king_zone, uint64_t black_king_zone, int & num_attackers_white_king_zone, int & num_attackers_black_king_zone) {
    uint64_t pawns = board.pieces[Constants::Piece::WHITE_PAWN] | board.pieces[Constants::Piece::BLACK_PAWN];
    int anyGamePhaseEvaluation = 0;

    // rook standing in king zone
    anyGamePhaseEvaluation += ROOK_IN_KING_ZONE*(__builtin_popcountll(board.pieces[Constants::WHITE_ROOK] & black_king_zone) - __builtin_popcountll(board.pieces[Constants::BLACK_ROOK] & white_king_zone));

    forEachPiece(Constants::WHITE_ROOK,board,[&](int position, GameBoard const & game_board) {

        // open lines
        int num_pawns_on_line = __builtin_popcountll(LINE_BITMASKS[position & 7] & pawns);
        if (num_pawns_on_line == 0) anyGamePhaseEvaluation += ROOK_ON_OPEN_LINE;
        if (num_pawns_on_line == 1) anyGamePhaseEvaluation += ROOK_ON_HALF_OPEN_LINE;

        // pressure on king
        uint64_t rook_attack_bits = getRookAttackBits(position,board.allPieces);
        if (black_king_zone & rook_attack_bits) {
            num_attackers_black_king_zone++;
            anyGamePhaseEvaluation += ROOK_ATTACKS_KING_ZONE * __builtin_popcountll(black_king_zone & rook_attack_bits);
        }

        // TODO xray und connected rooks

        // PST
        mg_evaluation += PST_MG_WHITE_ROOK[position];
        eg_evaluation += PST_EG_WHITE_ROOK[position];
    });

    forEachPiece(Constants::BLACK_ROOK,board,[&](int position, GameBoard const & game_board) {

        // open lines
        int num_pawns_on_line = __builtin_popcountll(LINE_BITMASKS[position & 7] & pawns);
        if (num_pawns_on_line == 0) anyGamePhaseEvaluation -= ROOK_ON_OPEN_LINE;
        if (num_pawns_on_line == 1) anyGamePhaseEvaluation -= ROOK_ON_HALF_OPEN_LINE;


        // pressure on king
        uint64_t rook_attack_bits = getRookAttackBits(position,board.allPieces);
        if (white_king_zone & rook_attack_bits) {
            num_attackers_white_king_zone++;
            anyGamePhaseEvaluation -= ROOK_ATTACKS_KING_ZONE * __builtin_popcountll(white_king_zone & rook_attack_bits);
        }

        // TODO xray und connected rooks

        // PST
        mg_evaluation -= PST_MG_WHITE_ROOK[position];
        eg_evaluation -= PST_EG_WHITE_ROOK[position];
    });
    mg_evaluation += anyGamePhaseEvaluation;
    eg_evaluation += anyGamePhaseEvaluation;
}



void evaluateQueen(GameBoard const &board, int & mg_evaluation, int & eg_evaluation, uint64_t white_king_zone, uint64_t black_king_zone, int & num_attackers_white_king_zone, int & num_attackers_black_king_zone) {
    int anyGamePhaseEvaluation = 0;

    // queen standing in king zone is big problem
    anyGamePhaseEvaluation += QUEEN_IN_KING_ZONE*(__builtin_popcountll(board.pieces[Constants::WHITE_QUEEN] & black_king_zone) - __builtin_popcountll(board.pieces[Constants::BLACK_QUEEN] & white_king_zone));

    forEachPiece(Constants::WHITE_QUEEN,board,[&](int position, GameBoard const & game_board) {

        // pressure on king
        uint64_t queen_attack_bits = getQueenAttackBits(position, board.allPieces);
        if (queen_attack_bits & black_king_zone) {
            num_attackers_black_king_zone++;
            anyGamePhaseEvaluation += QUEEN_ATTACKS_KING_ZONE * __builtin_popcountll(black_king_zone & queen_attack_bits);
        }

        // PST (not needed for eg, every square is worth the same)
        mg_evaluation += PST_MG_WHITE_QUEEN[position];
        eg_evaluation += STATIC_EG_PIECE_VALUES[Constants::WHITE_QUEEN];
    });
    forEachPiece(Constants::BLACK_QUEEN,board,[&](int position, GameBoard const & game_board) {

        // pressure on king
        uint64_t queen_attack_bits = getQueenAttackBits(position, board.allPieces);
        if (queen_attack_bits & white_king_zone) {
            num_attackers_white_king_zone++;
            anyGamePhaseEvaluation += QUEEN_ATTACKS_KING_ZONE * __builtin_popcountll(white_king_zone & queen_attack_bits);
        }

        // PST
        mg_evaluation -= PST_MG_BLACK_QUEEN[position];
        eg_evaluation += STATIC_EG_PIECE_VALUES[Constants::BLACK_QUEEN]; // + is correct here!
    });
    mg_evaluation += anyGamePhaseEvaluation;
    eg_evaluation += anyGamePhaseEvaluation;
}



int evaluateMaterialOnly(GameBoard const &board) {
    int evaluation = 0;
    for (int i = 1; i < 11; i++) {
        evaluation += STATIC_MG_PIECE_VALUES[i] * __builtin_popcountll(board.pieces[i]);
    }
    return board.whiteToMove ? evaluation : -evaluation;
}
