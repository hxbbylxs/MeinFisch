//
// Created by salom on 24.06.2025.
//

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <random>
#include <string>
#include <cstdint>
#include <array>



namespace Constants {

    inline constexpr int INFINITE = INT32_MAX;
    inline constexpr int IGNORE = -1;
    inline constexpr int DEFAULT_TIME_LIMIT = 10000;
    inline constexpr int TIME_IS_UP_FLAG = 200000;

    enum Piece {
        NONE = 0,
        WHITE_PAWN = 1,
        BLACK_PAWN = 2,
        WHITE_KNIGHT = 3,
        BLACK_KNIGHT = 4,
        WHITE_BISHOP = 5,
        BLACK_BISHOP = 6,
        WHITE_ROOK = 7,
        BLACK_ROOK = 8,
        WHITE_QUEEN = 9,
        BLACK_QUEEN = 10,
        WHITE_KING = 11,
        BLACK_KING = 12
    };

    enum Castle {
        NO_CASTLE = 0,
        WHITE_QUEEN_SIDE_CASTLE = 1,
        WHITE_KING_SIDE_CASTLE = 2,
        BLACK_QUEEN_SIDE_CASTLE = 3,
        BLACK_KING_SIDE_CASTLE = 4
    };



    std::string const STANDARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string const STANDARD_FEN_PIECES = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
    inline constexpr int BOARD_SIZE = 8;
    inline constexpr int NUM_SQUARES = 64;

    inline constexpr uint32_t MENU_EXIT_FLAG = 1U << 31;

    inline constexpr int MOVE_INVALID_FLAG = 1 << 31;

    inline constexpr int MAX_RECURSION_DEPTH = 30;

    inline constexpr uint32_t CRITICAL_MOVE_BITMASK = ((1 << 14)-1 -((1<<10)-1)) | 1 << 28;

    enum MoveDecoding {
        PIECE = 0,
        FROM = 1,
        CAPTURE = 2,
        TO = 3,
        PROMOTION = 4,
        CASTLE = 5,
        CHECK = 6
    };

    inline constexpr std::array<uint32_t,7> move_decoding_bitmasks = []() {
        std::array<uint32_t,7> result = {};
        result[PIECE] = (1 << 4)-1;                 //piece
        result[FROM] = (1 << 10)-1 -((1<<4)-1);    //from
        result[CAPTURE] = (1 << 14)-1 -((1<<10)-1);   //capture
        result[TO] = (1 << 20)-1 -((1<<14)-1);   //to
        result[PROMOTION] = (1 << 24)-1 -((1<<20)-1);   //promotion
        result[CASTLE] = (1 << 28)-1 -((1<<24)-1);   //castle
        result[CHECK] = 1 << 28;                    //check

        return result;
    }();

    constexpr std::array<Piece,13> piece_decoding = {
        NONE,
        WHITE_PAWN,
        BLACK_PAWN,
        WHITE_KNIGHT,
        BLACK_KNIGHT,
        WHITE_BISHOP,
        BLACK_BISHOP,
        WHITE_ROOK,
        BLACK_ROOK,
        WHITE_QUEEN,
        BLACK_QUEEN,
        WHITE_KING,
        BLACK_KING
    };

    constexpr std::array<const std::string,13> PIECE_SYMBOLS = {" ","♙","♟","♘","♞","♗","♝","♖","♜","♕","♛","♔","♚"};
    constexpr std::array<const char,13> PIECE_LETTERS = {' ','P','p','N','n','B','b','R','r','Q','q','K','k'};
    constexpr std::array<const std::string,13> PROMOTION_STRING = {" ","p","p","n","n","b","b","r","r","q","q","k","k"};


    inline std::array<std::array<uint64_t,13>,64> ZOBRIST_HASH_VALUES_PIECES = {};
    inline std::array<uint64_t,9> ZOBRIST_HASH_VALUES_ENPASSANT = {
        0xF1C3D2B74E9865AAULL,
        0x38A7BF209D45E31CULL,
        0x8E2D1F64AC9037BDULL,
        0xC95734FE71A8C024ULL,
        0x1A6CDEF820B3F97EULL,
        0x5FDB42A379E68419ULL,
        0xB37E9810CA5D2F4BULL,
        0x62C14DAF7B9E1302ULL,
        0x9C81A3D5EF26788FULL
    };
    inline std::array<uint64_t,5> ZOBRIST_HASH_VALUES_OTHER = {
        0xA3F1C56D8B4E7A92ULL, // white to move
        0x4D9B2C3E17A5F048ULL, // w qside castle
        0x91E7AD6C2F35B1DCULL, // w kside castle
        0x6B48F902CD3A7E1FULL, // b qside castle
        0xD37ACB84E1592F63ULL // b kside castle
    };

    inline constexpr int TTSIZE = 1ULL << 24; // transposition table size

}


inline void initializeZobristHashValues() {
    static std::random_device rd;  // Seed
    static std::mt19937_64 gen(rd());  // 64-bit Mersenne Twister Generator
    static std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);
    for (int i = 0; i < 64; i++) {
        for (int j = 1; j < 13; j++) {
            Constants::ZOBRIST_HASH_VALUES_PIECES[i][j] = dis(gen);
        }
    }
}




#endif //CONSTANTS_H
