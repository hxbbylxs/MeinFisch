//
// Created by salom on 16.07.2025.
//

#ifndef EVALUATIONCONSTANTS_H
#define EVALUATIONCONSTANTS_H
#include <array>
#include <cstdint>
#include "Constants.h"


inline constexpr int CHECKMATE_VALUE = 100000;
inline constexpr int LAZY_EVAL_SAFETY_MARGIN = 200;
inline constexpr int FUTILITY_BASE_SAFETY_MARGIN = 280;
inline constexpr int FUTILITY_DEPTH_SAFETY_MARGIN = 100;

inline constexpr int TEMPO_BONUS = 20;

inline constexpr int PAWN_SHIELD = 5;
inline constexpr int PAWN_AHEAD_KING_MISSING = -150;
inline constexpr int PAWN_PROTECTS_OWN_PIECE = 10;
inline constexpr int PAWN_DOUBLED = -10;
inline constexpr int PAWN_ISOLATED = -25;
inline constexpr std::array<int,8> PAWN_PASSED_EG = {0,150,140,120,50,20,0,0};
inline constexpr std::array<int,8> PAWN_PASSED_MG = {0,70,60,50,40,40,20,0};

inline constexpr int KNIGHT_BONUS_PER_SQUARE = 2;

inline constexpr int BISHOP_BONUS_PER_SQUARE = 1;
inline constexpr int BISHOP_BONUS_PER_CENTER_SQUARE = 2;
inline constexpr int BISHOP_XRAY = 15;
inline constexpr int BISHOP_PAIR = 50;

inline constexpr int ROOK_ON_OPEN_LINE = 40;
inline constexpr int ROOK_ON_HALF_OPEN_LINE = 20;
inline constexpr int ROOK_XRAY = 15;
inline constexpr int ROOK_CONNECTED = 10;
inline constexpr int ROOK_IN_KING_ZONE = 20;

inline constexpr int QUEEN_IN_KING_ZONE = 30;
inline constexpr int QUEEN_XRAY = 15;

inline constexpr int PAWN_ATTACKS_KING_ZONE = 2;
inline constexpr int KNIGHT_ATTACKS_KING_ZONE = 3;
inline constexpr int BISHOP_ATTACKS_KING_ZONE = 4;
inline constexpr int ROOK_ATTACKS_KING_ZONE = 5;
inline constexpr int QUEEN_ATTACKS_KING_ZONE = 9;

inline constexpr int KING_ZONE_ATTACKER_EXP_BASE = 3; // 3^num_attackers

inline constexpr uint64_t PST_CENTER_RING = 3ULL << 19 | 15ULL << 26 | 15ULL << 34 | 3ULL << 43;
inline constexpr uint64_t PST_CENTER_SQUARE = 3ULL << 27 | 3ULL << 35;

inline constexpr std::array<int,13> STATIC_MG_PIECE_VALUES = {0,100,-100,320,-320,330,-330,500,-500,900,-900,0,0};
inline constexpr std::array<int,13> STATIC_EG_PIECE_VALUES = {0,120,-120,330,-330,340,-340,520,-520,940,-940,0,0};


// PST (Piece Square Table) assigns a value to a piece based only on its position

inline constexpr std::array<int,64> PST_MG_WHITE_KING = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
     10,  10,   -5,  -5,  -5,  -5,  10,  10,
     20,  30,  10,   -5,   0,  10,  30,  20
};

inline constexpr std::array<int,64> PST_EG_WHITE_KING = {
    -10,   0,   0,   0,   0,   0,   0, -10,
    -20,   5,   0,  10,  10,   0,   5, -20,
    -20,   0,  10,  20,  20,  10,   0, -20,
    -30,   0,  20,  30,  30,  20,   0, -30,
    -30,  -5,  20,  30,  30,  20,  -5, -30,
    -30, -10,  10,  20,  20,  10, -10, -30,
    -30, -20, -20, -20, -20, -20, -30, -30,
    -50, -40, -40, -40, -40, -40, -40, -50
};

inline constexpr std::array<int,64> PST_MG_BLACK_KING = {
    20,  30,  10,   -5,   0,  10,  30,  20,
    10,  10,   -5,  -5,  -5,  -5,  10,  10,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
};

inline constexpr std::array<int,64> PST_EG_BLACK_KING = {
    -50, -40, -40, -40, -40, -40, -40, -50,
    -30, -20, -20, -20, -20, -20, -30, -30,
    -30, -10,  10,  20,  20,  10, -10, -30,
    -30,  -5,  20,  30,  30,  20,  -5, -30,
    -20,   0,  10,  20,  20,  10,   0, -20,
    -20,   5,   0,  10,  10,   0,   5, -20,
    -10,   0,   0,   0,   0,   0,   0, -10
};

