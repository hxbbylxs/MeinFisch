//
// Created by salom on 16.07.2025.
//

#ifndef MOVEGENERATIONCONSTANTS_H
#define MOVEGENERATIONCONSTANTS_H
#include <array>
#include <vector>
#include <cstdint>
#include "Constants.h"

inline constexpr int MAX_NUM_BLOCKER_COMBINATIONS = 4096;

constexpr std::array<const uint32_t,5> precalculated_castle_moves = {
    0ULL,                                                                           // NO_CASTLE
    (Constants::WHITE_KING) | (60 << 4) | (58 << 14) | (Constants::WHITE_QUEEN_SIDE_CASTLE << 24),
    (Constants::WHITE_KING) | (60 << 4) | (62 << 14) | (Constants::WHITE_KING_SIDE_CASTLE << 24),
    (Constants::BLACK_KING) | (4 << 4) | (2 << 14) | (Constants::BLACK_QUEEN_SIDE_CASTLE << 24),
    (Constants::BLACK_KING) | (4 << 4) | (6 << 14) | (Constants::BLACK_KING_SIDE_CASTLE << 24)
    //      piece               from        to          castle
};

constexpr std::array<std::array<uint64_t,2>,5> castle_rook_positions = []() {
    std::array<std::array<uint64_t,2>,5> castle_rook_positions = {};
    castle_rook_positions[Constants::Castle::WHITE_KING_SIDE_CASTLE][0] = 1ULL << 63; //from
    castle_rook_positions[Constants::Castle::WHITE_KING_SIDE_CASTLE][1] = 1ULL << 61; //to
    castle_rook_positions[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE][0] = 1ULL << 56; //from
    castle_rook_positions[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE][1] = 1ULL << 59; //to
    castle_rook_positions[Constants::Castle::BLACK_KING_SIDE_CASTLE][0] = 1ULL << 7; //from
    castle_rook_positions[Constants::Castle::BLACK_KING_SIDE_CASTLE][1] = 1ULL << 5; //to
    castle_rook_positions[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE][0] = 1ULL << 0; //from
    castle_rook_positions[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE][1] = 1ULL << 3; //to

    return castle_rook_positions;
}();

