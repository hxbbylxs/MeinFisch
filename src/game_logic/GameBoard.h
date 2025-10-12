//
// Created by salom on 24.06.2025.
//

#ifndef GAMEBOARD_H
#define GAMEBOARD_H
#include <cstdint>
#include <array>
#include <unordered_map>
#include "Move.h"

#include "Utils.h"

struct GameBoard {
    std::array<uint64_t, 13> pieces;
    uint64_t white_pieces;
    uint64_t black_pieces;
    bool whiteToMove;
    std::array<bool, 5> castleInformation;
    int enPassant = -1;
    uint8_t plies;
    uint8_t moves;
    uint64_t zobristHash;
    std::unordered_map<uint64_t,int> board_positions = {}; // history of positions for threefold repetition check

    //Constructor
    GameBoard(
        std::array<uint64_t, 13> const & pieces,
        bool whiteToMove,
        std::array<bool, 5> const & castleInformation,
        int enPassant,
        uint8_t plies,
        uint8_t moves);

    GameBoard();

    //functions
    [[nodiscard]]
    bool isCheck(bool whiteKing) const;
    [[nodiscard]]
    bool noLegalMoves() const;

    void applyPseudoLegalMove(Move move);
    void unmakeMove(Move move, int enPassant, std::array<bool, 5> castleRights, uint8_t plies, uint64_t hash_before);
    void updateCastleInformation(Move mv);
    void moveRookForCastle(unsigned castle, bool unmake);
    void handlePawnSpecialCases(Move mv);
    void makeNullMove();
    void unmakeNullMove(int en_passant, uint64_t zobrist_hash);

    void addNewBoardPosition(uint64_t hash);
    void removeBoardPosition(uint64_t hash);

    //operator
    bool operator==(GameBoard const & other) const;
};




template<typename F>
void forEachPiece(Constants::Piece piece, GameBoard const & board, F operation) {
    uint64_t bitboard = board.pieces[piece];
    while (bitboard != 0) {
        auto position = static_cast<unsigned>(counttzll(bitboard));
        operation(position, board);
        bitboard = bitboard & (bitboard - 1); // removes last bit
    }
}

#endif //GAMEBOARD_H
