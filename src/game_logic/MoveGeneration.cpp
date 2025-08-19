//
// Created by salom on 25.06.2025.
//

#include "MoveGeneration.h"
#include "MoveGenerationConstants.h"

#include <cstdint>
#include <vector>
using std::vector;
#include <stdexcept>


vector<uint32_t> getPseudoLegalMoves(GameBoard const & board, bool forWhite, MoveType type) {
    vector<uint32_t> pseudoLegalMoves;
    pseudoLegalMoves.reserve(50);

    uint64_t ownPieces = forWhite ? board.pieces[Constants::WHITE_KING] | board.pieces[Constants::WHITE_QUEEN]
                                            | board.pieces[Constants::WHITE_ROOK] | board.pieces[Constants::WHITE_BISHOP]
                                            | board.pieces[Constants::WHITE_KNIGHT] | board.pieces[Constants::WHITE_PAWN]
                                            : board.pieces[Constants::BLACK_KING] | board.pieces[Constants::BLACK_QUEEN]
                                            | board.pieces[Constants::BLACK_ROOK] | board.pieces[Constants::BLACK_BISHOP]
                                            | board.pieces[Constants::BLACK_KNIGHT] | board.pieces[Constants::BLACK_PAWN];
    uint64_t enemyPieces = board.allPieces & ~ownPieces;

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
                                std::vector<uint32_t> &pseudoLegalMoves,
                                uint64_t ownPieces,
                                uint64_t enemyPieces,
                                Constants::Piece piece,
                                F getAttackBitMask,
                                MoveType type) {

    uint64_t pieceOccupancy = ownPieces | enemyPieces;
    uint64_t piecePositions = board.pieces[piece];
    while (piecePositions != 0) {
        int from = __builtin_ctzll(piecePositions); // position of current first piece
        uint64_t possibleMoves = getAttackBitMask(from, pieceOccupancy);
        uint64_t captures = possibleMoves & enemyPieces;
        uint64_t quiets = possibleMoves & ~pieceOccupancy;

        if (type == CAPTURES || type == ALL) {
            while (captures != 0) {
                int to = __builtin_ctzll(captures); // position of current first attack
                uint32_t move = piece;
                move |= from << 4;
                move |= to << 14;
                move |= getPieceAt(board, to, !forWhite) << 10;
                pseudoLegalMoves.push_back(move);

                captures &= captures-1; // remove this attack
            }
        }
        if (type == QUIETS || type == ALL) {
            while (quiets != 0) {
                int to = __builtin_ctzll(quiets); // position of current first attack
                uint32_t move = piece;
                move |= from << 4;
                move |= to << 14;
                pseudoLegalMoves.push_back(move);

                quiets &= quiets-1; // remove this attack
            }
        }

        piecePositions &= piecePositions-1; //remove this piece
    }
}


