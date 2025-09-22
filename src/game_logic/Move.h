//
// Created by salom on 24.06.2025.
//

#ifndef MOVE_H
#define MOVE_H
#include <cstdint>
#include "Constants.h"


struct Move {
    uint32_t value;
    // 0    4      10   14     20   24
    // pppp ffffff cccc tttttt pppp cccc

    // getter
    [[nodiscard]] constexpr unsigned piece() const noexcept {return (value & Constants::move_decoding_bitmasks[Constants::MoveDecoding::PIECE]);}
    [[nodiscard]] constexpr unsigned from() const noexcept {return ((value & Constants::move_decoding_bitmasks[Constants::MoveDecoding::FROM]) >> 4);}
    [[nodiscard]] constexpr unsigned capture() const noexcept {return ((value & Constants::move_decoding_bitmasks[Constants::MoveDecoding::CAPTURE]) >> 10);}
    [[nodiscard]] constexpr unsigned to() const noexcept {return ((value & Constants::move_decoding_bitmasks[Constants::MoveDecoding::TO]) >> 14);}
    [[nodiscard]] constexpr unsigned promotion() const noexcept {return ((value & Constants::move_decoding_bitmasks[Constants::MoveDecoding::PROMOTION]) >> 20);}
    [[nodiscard]] constexpr unsigned castle() const noexcept {return ((value & Constants::move_decoding_bitmasks[Constants::MoveDecoding::CASTLE]) >> 24);}

    // setter
    constexpr void set_piece(Constants::Piece piece) noexcept {
        value = (value & ~Constants::move_decoding_bitmasks[Constants::MoveDecoding::PIECE]) | piece;
    }
    constexpr void set_from(unsigned from) noexcept {
        value = (value & ~Constants::move_decoding_bitmasks[Constants::MoveDecoding::FROM]) | (from << 4);
    }
    constexpr void set_capture(Constants::Piece capture) noexcept {
        value = (value & ~Constants::move_decoding_bitmasks[Constants::MoveDecoding::CAPTURE]) | (capture << 10);
    }
    constexpr void set_to(unsigned to) noexcept {
        value = (value & ~Constants::move_decoding_bitmasks[Constants::MoveDecoding::TO]) | (to << 14);
    }
    constexpr void set_promotion(Constants::Piece promotion) noexcept {
        value = (value & ~Constants::move_decoding_bitmasks[Constants::MoveDecoding::PROMOTION]) | (promotion << 20);
    }
    constexpr void set_castle(Constants::Castle castle) noexcept {
        value = (value & ~Constants::move_decoding_bitmasks[Constants::MoveDecoding::CASTLE]) | (castle << 24);
    }

    Move(Constants::Piece piece, unsigned from, Constants::Piece capture, unsigned to, Constants::Piece promotion, Constants::Castle castle) noexcept : value(0) {
        set_piece(piece);
        set_from(from);
        set_capture(capture);
        set_to(to);
        set_promotion(promotion);
        set_castle(castle);
    };
    Move(Constants::Piece piece, unsigned from, Constants::Piece capture, unsigned to) noexcept : value(0) {
        set_piece(piece);
        set_from(from);
        set_capture(capture);
        set_to(to);
    };
    Move(uint32_t value) noexcept : value(value) {};
    Move() noexcept : value(0) {};

    bool operator==(const Move & other) const {
        return value == other.value;
    }
    bool operator==(uint32_t val) const {
        return value == val;
    }
};


#endif //MOVE_H
