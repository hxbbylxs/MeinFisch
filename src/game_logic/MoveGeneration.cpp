//
// Created by salom on 25.06.2025.
//
#include <cstdint>
#include <vector>
using std::vector;

#include "MoveGeneration.h"
#include "MoveGenerationConstants.h"




vector<Move> getPseudoLegalMoves(GameBoard const & board, bool forWhite, MoveType type) {
    vector<Move> pseudoLegalMoves;
    pseudoLegalMoves.reserve(50);

    uint64_t ownPieces = forWhite ? board.white_pieces : board.black_pieces;
    uint64_t enemyPieces = forWhite ? board.black_pieces : board.white_pieces;

    addPseudoLegalPawnMoves(board, forWhite, pseudoLegalMoves, ownPieces, enemyPieces, type);

    addPseudoLegalGenericPieceTypeMoves(board, forWhite, pseudoLegalMoves, ownPieces, enemyPieces,forWhite?Constants::WHITE_BISHOP:Constants::BLACK_BISHOP,getBishopAttackBits,type);
    addPseudoLegalGenericPieceTypeMoves(board, forWhite, pseudoLegalMoves, ownPieces, enemyPieces,forWhite?Constants::WHITE_ROOK:Constants::BLACK_ROOK,getRookAttackBits,type);
    addPseudoLegalGenericPieceTypeMoves(board, forWhite, pseudoLegalMoves, ownPieces, enemyPieces,forWhite?Constants::WHITE_QUEEN:Constants::BLACK_QUEEN,getQueenAttackBits,type);

    addPseudoLegalGenericPieceTypeMoves(board, forWhite, pseudoLegalMoves, ownPieces, enemyPieces,forWhite?Constants::WHITE_KNIGHT:Constants::BLACK_KNIGHT,getKnightAttackBits,type);
    addPseudoLegalGenericPieceTypeMoves(board, forWhite, pseudoLegalMoves, ownPieces, enemyPieces,forWhite?Constants::WHITE_KING:Constants::BLACK_KING,getKingAttackBits,type);

    if (type != CAPTURES) addPseudoLegalCastleMoves(board,forWhite,pseudoLegalMoves,ownPieces,enemyPieces);

    return pseudoLegalMoves;
}

template <typename F>
void addPseudoLegalGenericPieceTypeMoves( GameBoard const &board,
                                bool forWhite,
                                std::vector<Move> &pseudoLegalMoves,
                                uint64_t ownPieces,
                                uint64_t enemyPieces,
                                Constants::Piece piece,
                                F getAttackBitMask,
                                MoveType type) {

    uint64_t pieceOccupancy = ownPieces | enemyPieces;
    uint64_t piecePositions = board.pieces[piece];
    while (piecePositions != 0) {
        auto from = static_cast<unsigned>(__builtin_ctzll(piecePositions)); // position of current first piece
        uint64_t possibleMoves = getAttackBitMask(from, pieceOccupancy);
        uint64_t captures = possibleMoves & enemyPieces;
        uint64_t quiets = possibleMoves & ~pieceOccupancy;

        if (type == CAPTURES || type == ALL) {
            while (captures != 0) {
                auto to = static_cast<unsigned>(__builtin_ctzll(captures)); // position of current first attack
                pseudoLegalMoves.emplace_back(piece,from,getPieceAt(board, to, !forWhite),to);
                captures &= captures-1; // remove this attack
            }
        }
        if (type == QUIETS || type == ALL) {
            while (quiets != 0) {
                auto to = static_cast<unsigned>(__builtin_ctzll(quiets)); // position of current first attack
                pseudoLegalMoves.emplace_back(piece,from,Constants::Piece::NONE,to);
                quiets &= quiets-1; // remove this attack
            }
        }

        piecePositions &= piecePositions-1; //remove this piece
    }
}


