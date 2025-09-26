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
std::vector<Move> getPseudoLegalMoves(GameBoard const & board, bool forWhite, MoveType type);
[[nodiscard]] std::vector<Move> getPseudoLegalAdvancedPawnPushes(GameBoard const & board, bool forWhite);


void addPseudoLegalPawnMoves(GameBoard const & board, bool forWhite, std::vector<Move> & pseudoLegalMoves, uint64_t ownPieces, uint64_t enemyPieces, MoveType type);
void addPseudoLegalPawnCaptureMoves(GameBoard const &board, bool forWhite, std::vector<Move> &pseudoLegalMoves, uint64_t ownPieces, uint64_t enemyPieces, Constants::Piece piece, unsigned from);
void addPseudoLegalPawnPushMoves(GameBoard const &board, bool forWhite, std::vector<Move> &pseudoLegalMoves, uint64_t ownPieces, uint64_t enemyPieces, Constants::Piece piece, unsigned from);

void addPseudoLegalPromotionMoves(std::vector<Move> & pseudoLegalMoves, Move move, bool forWhite);
void addPseudoLegalCastleMoves(GameBoard const &board, bool forWhite, std::vector<Move> &pseudoLegalMoves, uint64_t ownPieces, uint64_t enemyPieces);

template <typename F>
void addPseudoLegalGenericPieceTypeMoves( GameBoard const &board,
                                bool forWhite,
                                std::vector<Move> &pseudoLegalMoves,
                                uint64_t ownPieces,
                                uint64_t enemyPieces,
                                Constants::Piece piece,
                                F getAttackBitMask,
                                MoveType type);

Constants::Piece getPieceAt(GameBoard const & board, unsigned position, bool pieceIsWhite);

bool isLegalMove(Move move, GameBoard & board);
bool isPseudoLegalMove(Move move, GameBoard const & board);
bool isSquareAttacked(unsigned square, bool attacker_is_white, GameBoard const & board);
bool isCastlingThroughCheck(Move move, GameBoard const & board);



#endif //MOVEGENERATION_H
