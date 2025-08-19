//
// Created by salom on 25.06.2025.
//

#ifndef MOVEGENERATION_H
#define MOVEGENERATION_H

#include "Move.h"
#include <vector>

#include "GameBoard.h"

enum MoveType {
    CAPTURES,
    QUIETS,
    ALL
};

[[nodiscard]]
std::vector<uint32_t> getPseudoLegalMoves(GameBoard const & board, bool forWhite, MoveType type);


void addPseudoLegalPawnMoves(GameBoard const & board, bool forWhite, std::vector<uint32_t> & pseudoLegalMoves, uint64_t ownPieces, uint64_t enemyPieces, MoveType type);
void addPseudoLegalPawnCaptureMoves(GameBoard const &board, bool forWhite, std::vector<uint32_t> &pseudoLegalMoves, uint64_t ownPieces, uint64_t enemyPieces, Constants::Piece piece, int from);
void addPseudoLegalPawnPushMoves(GameBoard const &board, bool forWhite, std::vector<uint32_t> &pseudoLegalMoves, uint64_t ownPieces, uint64_t enemyPieces, Constants::Piece piece, int from);

void addPseudoLegalPromotionMoves(std::vector<uint32_t> & pseudoLegalMoves, uint32_t move, bool forWhite);
void addPseudoLegalCastleMoves(GameBoard const &board, bool forWhite, std::vector<uint32_t> &pseudoLegalMoves, uint64_t ownPieces, uint64_t enemyPieces);

template <typename F>
void addPseudoLegalGenericPieceTypeMoves( GameBoard const &board,
                                bool forWhite,
                                std::vector<uint32_t> &pseudoLegalMoves,
                                uint64_t ownPieces,
                                uint64_t enemyPieces,
                                Constants::Piece piece,
                                F getAttackBitMask,
                                MoveType type);

Constants::Piece getPieceAt(GameBoard const & board, int position, bool pieceIsWhite);

bool isLegalMove(uint32_t move, GameBoard & board);
bool isPseudoLegalMove(uint32_t move, GameBoard const & board);
bool isSquareAttacked(int square, bool attacker_is_white, GameBoard const & board);
bool isCastlingThroughCheck(uint32_t move, GameBoard const & board);
uint32_t getCompleteMove(GameBoard const & board, uint32_t input_move);



#endif //MOVEGENERATION_H