void addPseudoLegalPawnMoves(   GameBoard const &board,
                                bool forWhite,
                                std::vector<Move> &pseudoLegalMoves,
                                uint64_t ownPieces,
                                uint64_t enemyPieces,
                                MoveType type) {

    Constants::Piece piece = forWhite ? Constants::Piece::WHITE_PAWN:Constants::Piece::BLACK_PAWN;

    forEachPiece(piece,board,[&](unsigned position, GameBoard const & game_board) {
        if (type == CAPTURES || type == ALL) {
            addPseudoLegalPawnCaptureMoves(board,forWhite,pseudoLegalMoves,ownPieces,enemyPieces,piece,position);
        }
        if (type == QUIETS || type == ALL) {
            addPseudoLegalPawnPushMoves(board,forWhite,pseudoLegalMoves,ownPieces,enemyPieces,piece,position);
        }
    });

}

void addPseudoLegalPawnCaptureMoves(    GameBoard const &board,
                                        bool forWhite,
                                        std::vector<Move> &pseudoLegalMoves,
                                        uint64_t ownPieces,
                                        uint64_t enemyPieces,
                                        Constants::Piece piece,
                                        unsigned from) {

    uint64_t possibleCaptureMoves = pawnCaptureBitMask[forWhite?0:1][from];
    //enPassant
    if (board.enPassant != -1 && possibleCaptureMoves & (1ULL << board.enPassant)) {
        pseudoLegalMoves.emplace_back(piece,from,forWhite?Constants::BLACK_PAWN:Constants::WHITE_PAWN,board.enPassant);
    }
    possibleCaptureMoves &= enemyPieces;
    while (possibleCaptureMoves != 0) {
        auto to = static_cast<unsigned>(__builtin_ctzll(possibleCaptureMoves)); // position of current attack

        Move move = {piece,from,getPieceAt(board,to,!forWhite),to};

        if (to <= MAX_VALID_PAWN_POSITION && to >= MIN_VALID_PAWN_POSITION) {
            pseudoLegalMoves.push_back(move);
        } else {
            addPseudoLegalPromotionMoves(pseudoLegalMoves, move, forWhite);
        }

        possibleCaptureMoves &= possibleCaptureMoves - 1; //remove this attack
    }
}

void addPseudoLegalPawnPushMoves(   GameBoard const &board,
                                    bool forWhite,
                                    std::vector<Move> &pseudoLegalMoves,
                                    uint64_t ownPieces,
                                    uint64_t enemyPieces,
                                    Constants::Piece piece,
                                    unsigned from) {

uint64_t singlePush = (1ULL << (from + (forWhite ? Constants::Direction::NORTH : Constants::Direction::SOUTH))) & ~(ownPieces | enemyPieces);
    uint64_t doublePush = pawnDoublePushBitMask[forWhite?0:1][from] & ~(ownPieces | enemyPieces);
    if (singlePush != 0) {
        auto to = static_cast<unsigned>(__builtin_ctzll(singlePush));
        Move move =     {piece,from, Constants::NONE, to};

        if (to <= MAX_VALID_PAWN_POSITION && to >= MIN_VALID_PAWN_POSITION) {
            pseudoLegalMoves.push_back(move);
        } else {
            addPseudoLegalPromotionMoves(pseudoLegalMoves, move, forWhite);
        }
        if (doublePush != 0) {
            pseudoLegalMoves.emplace_back(piece, from,Constants::NONE,__builtin_ctzll(doublePush));
        }
    }
}


void addPseudoLegalCastleMoves(GameBoard const &board, bool forWhite, std::vector<Move> &pseudoLegalMoves, uint64_t ownPieces, uint64_t enemyPieces) {

    int kingPosition = forWhite ? 60 : 4;
    Constants::Castle queen_side = forWhite? Constants::Castle::WHITE_QUEEN_SIDE_CASTLE : Constants::Castle::BLACK_QUEEN_SIDE_CASTLE;
    Constants::Castle king_side = forWhite? Constants::Castle::WHITE_KING_SIDE_CASTLE : Constants::Castle::BLACK_KING_SIDE_CASTLE;

    if (board.castleInformation[king_side]
    && !((ownPieces|enemyPieces)&(3ULL << kingPosition+1))) {
        pseudoLegalMoves.emplace_back(precalculated_castle_moves[king_side]);
    }
    if (board.castleInformation[queen_side]
    && !((ownPieces|enemyPieces)&(7ULL << kingPosition-3))) {
        pseudoLegalMoves.emplace_back(precalculated_castle_moves[queen_side]);
    }
}