inline constexpr std::array<int,64> PST_MG_KNIGHT = {
    280, 290, 300, 300, 300, 300, 290, 280,
    290, 300, 310, 320, 320, 310, 300, 290,
    300, 310, 330, 335, 335, 330, 310, 300,
    300, 320, 335, 340, 340, 335, 320, 300,
    300, 320, 335, 340, 340, 335, 320, 300,
    300, 310, 330, 335, 335, 330, 310, 300,
    290, 300, 310, 320, 320, 310, 300, 290,
    280, 290, 300, 300, 300, 300, 290, 280
};

inline constexpr std::array<int,64> PST_EG_KNIGHT = {
    290, 300, 310, 310, 310, 310, 300, 290,
    300, 310, 320, 330, 330, 320, 310, 300,
    310, 320, 335, 340, 340, 335, 320, 310,
    310, 330, 340, 345, 345, 340, 330, 310,
    310, 330, 340, 345, 345, 340, 330, 310,
    310, 320, 335, 340, 340, 335, 320, 310,
    300, 310, 320, 330, 330, 320, 310, 300,
    280, 300, 310, 310, 310, 310, 300, 290
};


inline constexpr std::array<int,64> PST_MG_BISHOP = {
    310, 315, 315, 315, 315, 315, 315, 310,
    315, 325, 330, 330, 330, 330, 325, 315,
    315, 330, 335, 335, 335, 335, 330, 315,
    315, 330, 335, 340, 340, 335, 330, 315,
    315, 330, 335, 340, 340, 335, 330, 315,
    315, 330, 335, 335, 335, 335, 330, 315,
    315, 325, 330, 330, 330, 330, 325, 315,
    310, 315, 315, 315, 315, 315, 315, 310
};
inline constexpr std::array<int,64> PST_EG_BISHOP = {
    320, 325, 325, 325, 325, 325, 325, 320,
    325, 335, 340, 340, 340, 340, 335, 325,
    325, 340, 345, 345, 345, 345, 340, 325,
    325, 340, 345, 350, 350, 345, 340, 325,
    325, 340, 345, 350, 350, 345, 340, 325,
    325, 340, 345, 345, 345, 345, 340, 325,
    325, 335, 340, 340, 340, 340, 335, 325,
    320, 325, 325, 325, 325, 325, 325, 320
};

inline constexpr std::array<int,64> PST_MG_WHITE_QUEEN = {
    880, 900, 900, 900, 900, 900, 900, 880,
    895, 900, 900, 900, 900, 900, 900, 895,
    895, 900, 900, 900, 900, 900, 900, 895,
    895, 900, 900, 900, 900, 900, 900, 895,
    895, 900, 900, 900, 900, 900, 900, 895,
    895, 900, 900, 900, 900, 900, 900, 895,
    890, 900, 900, 900, 900, 900, 900, 890,
    870, 880, 890, 910, 890, 890, 880, 870
};
inline constexpr std::array<int,64> PST_MG_BLACK_QUEEN = {
    870, 880, 890, 910, 890, 890, 880, 870,
    890, 900, 900, 900, 900, 900, 900, 890,
    895, 900, 900, 900, 900, 900, 900, 895,
    895, 900, 900, 900, 900, 900, 900, 895,
    895, 900, 900, 900, 900, 900, 900, 895,
    895, 900, 900, 900, 900, 900, 900, 895,
    895, 900, 900, 900, 900, 900, 900, 895,
    880, 900, 900, 900, 900, 900, 900, 880
};

inline constexpr std::array<int,64> PST_MG_WHITE_PAWN = {
    0,   0,   0,   0,   0,   0,   0,   0,
  150, 150, 150, 150, 150, 150, 150, 150,
  120, 120, 130, 140, 140, 130, 120, 120,
  100, 100, 115, 125, 125, 115, 100, 100,
   90,  80, 115, 120, 120, 115,  80,  90,
  105,  90,  90, 100, 100,  90,  90, 105,
  105, 100, 100,  80,  80, 100, 100, 105,
    0,   0,   0,   0,   0,   0,   0,   0
};
inline constexpr std::array<int,64> PST_MG_BLACK_PAWN = {
    0,   0,   0,   0,   0,   0,   0,   0,
    105, 100, 100,  80,  80, 100, 100, 105,
    105,  90,  90, 100, 100,  90,  90, 105,
    90,  80, 115, 120, 120, 115,  80,  90,
    100, 100, 115, 125, 125, 115, 100, 100,
    120, 120, 130, 140, 140, 130, 120, 120,
    150, 150, 150, 150, 150, 150, 150, 150,
    0,   0,   0,   0,   0,   0,   0,   0
};
inline constexpr std::array<int,64> PST_EG_WHITE_PAWN = {
    0,   0,   0,   0,   0,   0,   0,   0,
  150, 150, 150, 150, 150, 150, 150, 150,
  140, 140, 140, 140, 140, 140, 140, 140,
  130, 130, 130, 130, 130, 130, 130, 130,
  120, 120, 120, 120, 120, 120, 120, 120,
  105, 105, 105, 105, 105, 105, 105, 105,
  100, 100, 100, 100, 100, 100, 100, 100,
    0,   0,   0,   0,   0,   0,   0,   0
};
inline constexpr std::array<int,64> PST_EG_BLACK_PAWN = {
    0,   0,   0,   0,   0,   0,   0,   0,
    100, 100, 100, 100, 100, 100, 100, 100,
    105, 105, 105, 105, 105, 105, 105, 105,
    120, 120, 120, 120, 120, 120, 120, 120,
    130, 130, 130, 130, 130, 130, 130, 130,
    140, 140, 140, 140, 140, 140, 140, 140,
    150, 150, 150, 150, 150, 150, 150, 150,
    0,   0,   0,   0,   0,   0,   0,   0
};