void addPseudoLegalPawnMoves(   GameBoard const &board,
                                bool forWhite,
                                std::vector<uint32_t> &pseudoLegalMoves,
                                uint64_t ownPieces,
                                uint64_t enemyPieces,
                                MoveType type) {

    Constants::Piece piece = forWhite ? Constants::Piece::WHITE_PAWN:Constants::Piece::BLACK_PAWN;

    forEachPiece(piece,board,[&](int position, GameBoard const & game_board) {
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
                                        std::vector<uint32_t> &pseudoLegalMoves,
                                        uint64_t ownPieces,
                                        uint64_t enemyPieces,
                                        Constants::Piece piece,
                                        int from) {

    uint64_t possibleCaptureMoves = pawnCaptureBitMask[forWhite?0:1][from];
    if (board.enPassant != -1 && possibleCaptureMoves & (1ULL << board.enPassant)) {
        pseudoLegalMoves.push_back(     piece |
                                        ((forWhite? Constants::Piece::BLACK_PAWN : Constants::Piece::WHITE_PAWN) << 10) |
                                        from << 4 |
                                        board.enPassant << 14);
    }
    possibleCaptureMoves &= enemyPieces;
    while (possibleCaptureMoves != 0) {
        int to = __builtin_ctzll(possibleCaptureMoves); // position of current attack
        uint64_t move =     piece |
                            getPieceAt(board,to,!forWhite) << 10 |
                            from << 4 |
                            to << 14;

        if (to < 56 && to >= 8) {
            pseudoLegalMoves.push_back(move);
        } else {
            addPseudoLegalPromotionMoves(pseudoLegalMoves, move, forWhite);
        }

        possibleCaptureMoves &= possibleCaptureMoves - 1; //remove this attack
    }
}

void addPseudoLegalPawnPushMoves(   GameBoard const &board,
                                    bool forWhite,
                                    std::vector<uint32_t> &pseudoLegalMoves,
                                    uint64_t ownPieces,
                                    uint64_t enemyPieces,
                                    Constants::Piece piece,
                                    int from) {

    uint64_t singlePush = (1ULL << (from + (forWhite?-8:8))) & ~(ownPieces | enemyPieces);
    uint64_t doublePush = pawnDoublePushBitMask[forWhite?0:1][from] & ~(ownPieces | enemyPieces);
    if (singlePush != 0) {
        int to = __builtin_ctzll(singlePush);
        uint32_t move =     piece |
                            from << 4 |
                            to << 14;

        if (to < 56 && to >= 8) {
            pseudoLegalMoves.push_back(move);
        } else {
            addPseudoLegalPromotionMoves(pseudoLegalMoves, move, forWhite);
        }
        if (doublePush != 0) {
            pseudoLegalMoves.push_back(piece |
                                        from << 4 |
                                        __builtin_ctzll(doublePush) << 14);
        }
    }
}


void addPseudoLegalCastleMoves(GameBoard const &board, bool forWhite, std::vector<uint32_t> &pseudoLegalMoves, uint64_t ownPieces, uint64_t enemyPieces) {

    int kingPosition = forWhite ? 60 : 4;
    Constants::Castle queen_side = forWhite? Constants::Castle::WHITE_QUEEN_SIDE_CASTLE : Constants::Castle::BLACK_QUEEN_SIDE_CASTLE;
    Constants::Castle king_side = forWhite? Constants::Castle::WHITE_KING_SIDE_CASTLE : Constants::Castle::BLACK_KING_SIDE_CASTLE;

    if (board.castleInformation[king_side]
    && !((ownPieces|enemyPieces)&(3ULL << kingPosition+1))) {
        pseudoLegalMoves.push_back(precalculated_castle_moves[king_side]);
    }
    if (board.castleInformation[queen_side]
    && !((ownPieces|enemyPieces)&(7ULL << kingPosition-3))) {
        pseudoLegalMoves.push_back(precalculated_castle_moves[queen_side]);
    }
}

Constants::Piece getPieceAt(GameBoard const &board, int position, bool pieceIsWhite) {
    unsigned start = pieceIsWhite ? 1 : 2;
    uint64_t mask = 1ULL << position;
    for (unsigned i = start; i < 13; i+= 2) {
        if (board.pieces[i] & mask) {
            return Constants::piece_decoding[i];
        }
    }
    return Constants::Piece::NONE;
}

void addPseudoLegalPromotionMoves(vector<uint32_t> & pseudoLegalMoves, uint32_t move, bool forWhite) {
    if (forWhite) {
        pseudoLegalMoves.push_back(move | Constants::Piece::WHITE_QUEEN << 20);
        pseudoLegalMoves.push_back(move | Constants::Piece::WHITE_ROOK << 20);
        pseudoLegalMoves.push_back(move | Constants::Piece::WHITE_BISHOP << 20);
        pseudoLegalMoves.push_back(move | Constants::Piece::WHITE_KNIGHT << 20);

    } else {
        pseudoLegalMoves.push_back(move | Constants::Piece::BLACK_QUEEN << 20);
        pseudoLegalMoves.push_back(move | Constants::Piece::BLACK_ROOK << 20);
        pseudoLegalMoves.push_back(move | Constants::Piece::BLACK_BISHOP << 20);
        pseudoLegalMoves.push_back(move | Constants::Piece::BLACK_KNIGHT << 20);
    }
}

bool isLegalMove(uint32_t move, GameBoard & board) {
    if (move & Constants::move_decoding_bitmasks[Constants::MoveDecoding::CASTLE]) { //case castle
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

bool isPseudoLegalMove(uint32_t move, GameBoard const &board) {
    Move mv = decodeMove(move);
    if (board.whiteToMove != mv.piece%2) return false;
    if (!(board.pieces[mv.piece] & (1ULL << mv.from))) return false;
    if (mv.captured_piece && !(board.pieces[mv.captured_piece] & 1ULL << mv.to)){
        // captured piece is wrong except it is an en passant capture
        if (mv.to != board.enPassant || (mv.captured_piece != Constants::WHITE_PAWN && mv.captured_piece != Constants::BLACK_PAWN) || (mv.piece != Constants::WHITE_PAWN && mv.piece != Constants::BLACK_PAWN) ) return false;
    }
    if (mv.castle) {
        if (!(board.castleInformation[mv.castle])) return false;
        if (mv.castle == Constants::WHITE_KING_SIDE_CASTLE || mv.castle == Constants::BLACK_KING_SIDE_CASTLE) {
            if (board.allPieces & 3ULL << mv.from+1) return false;
        } else {
            if (board.allPieces & 7ULL << mv.from-3) return false;
        }
        return true;
    }
    uint64_t ownPieces = board.whiteToMove ? board.pieces[Constants::WHITE_KING] | board.pieces[Constants::WHITE_QUEEN]
                                            | board.pieces[Constants::WHITE_ROOK] | board.pieces[Constants::WHITE_BISHOP]
                                            | board.pieces[Constants::WHITE_KNIGHT] | board.pieces[Constants::WHITE_PAWN]
                                            : board.pieces[Constants::BLACK_KING] | board.pieces[Constants::BLACK_QUEEN]
                                            | board.pieces[Constants::BLACK_ROOK] | board.pieces[Constants::BLACK_BISHOP]
                                            | board.pieces[Constants::BLACK_KNIGHT] | board.pieces[Constants::BLACK_PAWN];
    uint64_t enemyPieces = board.allPieces & ~ownPieces;

    if (mv.piece == Constants::WHITE_PAWN || mv.piece == Constants::BLACK_PAWN) {
        uint64_t pseudoLegalSquares = pawnCaptureBitMask[!board.whiteToMove][mv.from] & (enemyPieces | (board.enPassant != -1 ? 1ULL << board.enPassant : 0));
        if (!mv.captured_piece) pseudoLegalSquares = 0;
        int singlePushSquare = (mv.from + (board.whiteToMove ? -8 : 8));
        if (1ULL << singlePushSquare & ~board.allPieces) {
            pseudoLegalSquares |= 1ULL << singlePushSquare;
            if (pawnDoublePushBitMask[!board.whiteToMove][mv.from] & ~board.allPieces) {
                pseudoLegalSquares |= pawnDoublePushBitMask[!board.whiteToMove][mv.from];
            }
        }
        if (!((1ULL << mv.to) & pseudoLegalSquares)) return false;
    } else if (mv.piece == Constants::WHITE_KNIGHT || mv.piece == Constants::BLACK_KNIGHT) {
        uint64_t pseudoLegalSquares = knightAttackBitMasks[mv.from] & ~ownPieces;
        if (!mv.captured_piece) pseudoLegalSquares &= ~enemyPieces;
        if (!((1ULL << mv.to) & pseudoLegalSquares)) return false;
    } else if (mv.piece == Constants::WHITE_BISHOP || mv.piece == Constants::BLACK_BISHOP) {
        uint64_t pseudoLegalSquares = getBishopAttackBits(mv.from,board.allPieces) & ~ownPieces;
        if (!mv.captured_piece) pseudoLegalSquares &= ~enemyPieces;
        if (!((1ULL << mv.to) & pseudoLegalSquares)) return false;
    } else if (mv.piece == Constants::WHITE_ROOK || mv.piece == Constants::BLACK_ROOK) {
        uint64_t pseudoLegalSquares = getRookAttackBits(mv.from,board.allPieces) & ~ownPieces;
        if (!mv.captured_piece) pseudoLegalSquares &= ~enemyPieces;
        if (!((1ULL << mv.to) & pseudoLegalSquares)) return false;
    } else if (mv.piece == Constants::WHITE_QUEEN || mv.piece == Constants::BLACK_QUEEN) {
        uint64_t pseudoLegalSquares = getQueenAttackBits(mv.from,board.allPieces) & ~ownPieces;
        if (!mv.captured_piece) pseudoLegalSquares &= ~enemyPieces;
        if (!((1ULL << mv.to) & pseudoLegalSquares)) return false;
    } else {
        uint64_t pseudoLegalSquares = getKingAttackBits(mv.from,board.allPieces) & ~ownPieces;
        if (!mv.captured_piece) pseudoLegalSquares &= ~enemyPieces;
        if (!((1ULL << mv.to) & pseudoLegalSquares)) return false;
    }
    return true;
}


bool isSquareAttacked(int square, bool attacker_is_white, GameBoard const &board) {
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
    uint64_t possible_slider_positions = getRookAttackBits(square,board.allPieces);
    if (possible_slider_positions & (board.pieces[attacker_is_white?Constants::Piece::WHITE_ROOK:Constants::Piece::BLACK_ROOK] | board.pieces[attacker_is_white?Constants::Piece::WHITE_QUEEN:Constants::Piece::BLACK_QUEEN])) {
        return true;
    }
    //Is square attacked by bishop or queen (diagonal)
    possible_slider_positions = getBishopAttackBits(square,board.allPieces);
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


bool isCastlingThroughCheck(uint32_t move, GameBoard const & board) {
    uint32_t from = (move & Constants::move_decoding_bitmasks[Constants::MoveDecoding::FROM]) >> 4;
    uint32_t to = (move & Constants::move_decoding_bitmasks[Constants::MoveDecoding::TO]) >> 14;
    uint32_t square_between = (from + to) /2;
    return isSquareAttacked(square_between,!board.whiteToMove, board) | isSquareAttacked(from, !board.whiteToMove, board);
}

uint32_t getCompleteMove(GameBoard const & board,uint32_t input_move) {
    vector<uint32_t> pseudoLegalMoves = getPseudoLegalMoves(board,board.whiteToMove,ALL);
    for (uint32_t pseudo_legal_move : pseudoLegalMoves) {
        if (((pseudo_legal_move & Constants::move_decoding_bitmasks[Constants::MoveDecoding::FROM]) == (input_move & Constants::move_decoding_bitmasks[Constants::MoveDecoding::FROM]))
            && ((pseudo_legal_move & Constants::move_decoding_bitmasks[Constants::MoveDecoding::TO]) == (input_move & Constants::move_decoding_bitmasks[Constants::MoveDecoding::TO]))) {
            return pseudo_legal_move;
            }
    }
    throw std::invalid_argument("Invalid input move");
}