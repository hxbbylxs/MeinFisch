//
// Created by salom on 24.06.2025.
//
#include <array>
#include <thread>

#include "Utils.h"
#include "../io/Output.h"

using std::array;
#include <vector>
using std::vector;
#include <algorithm>
#include <cassert>

#include "Conversions.h"
#include "GameBoard.h"
#include "MoveGeneration.h"

#include "MoveGenerationConstants.h" // lib constants

#include "Utils.h" // lib utils

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
    // construct white_pieces and black_pieces from pieces
    this->white_pieces = pieces[Constants::WHITE_KING] | pieces[Constants::WHITE_QUEEN]
                        | pieces[Constants::WHITE_ROOK] | pieces[Constants::WHITE_BISHOP]
                        | pieces[Constants::WHITE_KNIGHT] | pieces[Constants::WHITE_PAWN];
    this->black_pieces = pieces[Constants::BLACK_KING] | pieces[Constants::BLACK_QUEEN]
                        | pieces[Constants::BLACK_ROOK] | pieces[Constants::BLACK_BISHOP]
                        | pieces[Constants::BLACK_KNIGHT] | pieces[Constants::BLACK_PAWN];

    //initialize zobrist hash
    for (int i = 1; i < 13; i++) {
        uint64_t piece = pieces[i];
        while (piece != 0) {
            int pos_of_piece = counttzll(piece);
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
    this->white_pieces = pieces[Constants::WHITE_KING] | pieces[Constants::WHITE_QUEEN]
                        | pieces[Constants::WHITE_ROOK] | pieces[Constants::WHITE_BISHOP]
                        | pieces[Constants::WHITE_KNIGHT] | pieces[Constants::WHITE_PAWN];
    this->black_pieces = pieces[Constants::BLACK_KING] | pieces[Constants::BLACK_QUEEN]
                        | pieces[Constants::BLACK_ROOK] | pieces[Constants::BLACK_BISHOP]
                        | pieces[Constants::BLACK_KNIGHT] | pieces[Constants::BLACK_PAWN];

    // initialiue zobrist hash
        for (int i = 1; i < 13; i++) {
            uint64_t piece = pieces[i];
            while (piece != 0) {
                int pos_of_piece = counttzll(piece);
                zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[pos_of_piece][i];
                piece &= piece -1;
            }
        }
        for (int i = 0; i < 5; i++) {
            zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[i];
        }
}



bool GameBoard::isCheck(bool whiteKing) const {
    int king_position = counttzll(pieces[whiteKing?Constants::Piece::WHITE_KING:Constants::Piece::BLACK_KING]);
    return isSquareAttacked(king_position,!whiteKing,*this);
}

void GameBoard::applyPseudoLegalMove(Move move) {

    assert(isPseudoLegalMove(move,*this));

    if (!whiteToMove) moves++;
    plies++;

    // remove piece from its square
    pieces[move.piece()] &= ~(1ULL << move.from());
    zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[move.from()][move.piece()];
    if (whiteToMove) white_pieces &= ~(1ULL << move.from());
    else black_pieces &= ~(1ULL << move.from());

    // put piece on new square
    pieces[move.piece()] |= (1ULL << move.to());
    zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[move.to()][move.piece()];
    if (whiteToMove) white_pieces |= (1ULL << move.to());
    else black_pieces |= (1ULL << move.to());

    // remove captured piece (en passant is handled later, zobrist hash change has to be unmade then)
    pieces[move.capture()] &= ~(1ULL << move.to());
    zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[move.to()][move.capture()];
    if (whiteToMove) black_pieces &= ~(1ULL << move.to());
    else white_pieces &= ~(1ULL << move.to());

    // adjust captured piece in en passant case
    if (move.to() == enPassant && move.capture() != Constants::Piece::NONE) {
        // unmake previous changes because captured pawn is not standing on mv.to
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[move.to()][move.capture()];
        int offset = whiteToMove ? Constants::SOUTH : Constants::NORTH;
        //remove pawn captured pawn from the square above/below
        pieces[move.capture()] &= ~(1ULL << move.to()+offset);
        if (whiteToMove) black_pieces &= ~(1ULL << move.to()+offset);
        else white_pieces &= ~(1ULL << move.to()+offset);

        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[move.to()+offset][move.capture()];
    }

    // reset old en passant
    if (enPassant != -1) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_ENPASSANT[enPassant%8];
    enPassant = -1;

    // promotions and setting new en passant square
    if (move.piece() == Constants::WHITE_PAWN || move.piece() == Constants::BLACK_PAWN) {
        plies = 0;
        handlePawnSpecialCases(move);
    }
    if (move.castle()) {
        moveRookForCastle(move.castle(),false);
    }
    updateCastleInformation(move);

    if (move.capture() != Constants::Piece::NONE) plies = 0;

    whiteToMove = !whiteToMove;
    zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[0]; // switch player to move

    // to detect threefold repetition
    addNewBoardPosition(zobristHash);
}

void GameBoard::handlePawnSpecialCases(Move move) {
    if (move.promotion() != Constants::Piece::NONE) {
        // remove pawn from the square it has previously been set to
        pieces[move.piece()] &= ~(1ULL << move.to());
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[move.to()][move.piece()];

        // put the new piece on that square
        pieces[move.promotion()] |= (1ULL << move.to());
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[move.to()][move.promotion()];
    }
    if (move.from() == move.to() +16 || move.from() == move.to() -16) {
        // in case of double push, set enPassant to the square in between
        enPassant =  static_cast<int>((move.from() + move.to())/2);
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_ENPASSANT[enPassant%8];
    }
}

void GameBoard::moveRookForCastle(unsigned castle, bool unmake) {

    if (castle == Constants::Castle::WHITE_KING_SIDE_CASTLE || castle == Constants::Castle::WHITE_QUEEN_SIDE_CASTLE) {
        //case white castles
        pieces[Constants::Piece::WHITE_ROOK] &= ~(castle_rook_positions[castle][unmake]);
        pieces[Constants::Piece::WHITE_ROOK] |= (castle_rook_positions[castle][!unmake]);
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[counttzll(castle_rook_positions[castle][unmake])][Constants::Piece::WHITE_ROOK];
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[counttzll(castle_rook_positions[castle][!unmake])][Constants::Piece::WHITE_ROOK];
        white_pieces &= ~(castle_rook_positions[castle][unmake]);
        white_pieces|= (castle_rook_positions[castle][!unmake]);

    } else {
        //case black castles
        pieces[Constants::Piece::BLACK_ROOK] &= ~(castle_rook_positions[castle][unmake]);
        pieces[Constants::Piece::BLACK_ROOK] |= (castle_rook_positions[castle][!unmake]);
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[counttzll(castle_rook_positions[castle][unmake])][Constants::Piece::BLACK_ROOK];
        zobristHash ^= Constants::ZOBRIST_HASH_VALUES_PIECES[counttzll(castle_rook_positions[castle][!unmake])][Constants::Piece::BLACK_ROOK];
        black_pieces &= ~(castle_rook_positions[castle][unmake]);
        black_pieces |= (castle_rook_positions[castle][!unmake]);
    }
}

void GameBoard::updateCastleInformation(Move move) {
    if (move.piece() == Constants::WHITE_KING) {
        if (castleInformation[Constants::Castle::WHITE_KING_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::WHITE_KING_SIDE_CASTLE];
        castleInformation[Constants::Castle::WHITE_KING_SIDE_CASTLE] = false;
        if (castleInformation[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE];
        castleInformation[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE] = false;
    }
    if (move.piece() == Constants::BLACK_KING) {
        if (castleInformation[Constants::Castle::BLACK_KING_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::BLACK_KING_SIDE_CASTLE];
        castleInformation[Constants::Castle::BLACK_KING_SIDE_CASTLE] = false;
        if (castleInformation[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE];
        castleInformation[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE] = false;
    }
    // TODO replace magic numbers
    if (move.to() == 0 || move.from() == 0) {
        if (castleInformation[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE];
        castleInformation[Constants::Castle::BLACK_QUEEN_SIDE_CASTLE] = false;
    }
    if (move.to() == 7 || move.from() == 7) {
        if (castleInformation[Constants::Castle::BLACK_KING_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::BLACK_KING_SIDE_CASTLE];
        castleInformation[Constants::Castle::BLACK_KING_SIDE_CASTLE] = false;
    }
    if (move.to() == 56 || move.from() == 56) {
        if (castleInformation[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE];
        castleInformation[Constants::Castle::WHITE_QUEEN_SIDE_CASTLE] = false;
    }
    if (move.to() == 63 || move.from() == 63) {
        if (castleInformation[Constants::Castle::WHITE_KING_SIDE_CASTLE]) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[Constants::Castle::WHITE_KING_SIDE_CASTLE];
        castleInformation[Constants::Castle::WHITE_KING_SIDE_CASTLE] = false;
    }
}

void GameBoard::unmakeMove(Move move, int enPassant_, std::array<bool, 5> castleRights, uint8_t plies_before, uint64_t hash_before) {

    removeBoardPosition(zobristHash); // removes board position from the saved positions for threefold rep

    castleInformation = castleRights;
    plies = plies_before;
    if (whiteToMove) moves--;

    // remove piece from the target square
    pieces[move.piece()] &= ~(1ULL << move.to());
    if (move.promotion() != Constants::Piece::NONE) {
        pieces[move.promotion()] &= ~(1ULL << move.to());
    }
    if (whiteToMove) black_pieces &= ~(1ULL << move.to());
    else white_pieces &= ~(1ULL << move.to());

    // add piece to the original square
    pieces[move.piece()] |= (1ULL << move.from());
    if (whiteToMove) black_pieces |= (1ULL << move.from());
    else white_pieces |= (1ULL << move.from());

    // put captured piece back on the target square
    if (move.capture() != Constants::Piece::NONE) {
        if (enPassant_ == move.to()) {
            int offset = whiteToMove ? Constants::NORTH : Constants::SOUTH;
            pieces[move.capture()] |= (1ULL << move.to() + offset);
            if (whiteToMove) white_pieces |= (1ULL << move.to() + offset);
            else black_pieces |= (1ULL << move.to() + offset);
        } else {
            pieces[move.capture()] |= (1ULL << move.to());
            if (whiteToMove) white_pieces |= (1ULL << move.to());
            else black_pieces |= (1ULL << move.to());
        }
    }
    if (move.castle()) moveRookForCastle (move.castle(),true);

    enPassant = enPassant_;
    whiteToMove = !whiteToMove;

    zobristHash = hash_before;
}

void GameBoard::makeNullMove() {
    if (!whiteToMove) moves++;
    plies++;

    whiteToMove = !whiteToMove;
    zobristHash ^= Constants::ZOBRIST_HASH_VALUES_OTHER[0]; // switch player to move

    // reset old en passant
    if (enPassant != -1) zobristHash ^= Constants::ZOBRIST_HASH_VALUES_ENPASSANT[enPassant%8];
    enPassant = -1;

    // to detect threefold repetition
    addNewBoardPosition(zobristHash);
}
void GameBoard::unmakeNullMove(int en_passant, uint64_t zobrist_hash) {
    removeBoardPosition(zobristHash);
    if (whiteToMove) moves--;
    plies--;
    whiteToMove = !whiteToMove;
    enPassant = en_passant;
    zobristHash = zobrist_hash;
}


bool GameBoard::noLegalMoves() const {
    vector<Move> pseudoLegalMoves = getPseudoLegalMoves(*this, whiteToMove,ALL);
    auto copy = GameBoard(*this);
    return !std::any_of(pseudoLegalMoves.begin(), pseudoLegalMoves.end(),[copy](Move move) mutable {
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
    return enPassant == other.enPassant && white_pieces == other.white_pieces && black_pieces == other.black_pieces && zobristHash == other.zobristHash;
}

// board positions for move repetition check

void GameBoard::addNewBoardPosition(uint64_t hash) {
    int& count = board_positions[hash];
    count++;
}

void GameBoard::removeBoardPosition(uint64_t hash) {
    auto it = board_positions.find(hash);
    if (it == board_positions.end()) {
        assert(false);
    }
    if (it->second == 1) {
        board_positions.erase(it);
    } else {
        it->second--;
    }
}