inline constexpr std::array<std::tuple<int,int,int>,4> NON_DIAGONAL_DIRECTIONS = {
    // delta, yLimit, xLimit
    std::make_tuple(-8,0,Constants::IGNORE),                        //North
    std::make_tuple(8,Constants::BOARD_SIZE-1,Constants::IGNORE),  // South
    std::make_tuple(-1,Constants::IGNORE,0),                        // West
    std::make_tuple(1,Constants::IGNORE,Constants::BOARD_SIZE-1)   // East
};
inline constexpr std::array<std::tuple<int,int,int>,4> DIAGONAL_DIRECTIONS = {
    // delta, yLimit, xLimit
    std::make_tuple(-7,0,Constants::BOARD_SIZE-1),                      //Northeast
    std::make_tuple(9,Constants::BOARD_SIZE-1,Constants::BOARD_SIZE-1),   // Southeast
    std::make_tuple(-9,0,0),                                            // Northwest
    std::make_tuple(7,Constants::BOARD_SIZE-1,0)                         // Southwest
};

    //Attack Tables
    //Knight
    inline constexpr std::array<uint64_t,Constants::NUM_SQUARES> knightAttackBitMasks = []() {
        std::array<uint64_t,Constants::NUM_SQUARES> bitmasks = {};
        for (int knight_position = 0; knight_position < Constants::NUM_SQUARES; knight_position++) {
            for (int x_offset = -2; x_offset <= 2; x_offset++) {
                for (int y_offset = -2; y_offset <= 2; y_offset++) {
                    if (abs(x_offset)+abs(y_offset) == 3
                        && (knight_position%Constants::BOARD_SIZE) + x_offset < Constants::BOARD_SIZE
                        && (knight_position%Constants::BOARD_SIZE) + x_offset >= 0
                        && (knight_position/Constants::BOARD_SIZE) + y_offset < Constants::BOARD_SIZE
                        && (knight_position/Constants::BOARD_SIZE) + y_offset >= 0 ) {
                        bitmasks[knight_position] |= (1ULL<<(knight_position+x_offset+Constants::BOARD_SIZE*y_offset));
                    }
                }
            }
        }
        return bitmasks;
    }();

    //Rook and (Queen)
     inline constexpr std::array<uint64_t,Constants::NUM_SQUARES> nonDiagonalSliderBlockerBitMasks = []() {
        std::array<uint64_t,Constants::NUM_SQUARES> bitmasks = {};

        for (int i = 0; i < Constants::NUM_SQUARES; i++) {
            for (int j = 1; j < 7; j++) {
                bitmasks[i] |= (1ULL << i%Constants::BOARD_SIZE + Constants::BOARD_SIZE*j);
                bitmasks[i] |= (1ULL << (i-(i%Constants::BOARD_SIZE) + j));
            }
            bitmasks[i] &= ~(1ULL << i);
        }

        return bitmasks;
    }();
    inline std::array<uint64_t,Constants::NUM_SQUARES> magicNumbersForNonDiagonalSliders = {
        0x41800010208c4000, 0x140009002200340,  0x200081082402200,  0x80100008008084,
        0x50800a8004000801, 0x80020014000980,   0x9100040882002100, 0xa100021220418500,
        0x208000824000b4,   0x280400250012008,  0x421808020001000,  0x4009004810002102,
        0x8901000800110004, 0x4002000200099410, 0x4004001205040810, 0x4309800100004480,
        0x880800031c000,    0x2008404010022002, 0x8400410020001108, 0x5010020181001,
        0x900808008000400,  0x28c808002000400,  0x8200040010080201, 0x420009004084,
        0x4082400080008423, 0x60100040084020,   0x80020082004020f0, 0x48d0008080108800,
        0x4024000808004080, 0x92000200800c0080, 0x10020400082110,   0x440029200034704,
        0x2904000800428,    0x4110082000c00c40, 0xc13103002001,     0x89001002102,
        0x10088010010e4,    0x200200040a001008, 0x20100284000849,   0x300800040800900,
        0x25244008828000,   0x141004402005c001, 0x4410442001010010, 0x1902002008420010,
        0x10080100050010,   0x102000400808100,  0x500104508840006,  0x4000904084020009,
        0x322800100a0c100,  0x102010124804200,  0x1202200108c01100, 0x810000800821080,
        0x28041018010100,   0x4022000400800a80, 0x4080800100020080, 0x2408084081040200,
        0x41026010448001,   0x4200810020451202, 0x30200090084103,   0x1040881001002005,
        0x942001004210802,  0x14e200048801100a, 0x1040083086100904, 0x2022802400408502,
    };
    inline std::array<std::array<uint64_t,MAX_NUM_BLOCKER_COMBINATIONS>,Constants::NUM_SQUARES> nonDiagonalSlidersAttackBitMask = {};


    //Bishop and (Queen)
    inline constexpr std::array<uint64_t,Constants::NUM_SQUARES> diagonalSliderBlockerBitMasks = []() {
        std::array<uint64_t,Constants::NUM_SQUARES> bitmasks = {};

        for (int index = 0; index < Constants::NUM_SQUARES; index++) {
            auto directions = DIAGONAL_DIRECTIONS;

            //sliding from square = index diagonally in each direction until one square ahead of the edge of the board
            for (auto const & [delta,yLimit,xLimit] : directions) {
                int square = index;
                while (square/Constants::BOARD_SIZE != yLimit && square%Constants::BOARD_SIZE != xLimit) {
                    square += delta;
                    if (square/Constants::BOARD_SIZE != yLimit && square%Constants::BOARD_SIZE != xLimit) {
                        bitmasks[index] |= (1ULL << square);
                    }
                }
            }
        }
        return bitmasks;
    }();


