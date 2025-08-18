//
// Created by salom on 24.06.2025.
//
#include <array>

using std::array;
#include <vector>
using std::vector;
#include <algorithm>

#include "Conversions.h"
#include "GameBoard.h"
#include "MoveGeneration.h"
#include "MoveGenerationConstants.h"

GameBoard::GameBoard(
    array<uint64_t, 13> const & pieces,
    bool whiteToMove,
    array<bool, 5> const & castleInformation,
    int enPassant,
    uint8_t plies,
    uint8_t moves) :
    pieces(pieces),
    whiteToMove(whiteToMove),
    castleInformation(castleInformation),
    enPassant(enPassant),
    plies(plies), moves(moves),
    zobristHash(0)
{
    this->allPieces = 0;
    for (uint64_t piece : pieces) {
        this->allPieces |= piece;
    }
    for (int i = 1; i < 13; i++) {
        uint64_t piece = pieces[i];
        while (piece != 0) {
            int pos_of_piece = __builtin_ctzll(piece);
            zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[pos_of_piece][i];
            piece &= piece -1;
        }
    }
    if (enPassant != -1) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_ENPASSANT[enPassant%8];
    if (whiteToMove) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[0];
    for (int i = 1; i < 5; i++) {
        if (castleInformation[i]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[i];
    }
}

GameBoard::GameBoard() :
    pieces(convertStringToPieceInformation(Constants::STANDARD_FEN_PIECES)),
    whiteToMove(true),
    castleInformation({true,true,true,true,true}),
    plies(0), moves(1),
    zobristHash(0)
    {
        this->allPieces = 0;
        for (uint64_t piece : pieces) {
            this->allPieces |= piece;
        }
        for (int i = 1; i < 13; i++) {
            uint64_t piece = pieces[i];
            while (piece != 0) {
                int pos_of_piece = __builtin_ctzll(piece);
                zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[pos_of_piece][i];
                piece &= piece -1;
            }
        }
        for (int i = 0; i < 5; i++) {
            zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[i];
        }
}



bool GameBoard::isCheck(bool whiteKing) const {
    int king_position = __builtin_ctzll(pieces[whiteKing?Constants::Piece::WHITE_KING:Constants::Piece::BLACK_KING]);
    return isSquareAttacked(king_position,!whiteKing,*this);
}

void GameBoard::applyPseudoLegalMove(uint32_t move) {
    if (!whiteToMove) moves++;
    plies++;
    Move mv = decodeMove(move);

    pieces[mv.piece] &= ~(1ULL << mv.from);
    zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[mv.from][mv.piece];
    pieces[mv.piece] |= (1ULL << mv.to);
    zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[mv.to][mv.piece];
    pieces[mv.captured_piece] &= ~(1ULL << mv.to);
    zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[mv.to][mv.captured_piece];

    allPieces &= ~(1ULL << mv.from);
    allPieces |= (1ULL << mv.to);

    if (mv.to == enPassant && mv.captured_piece != Constants::Piece::NONE) {
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[mv.to][mv.captured_piece];
        int offset = whiteToMove ? 8 : -8;
        pieces[mv.captured_piece] &= ~(1ULL << mv.to+offset);
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[mv.to+offset][mv.captured_piece];
        allPieces &= ~(1ULL << mv.to+offset);
    }
    if (enPassant != -1) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_ENPASSANT[enPassant%8];
    enPassant = -1;

    if (mv.piece == Constants::WHITE_PAWN || mv.piece == Constants::BLACK_PAWN) {
        plies = 0;
        handlePawnSpecialCases(mv);
    }
    if (mv.castle) {
        moveRookForCastle(mv.castle,false);
    }
    updateCastleInformation(mv);

    if (mv.captured_piece != Constants::Piece::NONE) plies = 0;

    whiteToMove = !whiteToMove;
    zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[0];

    addNewBoardPosition(zobristHash);
}

void GameBoard::handlePawnSpecialCases(Move const & mv) {
    if (mv.pawn_promote_to != Constants::Piece::NONE) {
        pieces[mv.piece] &= ~(1ULL << mv.to);
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[mv.to][mv.piece];
        pieces[mv.pawn_promote_to] |= (1ULL << mv.to);
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[mv.to][mv.pawn_promote_to];
    }
    if (mv.from == mv.to +16 || mv.from == mv.to -16) {
        enPassant =  (mv.from+mv.to)/2;
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_ENPASSANT[enPassant%8];
    }
}

void GameBoard::moveRookForCastle(Constants::Castle castle, bool unmake) {
    allPieces &= ~(castle_rook_positions[castle][unmake]);
    allPieces |= (castle_rook_positions[castle][!unmake]);
    if (castle == Constants::Castle::WHITE_KING_SIDE_CASTLE || castle == Constants::Castle::WHITE_QUEEN_SIDE_CASTLE) {
        pieces[Constants::Piece::WHITE_ROOK] &= ~(castle_rook_positions[castle][unmake]);
        pieces[Constants::Piece::WHITE_ROOK] |= (castle_rook_positions[castle][!unmake]);
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[__builtin_ctzll(castle_rook_positions[castle][unmake])][Constants::Piece::WHITE_ROOK];
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[__builtin_ctzll(castle_rook_positions[castle][!unmake])][Constants::Piece::WHITE_ROOK];

    } else {
        pieces[Constants::Piece::BLACK_ROOK] &= ~(castle_rook_positions[castle][unmake]);
        pieces[Constants::Piece::BLACK_ROOK] |= (castle_rook_positions[castle][!unmake]);
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[__builtin_ctzll(castle_rook_positions[castle][unmake])][Constants::Piece::BLACK_ROOK];
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[__builtin_ctzll(castle_rook_positions[castle][!unmake])][Constants::Piece::BLACK_ROOK];
    }
}

void GameBoard::updateCastleInformation(Move const & mv) {
    if (mv.piece == Constants::WHITE_KING) {
        if (castleInformation[Constants::Castle::WHITE_KING_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::WHITE_KING_SIDE_CASTLE];
        castleInformation[Constants::Castle::WHITE_KING_SIDE_CASTLE] = false;
        if (castleInformation[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE];
        castleInformation[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE] = false;
    }
    if (mv.piece == Constants::BLACK_KING) {
        if (castleInformation[Constants::Castle::BLACK_KING_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::BLACK_KING_SIDE_CASTLE];
        castleInformation[Constants::Castle::BLACK_KING_SIDE_CASTLE] = false;
        if (castleInformation[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE];
        castleInformation[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE] = false;
    }
    if (mv.to == 0 || mv.from == 0) {
        if (castleInformation[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE];
        castleInformation[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE] = false;
    }
    if (mv.to == 7 || mv.from == 7) {
        if (castleInformation[Constants::Castle::BLACK_KING_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::BLACK_KING_SIDE_CASTLE];
        castleInformation[Constants::Castle::BLACK_KING_SIDE_CASTLE] = false;
    }
    if (mv.to == 56 || mv.from == 56) {
        if (castleInformation[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE];
        castleInformation[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE] = false;
    }
    if (mv.to == 63 || mv.from == 63) {
        if (castleInformation[Constants::Castle::WHITE_KING_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::WHITE_KING_SIDE_CASTLE];
        castleInformation[Constants::Castle::WHITE_KING_SIDE_CASTLE] = false;
    }
}

void GameBoard::unmakeMove(uint32_t move, int enPassant_, std::array<bool, 5> castleRights, uint8_t plies_before, uint64_t hash_before) {

    removeBoardPosition(zobristHash); // removes board position from the saved positions

    Move mv = decodeMove(move);

    castleInformation = castleRights;
    plies = plies_before;
    if (whiteToMove) moves--;

    pieces[mv.piece] &= ~(1ULL << mv.to);
    if (mv.pawn_promote_to != Constants::Piece::NONE) {
        pieces[mv.pawn_promote_to] &= ~(1ULL << mv.to);
    }
    allPieces &= ~(1ULL << mv.to);
    pieces[mv.piece] |= (1ULL << mv.from);
    allPieces |= (1ULL << mv.from);
    if (mv.captured_piece != Constants::Piece::NONE) {
        if (enPassant_ == mv.to) {
            int offset = whiteToMove ? -8 : 8;
            pieces[mv.captured_piece] |= (1ULL << mv.to+offset);
            allPieces |= (1ULL << mv.to+offset);
        } else {
            pieces[mv.captured_piece] |= (1ULL << mv.to);
            allPieces |= (1ULL << mv.to);
        }
    }
    if (mv.castle) moveRookForCastle (mv.castle,true);

    enPassant = enPassant_;
    whiteToMove = !whiteToMove;

    zobristHash = hash_before;
}


bool GameBoard::noLegalMoves() const {
    vector<uint32_t> pseudoLegalMoves = getPseudoLegalMoves(*this, whiteToMove,ALL);
    auto copy = GameBoard(*this);
    return !std::any_of(pseudoLegalMoves.begin(), pseudoLegalMoves.end(),[copy](uint32_t move) mutable {
        return isLegalMove(move,copy);
    });
}

bool GameBoard::operator==(GameBoard const & other) const {
    if (whiteToMove != other.whiteToMove) return false;
    for (int i = 1; i < 13; i++) {
        if (pieces[i] != other.pieces[i]) return false;
    }
    for (int i = 1; i < 5; i++) {
        if (castleInformation[i] != other.castleInformation[i]) return false;
    }
    return enPassant == other.enPassant && allPieces == other.allPieces && zobristHash == other.zobristHash;
}

// board positions for move repetition check

// returns true if the position is the third repetition
void GameBoard::addNewBoardPosition(uint64_t hash) {
    int& count = board_positions[hash];
    count++;
}

void GameBoard::removeBoardPosition(uint64_t hash) {
    auto it = board_positions.find(hash);
    if (it == board_positions.end()) {
        // error
        return;
    }
    if (it->second == 1) {
        board_positions.erase(it);
    } else {
        it->second--;
    }
}

