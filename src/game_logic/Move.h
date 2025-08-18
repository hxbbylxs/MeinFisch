//
// Created by salom on 24.06.2025.
//

#ifndef MOVE_H
#define MOVE_H
#include <cstdint>
#include "Constants.h"


struct Move {
    // 0    4      10   14     20   24   28
    // pppp ffffff cccc tttttt pppp cccc c
    Constants::Piece piece;
    uint8_t from;
    Constants::Piece captured_piece;
    uint8_t to;
    Constants::Piece pawn_promote_to;
    Constants::Castle castle;
    bool check;
};

uint32_t encodeMove(Move move);
Move decodeMove(uint32_t move);






#endif //MOVE_H