inline constexpr std::array<int,64> PST_MG_WHITE_ROOK = {
    500, 500, 500, 500, 500, 500, 500, 500,
    520, 520, 520, 520, 520, 520, 520, 520,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    495, 500, 500, 500, 500, 500, 500, 495,
    500, 500, 500, 510, 510, 505, 500, 500
};
inline constexpr std::array<int,64> PST_MG_BLACK_ROOK = {
    500, 500, 500, 510, 510, 505, 500, 500,
    495, 500, 500, 500, 500, 500, 500, 495,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    520, 520, 520, 520, 520, 520, 520, 520,
    500, 500, 500, 500, 500, 500, 500, 500
};

inline constexpr std::array<int,64> PST_EG_WHITE_ROOK = {
    520, 520, 520, 520, 520, 520, 520, 520,
    530, 530, 530, 530, 530, 530, 530, 530,
    520, 520, 520, 520, 520, 520, 520, 520,
    520, 520, 520, 520, 520, 520, 520, 520,
    520, 520, 520, 520, 520, 520, 520, 520,
    520, 520, 520, 520, 520, 520, 520, 520,
    520, 520, 520, 520, 520, 520, 520, 520,
    520, 520, 520, 520, 520, 520, 520, 520,
};
inline constexpr std::array<int,64> PST_EG_BLACK_ROOK = {
    520, 520, 520, 520, 520, 520, 520, 520,
    520, 520, 520, 520, 520, 520, 520, 520,
    520, 520, 520, 520, 520, 520, 520, 520,
    520, 520, 520, 520, 520, 520, 520, 520,
    520, 520, 520, 520, 520, 520, 520, 520,
    520, 520, 520, 520, 520, 520, 520, 520,
    530, 530, 530, 530, 530, 530, 530, 530,
    520, 520, 520, 520, 520, 520, 520, 520,
};


// Bitmasks

    inline constexpr std::array<uint64_t,8> LINE_BITMASKS = []() {
        std::array<uint64_t,8> result = {};
        for (int i = 0; i < 64; i++) {
            result[i%8] = 1ULL << i;
        }
        return result;
    }();

    inline constexpr std::array<uint64_t,8> ISOLATED_PAWN_BITMASK = []() {
        std::array<uint64_t,8> result = {};
        for (int i = 0; i < 8; i++) {
            if (i != 7) result[i] |= LINE_BITMASKS[i+1];
            if (i != 0) result[i] |= LINE_BITMASKS[i-1];
        }
        return result;
    }();

    inline constexpr std::array<std::array<uint64_t,64>,2> PASSED_PAWN_BITMASK = []() {
        std::array<std::array<uint64_t,64>,2> result = {};
        for (int i = 0; i < 64; i++) {
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    if (abs(i%8 - x) <= 1) {
                        if (y > i/8) {
                            result[0][8*y + x] |= 1ULL << i;
                        }
                        if (y < i/8) {
                            result[1][8*y + x] |= 1ULL << i;
                        }
                    }
                }
            }
        }
        return result;
    }();


    inline constexpr std::array<uint64_t,64> KING_PAWN_SHIELD_BITMASK = []() {
        std::array<uint64_t,64> result = {};
        for (int i = 0; i < 32; i++) {
            result[i] |= 1ULL << i+8;
            if (i % Constants::BOARD_SIZE != 0) result[i] |= 1ULL << i+7;
            if (i % Constants::BOARD_SIZE != Constants::BOARD_SIZE-1) result[i] |= 1ULL << i+9;
        }
        for (int i = 32; i < 64; i++) {
            result[i] |= 1ULL << i-8;
            if (i % Constants::BOARD_SIZE != 0) result[i] |= 1ULL << i-9;
            if (i % Constants::BOARD_SIZE != Constants::BOARD_SIZE-1) result[i] |= 1ULL << i-7;
        }

        return result;
    }();

#endif //EVALUATIONCONSTANTS_H
