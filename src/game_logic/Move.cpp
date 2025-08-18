//
// Created by salom on 24.06.2025.
//

#include <vector>
using std::vector;
#include "Move.h"
#include "MoveGeneration.h"

uint32_t encodeMove(Move move) {
    //first 4 bits piece
    uint32_t encoded_move = move.piece;
    //next 6 bits pos from
    encoded_move |= (move.from<<4);
    //next 4 bits captured piece
    encoded_move |= (move.captured_piece<<10);
    //next 6 bits pos to
    encoded_move |= (move.to<<14);
    //next 4 bits promotion
    encoded_move |= (move.pawn_promote_to<<20);
    //next 4 bits castle
    encoded_move |= (move.castle<<24);
    //next bit check
    encoded_move |= (move.check<<28);

    return encoded_move;
}

Move decodeMove(uint32_t move) {
    Constants::Piece piece = Constants::piece_decoding[Constants::move_decoding_bitmasks[Constants::MoveDecoding::PIECE] & move];
    uint8_t from = ((move & Constants::move_decoding_bitmasks[Constants::MoveDecoding::FROM]) >> 4);
    Constants::Piece captured_piece = Constants::piece_decoding[(Constants::move_decoding_bitmasks[Constants::MoveDecoding::CAPTURE] & move) >> 10];
    uint8_t to = ((move & Constants::move_decoding_bitmasks[Constants::MoveDecoding::TO]) >> 14);
    Constants::Piece pawn_promote_to = Constants::piece_decoding[(Constants::move_decoding_bitmasks[Constants::MoveDecoding::PROMOTION] & move) >> 20];
    auto castle = static_cast<Constants::Castle>((move & Constants::move_decoding_bitmasks[Constants::MoveDecoding::CASTLE]) >> 24);
    bool check = move >> 28;
    return {piece,from,captured_piece,to,pawn_promote_to,castle,check};
}





/*vector<uint32_t> pseudoLegalMoves = getPseudoLegalMoves(board,board.whiteToMove);
bool found_pseudo_legal_move = false;
for (uint32_t pseudo_legal_move : pseudoLegalMoves) {
    if (((pseudo_legal_move & Constants::move_decoding_bitmasks[1]) == (input_move & Constants::move_decoding_bitmasks[1]))
        && ((pseudo_legal_move & Constants::move_decoding_bitmasks[3]) == (input_move & Constants::move_decoding_bitmasks[3]))) {
        found_pseudo_legal_move = true;
        input_move = pseudo_legal_move;
        break;
        }
}
if (!found_pseudo_legal_move) return false;*/