Constants::Piece getPieceAt(GameBoard const &board, unsigned position, bool pieceIsWhite) {
    unsigned start = pieceIsWhite ? 1 : 2;
    uint64_t mask = 1ULL << position;
    for (unsigned i = start; i < 13; i+= 2) {
        if (board.pieces[i] & mask) {
            return Constants::piece_decoding[i];
        }
    }
    return Constants::Piece::NONE;
}

void addPseudoLegalPromotionMoves(vector<Move> & pseudoLegalMoves, Move move, bool forWhite) {
    if (forWhite) {
        move.set_promotion(Constants::WHITE_QUEEN);
        pseudoLegalMoves.emplace_back(move);
        move.set_promotion(Constants::WHITE_ROOK);
        pseudoLegalMoves.emplace_back(move);
        move.set_promotion(Constants::WHITE_BISHOP);
        pseudoLegalMoves.emplace_back(move);
        move.set_promotion(Constants::WHITE_KNIGHT);
        pseudoLegalMoves.emplace_back(move);

    } else {
        move.set_promotion(Constants::BLACK_QUEEN);
        pseudoLegalMoves.emplace_back(move);
        move.set_promotion(Constants::BLACK_ROOK);
        pseudoLegalMoves.emplace_back(move);
        move.set_promotion(Constants::BLACK_BISHOP);
        pseudoLegalMoves.emplace_back(move);
        move.set_promotion(Constants::BLACK_KNIGHT);
        pseudoLegalMoves.emplace_back(move);
    }
}

bool isLegalMove(Move move, GameBoard & board) {
    if (move.castle()) { //case castle
        if (isCastlingThroughCheck(move,board)) return false;
    }

    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    board.applyPseudoLegalMove(move);
    bool isLegal = !board.isCheck(!board.whiteToMove);
    board.unmakeMove(move,enPassant,castle_rights,plies,hash_before);

    return isLegal;
}

bool isPseudoLegalMove(Move move, GameBoard const &board) {
    uint64_t allPieces = board.white_pieces | board.black_pieces;

    if (board.whiteToMove != move.piece()%2) return false;
    if (!(board.pieces[move.piece()] & (1ULL << move.from()))) return false;
    if (move.capture() && !(board.pieces[move.capture()] & 1ULL << move.to())){
        // captured piece is wrong except it is an en passant capture
        if (move.to() != board.enPassant
            || (move.capture() != Constants::WHITE_PAWN && move.capture() != Constants::BLACK_PAWN)
            || (move.piece() != Constants::WHITE_PAWN && move.piece() != Constants::BLACK_PAWN) )
            return false;
    }
    if (move.castle()) {
        if (!(board.castleInformation[move.castle()])) return false;
        if (move.castle() == Constants::WHITE_KING_SIDE_CASTLE || move.castle() == Constants::BLACK_KING_SIDE_CASTLE) {
            if (allPieces & 3ULL << move.from()+1) return false;
        } else {
            if (allPieces & 7ULL << move.from()-3) return false;
        }
        return true;
    }

    uint64_t ownPieces = board.whiteToMove ? board.white_pieces : board.black_pieces;
    uint64_t enemyPieces = board.whiteToMove ? board.black_pieces : board.white_pieces;


    if (move.piece() == Constants::WHITE_PAWN || move.piece() == Constants::BLACK_PAWN) {
        uint64_t pseudoLegalSquares = pawnCaptureBitMask[!board.whiteToMove][move.from()] & (enemyPieces | (board.enPassant != -1 ? 1ULL << board.enPassant : 0));
        if (!move.capture()) pseudoLegalSquares = 0;
        unsigned singlePushSquare = (move.from() + (board.whiteToMove ? Constants::NORTH : Constants::SOUTH));
        if (1ULL << singlePushSquare & ~allPieces) {
            pseudoLegalSquares |= 1ULL << singlePushSquare;
            if (pawnDoublePushBitMask[!board.whiteToMove][move.from()] & ~allPieces) {
                pseudoLegalSquares |= pawnDoublePushBitMask[!board.whiteToMove][move.from()];
            }
        }
        if (!((1ULL << move.to()) & pseudoLegalSquares)) return false;
    } else if (move.piece() == Constants::WHITE_KNIGHT || move.piece() == Constants::BLACK_KNIGHT) {
        uint64_t pseudoLegalSquares = knightAttackBitMasks[move.from()] & ~ownPieces;
        if (!move.capture()) pseudoLegalSquares &= ~enemyPieces;
        if (!((1ULL << move.to()) & pseudoLegalSquares)) return false;
    } else if (move.piece() == Constants::WHITE_BISHOP || move.piece() == Constants::BLACK_BISHOP) {
        uint64_t pseudoLegalSquares = getBishopAttackBits(move.from(),allPieces) & ~ownPieces;
        if (!move.capture()) pseudoLegalSquares &= ~enemyPieces;
        if (!((1ULL << move.to()) & pseudoLegalSquares)) return false;
    } else if (move.piece() == Constants::WHITE_ROOK || move.piece() == Constants::BLACK_ROOK) {
        uint64_t pseudoLegalSquares = getRookAttackBits(move.from(),allPieces) & ~ownPieces;
        if (!move.capture()) pseudoLegalSquares &= ~enemyPieces;
        if (!((1ULL << move.to()) & pseudoLegalSquares)) return false;
    } else if (move.piece() == Constants::WHITE_QUEEN || move.piece() == Constants::BLACK_QUEEN) {
        uint64_t pseudoLegalSquares = getQueenAttackBits(move.from(),allPieces) & ~ownPieces;
        if (!move.capture()) pseudoLegalSquares &= ~enemyPieces;
        if (!((1ULL << move.to()) & pseudoLegalSquares)) return false;
    } else {
        uint64_t pseudoLegalSquares = getKingAttackBits(move.from(),allPieces) & ~ownPieces;
        if (!move.capture()) pseudoLegalSquares &= ~enemyPieces;
        if (!((1ULL << move.to()) & pseudoLegalSquares)) return false;
    }
    return true;
}