inline std::array<uint64_t,Constants::NUM_SQUARES> magicNumbersForDiagonalSliders = {
    0x3111011628004104, 0x469081a008021,    0x131080081001000,  0x1188060440802000,
    0x89104000000010,   0x1102804a01000,    0x1a82584060000,    0x11010090043230,
    0x103004102c384081, 0x901040428404102,  0xa50088a004820,    0x110401880300,
    0x824741044000011,  0x802018280c100025, 0x1a40400a8480800,  0x20100a2012000,
    0x8108002002144800, 0x4410000302020404, 0x28100008084030a0, 0x88010082014108,
    0x204008894200234,  0x1100028080c000,   0x800880402080642,  0x8408220084112803,
    0x8048420009022800, 0x8202808190300,    0xc002018148008400, 0x4080010220040,
    0x5840000802000,    0x113020200c104,    0x1820280821080,    0x8021060212b20108,
    0x141788809401000,  0x80a0202082800,    0x840580500080,     0x331010800050240,
    0x20420400020108,   0x2210014202004106, 0x804c408029201,    0x60a804a128208200,
    0x4008020814002098, 0x600844508026000,  0x10200940c000201,  0x4084c2071020802,
    0xa400081200200,    0xa840801605600,    0x40180804a8820408, 0x84282c8300400201,
    0x74402a4060040,    0x440201900118,     0x802084102c00,     0x414000194040592,
    0x805012022000,     0x1100210010000,    0x44240c68220181,   0xc002040828810014,
    0x802424400a84000,  0x100a004508280258, 0x80291c0040441080, 0x4000108c0400,
    0x8050280810020a03, 0xc200010810300280, 0x40040ca08208,     0xa420881051016020
};
inline std::array<std::array<uint64_t,MAX_NUM_BLOCKER_COMBINATIONS>,Constants::NUM_SQUARES> diagonalSlidersAttackBitMask = {};

    //King
    inline constexpr std::array<uint64_t,Constants::NUM_SQUARES> kingAttackBitMasks = []() {
        std::array<uint64_t,Constants::NUM_SQUARES> bitmasks = {};
        for (int king_position = 0; king_position <Constants::NUM_SQUARES; king_position++) {
            for (int x = -1; x <= 1; x++) {
                if (king_position%Constants::BOARD_SIZE + x >= Constants::BOARD_SIZE || king_position%Constants::BOARD_SIZE +x < 0) continue; //out of bounds in x direction
                for (int y = -1; y <= 1; y++) {
                    if (king_position/Constants::BOARD_SIZE +y >= Constants::BOARD_SIZE || king_position/Constants::BOARD_SIZE +y < 0) continue; // out of bounds in y direction
                    if (abs(x)+abs(y) != 0) {
                        bitmasks[king_position] |= (1ULL<<(king_position+x+Constants::BOARD_SIZE*y));
                    }
                }
            }
        }
        return bitmasks;

    }();

    //Pawns
    inline constexpr int MIN_VALID_PAWN_POSITION = 8;
    inline constexpr int MAX_VALID_PAWN_POSITION = 55;

    inline constexpr std::array<std::array<uint64_t,Constants::NUM_SQUARES>,2> pawnCaptureBitMask = []() {
        std::array<std::array<uint64_t,Constants::NUM_SQUARES>,2> bitmasks = {};
        for (int pawn_position = 0; pawn_position < Constants::NUM_SQUARES; pawn_position++) {
            if (pawn_position%Constants::BOARD_SIZE != 7) {
                if (pawn_position >= MIN_VALID_PAWN_POSITION) bitmasks[0][pawn_position] |= (1ULL << pawn_position-7);
                if (pawn_position <= MAX_VALID_PAWN_POSITION) bitmasks[1][pawn_position] |= (1ULL << pawn_position+9);
            }
            if (pawn_position%Constants::BOARD_SIZE != 0) {
                if (pawn_position >= MIN_VALID_PAWN_POSITION) bitmasks[0][pawn_position] |= (1ULL << pawn_position-9);
                if (pawn_position <= MAX_VALID_PAWN_POSITION) bitmasks[1][pawn_position] |= (1ULL << pawn_position+7);
            }
        }
        return bitmasks;
    }();

    inline constexpr std::array<std::array<uint64_t,Constants::NUM_SQUARES>,2> pawnDoublePushBitMask = []() {
        std::array<std::array<uint64_t,Constants::NUM_SQUARES>,2> bitmasks = {};
        for (int pawn_position = MIN_VALID_PAWN_POSITION; pawn_position <= MAX_VALID_PAWN_POSITION; pawn_position++) {
            if (pawn_position < 16) bitmasks[1][pawn_position] |= (1ULL << pawn_position+16); // blacks double pushes
            if (pawn_position >= 48) bitmasks[0][pawn_position] |= (1ULL << pawn_position-16); // whites double pushes
        }
        return bitmasks;
    }();


    inline auto getRookAttackBits = [](unsigned from, uint64_t pieceOccupancy) {
        uint64_t const blockerBitMask = nonDiagonalSliderBlockerBitMasks[from];
        uint64_t blockerCombination = pieceOccupancy & blockerBitMask;
        uint64_t magicIndex = blockerCombination * magicNumbersForNonDiagonalSliders[from] >> (64 - __builtin_popcountll(blockerBitMask));
        uint64_t possibleMoves = nonDiagonalSlidersAttackBitMask[from][magicIndex];
        return possibleMoves;
    };
    inline auto getBishopAttackBits = [](unsigned from, uint64_t pieceOccupancy) {
        uint64_t const blockerBitmask = diagonalSliderBlockerBitMasks[from];
        uint64_t blockerCombination = pieceOccupancy & blockerBitmask;
        uint64_t magicIndex = blockerCombination * magicNumbersForDiagonalSliders[from] >> (64 - __builtin_popcountll(blockerBitmask));
        uint64_t possibleMoves = diagonalSlidersAttackBitMask[from][magicIndex];
        return possibleMoves;
    };
    inline auto getQueenAttackBits = [](unsigned from, uint64_t pieceOccupancy) {
        uint64_t const d_blockerBitmask = diagonalSliderBlockerBitMasks[from];
        uint64_t const nd_blockerBitmask = nonDiagonalSliderBlockerBitMasks[from];
        uint64_t blockerCombination = pieceOccupancy & nd_blockerBitmask;
        uint64_t magicIndex = blockerCombination * magicNumbersForNonDiagonalSliders[from] >> (64 - __builtin_popcountll(nd_blockerBitmask));
        uint64_t possibleMoves = nonDiagonalSlidersAttackBitMask[from][magicIndex];
        blockerCombination = pieceOccupancy & d_blockerBitmask;
        magicIndex = blockerCombination * magicNumbersForDiagonalSliders[from] >> (64 - __builtin_popcountll(d_blockerBitmask));
        possibleMoves |= diagonalSlidersAttackBitMask[from][magicIndex];
        return possibleMoves;
    };

    inline auto getKnightAttackBits = [](unsigned from, uint64_t pieceOccupancy) {
        return knightAttackBitMasks[from];
    };
    inline auto getKingAttackBits = [](unsigned from, uint64_t pieceOccupancy) {
        return kingAttackBitMasks[from];
    };


void initializeSliderAttackBitMask();
void setSliderAttackBitMasks(bool diagonalSlider, int board_pos);

std::array<uint64_t,MAX_NUM_BLOCKER_COMBINATIONS> generateSliderBlockerCombos(int index, bool diagonalSlider);
uint64_t nextCombination(uint64_t blockerBitMask, std::vector<int> const & positions);
std::vector<int> bitmaskToIndices(uint64_t bitmask);

uint64_t generateSolutionToBlockerCombinationForSliders(uint64_t blockerCombination, int index, bool diagonalSlider);

std::array<uint64_t,Constants::NUM_SQUARES> findMagicNumbers(bool diagonalSlider);
bool checkMagicNumberCandidate(int shift,std::array<uint64_t,MAX_NUM_BLOCKER_COMBINATIONS> const & blockerCombinations, int board_pos, bool diagonalSlider,uint64_t magicNumberCandidate);
uint64_t generateRandomMagicNumberCandidate(int shift);
void printProgress(int board_pos, bool diagonalSliders);

#endif //MOVEGENERATIONCONSTANTS_H