bool isSquareAttacked(unsigned square, bool attacker_is_white, GameBoard const &board) {
    uint64_t allPieces = board.white_pieces | board.black_pieces;

    //Is square attacked by knight
    uint64_t possible_knight_positions = knightAttackBitMasks[square];
    if (possible_knight_positions & board.pieces[attacker_is_white?Constants::Piece::WHITE_KNIGHT:Constants::Piece::BLACK_KNIGHT]) {
        return true;
    }
    //Is square attacked by king
    uint64_t possible_other_king_positions = kingAttackBitMasks[square];
    if (possible_other_king_positions & board.pieces[attacker_is_white?Constants::Piece::WHITE_KING:Constants::Piece::BLACK_KING]) {
        return true;
    }
    //Is square attacked by rook or queen (non diagonal)
    uint64_t possible_slider_positions = getRookAttackBits(square,allPieces);
    if (possible_slider_positions & (board.pieces[attacker_is_white?Constants::Piece::WHITE_ROOK:Constants::Piece::BLACK_ROOK] | board.pieces[attacker_is_white?Constants::Piece::WHITE_QUEEN:Constants::Piece::BLACK_QUEEN])) {
        return true;
    }
    //Is square attacked by bishop or queen (diagonal)
    possible_slider_positions = getBishopAttackBits(square,allPieces);
    if (possible_slider_positions & (board.pieces[attacker_is_white?Constants::Piece::WHITE_BISHOP:Constants::Piece::BLACK_BISHOP] | board.pieces[attacker_is_white?Constants::Piece::WHITE_QUEEN:Constants::Piece::BLACK_QUEEN])) {
        return true;
    }
    //Is square attacked by pawns
    uint64_t possible_pawn_positions = pawnCaptureBitMask[attacker_is_white][square];
    if (possible_pawn_positions & (board.pieces[attacker_is_white?Constants::Piece::WHITE_PAWN:Constants::Piece::BLACK_PAWN])) {
        return true;
    }
    return false;
}


bool isCastlingThroughCheck(Move move, GameBoard const & board) {
    uint32_t square_between = (move.from() + move.to()) /2;
    return isSquareAttacked(square_between,!board.whiteToMove, board) | isSquareAttacked(move.from(), !board.whiteToMove, board);
}