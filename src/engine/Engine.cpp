//
// Created by salom on 13.07.2025.
//
#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <Conversions.h>
#include <set>
#include <queue>

using std::vector;

#include <utility>
using std::pair;
#include <future>
#include <chrono>
#include <atomic>

#include "Engine.h"
#include "Memory.h"
#include "EvaluationFunction.h"
#include "Movepicking.h"

#include "Constants.h" //lib constants
#include "EvaluationConstants.h"
using Constants::move_decoding_bitmasks;
using Constants::MoveDecoding;

#include "GameBoard.h" // lib game_logic
#include "MoveGeneration.h"

#include "Output.h" // lib io




int total_nodes_searched = 0;
int lowest_max_recursion_depth = 0;
int futility_attempts = 0;
int futility_researches = 0;
int lmr_attempts = 0;
int lmr_researches = 0;
std::array<int,5> cutoffs = {};





std::atomic<bool> timeIsUp(false);
pair<uint32_t,int> iterativeDeepening(GameBoard & board, int timeLimit) {
    timeIsUp = false;

    std::thread timeGuard( startTimeLimit,timeLimit);
    auto start = std::chrono::high_resolution_clock::now();
    auto dontStartNewDepthTime = start + (std::chrono::milliseconds((timeLimit)/2));

    total_nodes_searched = 0;

    pair<uint32_t,int> currentBestMove = {};
    pair<uint32_t,int> incompleteMoveCalculation = {}; // result is first stored in here. When timeIsUp the result might be incomplete
                                                        // if not timeIsUp in the next iteration we know that the last value is usable
    for (int depth = 0; depth <= 30; depth++) {
        if (timeIsUp) break;

        //debug
        lmr_attempts = 0;
        lmr_researches = 0;
        futility_attempts = 0;
        futility_researches = 0;



        currentBestMove = incompleteMoveCalculation;
        lowest_max_recursion_depth = 0;
        incompleteMoveCalculation = getOptimalMoveNegaMax(board,depth);
        if (!timeIsUp) {
            printAnalysisData(incompleteMoveCalculation,depth, depth-lowest_max_recursion_depth ,start,total_nodes_searched);
        }

        //debug
        /*std::cout << "Futility Attempts: " << futility_attempts << std::endl;
        std::cout << "Futility Researches: " << futility_researches << std::endl;
        std::cout << "LMR Attempts: " << lmr_attempts << std::endl;
        std::cout << "LMR Researches: " << lmr_researches << std::endl;*/


        // Not worth starting a new depth
        if (!timeIsUp && dontStartNewDepthTime-std::chrono::high_resolution_clock::now() < std::chrono::milliseconds(0)) {
            currentBestMove = incompleteMoveCalculation;
            timeIsUp = true;
            break;
        }
        decreaseAllMoveScores();
    }

    timeGuard.join();
    return currentBestMove;
}

void startTimeLimit(int timeLimit) {
    auto deadline = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeLimit);
    while (deadline > std::chrono::high_resolution_clock::now() && !timeIsUp) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    timeIsUp = true;
}


// basically the first call of the negamax algorithm but instead of just returning an evaluation
// it returns the best move with the evaluation so the computer knows which move to make
pair<uint32_t,int> getOptimalMoveNegaMax(GameBoard & board, int maxRecursionDepth) {
    //TODO save exact move order for better move ordering and search extensions
    total_nodes_searched++;
    cutoffs = {};

    Data savedData = getData(board.zobristHash);

    int alpha = -CHECKMATE_VALUE;
    int beta = CHECKMATE_VALUE;
    int max = -CHECKMATE_VALUE;

    uint32_t currentBestMove = 0;

    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    //std::priority_queue<pair<int,uint32_t>> MoveOrder;

    MoveGenPhase phase = TTMove;
    int extension = 1;

    while (phase != Done) {
        bool breakWhile = false;
        auto moves = pickNextMoves(savedData, killer_moves[maxRecursionDepth] ,board,phase);

        for (uint32_t move : moves) {
            if (!isLegalMove(move, board)) continue;
            //printCompleteMove(decodeMove(move));

            board.applyPseudoLegalMove(move);
            int currentValue = -negaMax(board,maxRecursionDepth-1+extension,updateAlphaBetaValue(-beta),updateAlphaBetaValue(-alpha));
            //std::cout << "Evaluation " << currentValue << std::endl;
            board.unmakeMove(move, enPassant,castle_rights,plies,hash_before);

            //MoveOrder.emplace(currentValue,move);
            if (currentValue > max) {
                max = currentValue;
                currentBestMove = move;
                if (max > alpha) {
                    alpha = max;
                }
            }
        }

        extension = 0; // only TTmove extended by 1
        phase = static_cast<MoveGenPhase>(phase +1);
    }

    /*for (int i = 0; i < 10; i++) {
        auto move = MoveOrder.top();
        MoveOrder.pop();
        std::cout << "Move " << longAlgebraicNotation(move.second) << " Eval: " << move.first << std::endl;
    }*/


    /*std::cout << "TT Cutoffs: " << cutoffs[TTMove] << std::endl;
    std::cout << "Killer Cutoffs: " << cutoffs[Killer] << std::endl;
    std::cout << "Capture Cutoffs: " << cutoffs[Captures] << std::endl;
    std::cout << "Quiet Cutoffs: " << cutoffs[Quiets] << std::endl;*/

    if (!timeIsUp) tryMakeNewEntry(EXACT,maxRecursionDepth,max,currentBestMove,board);
    return {currentBestMove,max};
}

int negaMax(GameBoard & board, int maxRecursionDepth, int alpha, int beta) {
    if (timeIsUp) return Constants::TIME_IS_UP_FLAG;

    total_nodes_searched++;

    int position_repetitions = board.board_positions[board.zobristHash];
    if (position_repetitions >= 3) return 0; // threefold repetition, early check
    if (maxRecursionDepth <= 0) return updateReturnValue(quiscenceSearch(board,maxRecursionDepth,(alpha),(beta)));

    Data savedData = getData(board.zobristHash);
    if (savedData.evaluationFlag != EMPTY && position_repetitions < 2 && savedData.depth >= maxRecursionDepth) {
        if (savedData.evaluationFlag == EXACT) {
            return updateReturnValue(savedData.evaluation); // mate in ... evaluation becomes mate in ...+ 1 ply
        }
        if (savedData.evaluationFlag == UPPER_BOUND && savedData.evaluation <= alpha) {
            return updateReturnValue(savedData.evaluation);
        }
        if (savedData.evaluationFlag == LOWER_BOUND && savedData.evaluation >= beta) {
            return updateReturnValue(savedData.evaluation);
        }
    }

    int max =  -CHECKMATE_VALUE-100;
    bool foundLegalMove = false;
    int originalAlpha = alpha;
    uint32_t bestMove = 0;

    //Data for unmaking the move
    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;


    MoveGenPhase phase = TTMove;

    bool isCheck = board.isCheck(board.whiteToMove);
    int num_pieces = __builtin_popcountll(board.allPieces);
    int standing_pat = CHECKMATE_VALUE;
    if (num_pieces >= 14 && maxRecursionDepth <= 4) standing_pat = evaluate(board,alpha,beta); // no futility pruning in end game


    while (phase != Done) {
        bool breakWhile = false;
        auto moves = pickNextMoves(savedData, killer_moves[maxRecursionDepth] ,board,phase);
        int move_number = 0;
        for (uint32_t move : moves) {
            if (!isLegalMove(move, board)) continue;

            if (move == savedData.bestMove && phase != TTMove) continue;
            if (move == killer_moves[maxRecursionDepth] && phase > Killer) continue;

            foundLegalMove = true;
            move_number++;

            board.applyPseudoLegalMove(move);
            int currentValue;

            if (phase == Quiets && !isCheck) {

            /*Conditions:
                Futility: Quiet, not in check, not the best move, only middlegame
                LMR: Quiet, not in check, higher depth,

              */
                // late move reduction/pruning
                bool search_at_full_depth = true;
                bool isPawnMove = (move & move_decoding_bitmasks[MoveDecoding::PIECE]) <= Constants::BLACK_PAWN;
                if (move_number > 1 && num_pieces >= 14 && standing_pat < alpha - FUTILITY_SAFETY_MARGIN*maxRecursionDepth) {
                    futility_attempts++;
                    currentValue = (maxRecursionDepth <= 2) ? -quiscenceSearch(board,0,(-beta),(-alpha)) : -negaMax(board,2,updateAlphaBetaValue(-beta),updateAlphaBetaValue(-alpha));
                    if (currentValue <= alpha) {
                        search_at_full_depth = false;
                    } else futility_researches++;
                }
                else if (maxRecursionDepth > 4 && move_number > 2 && !(num_pieces < 14)) {
                    lmr_attempts++;
                    currentValue = -negaMax(board,maxRecursionDepth-2,updateAlphaBetaValue(-beta),updateAlphaBetaValue(-alpha));
                    if (currentValue <= alpha) search_at_full_depth = false;
                    else lmr_researches++;
                }
                if (search_at_full_depth) {
                    currentValue = -negaMax(board,maxRecursionDepth-1,updateAlphaBetaValue(-beta),updateAlphaBetaValue(-alpha));
                }
            } else {
                currentValue = -negaMax(board,maxRecursionDepth-1,updateAlphaBetaValue(-beta),updateAlphaBetaValue(-alpha));
            }
            board.unmakeMove(move, enPassant,castle_rights,plies,hash_before);

            if (currentValue > max) {
                max = currentValue;
                bestMove = move;
                if (max > alpha) {
                    alpha = max;
                }
            }
            if (alpha >= beta) {
                if (!(move & move_decoding_bitmasks[MoveDecoding::CAPTURE])) killer_moves[maxRecursionDepth] = move;
                increaseMoveScore(move,maxRecursionDepth);
                cutoffs[phase-1]++;
                breakWhile = true;
                break;
            }
        }

        if (breakWhile) break;
        phase = static_cast<MoveGenPhase>(phase +1);
    }

    if (foundLegalMove) {
        Evaluation_Flag evaluation_flag;
        if (max <= originalAlpha) {
            evaluation_flag = UPPER_BOUND;
        } else if (max >= beta) {
            evaluation_flag = LOWER_BOUND;
        } else {
            evaluation_flag = EXACT;
        }
        if (!timeIsUp) tryMakeNewEntry(evaluation_flag,maxRecursionDepth,(max),bestMove,board);
        return updateReturnValue(max);
    }
    int mate_evaluation = board.isCheck(board.whiteToMove) ?  - CHECKMATE_VALUE : 0;
    //if (!timeIsUp) tryMakeNewEntry(EXACT,Constants::INFINITE,(mate_evaluation),bestMove,board);
    return updateReturnValue(mate_evaluation);
}

int quiscenceSearch(GameBoard & board, int maxRecursionDepth, int alpha, int beta) {

    if (timeIsUp) return Constants::TIME_IS_UP_FLAG;

    total_nodes_searched++;
    if (maxRecursionDepth < lowest_max_recursion_depth) lowest_max_recursion_depth = maxRecursionDepth;

    int current_eval = evaluate(board, alpha, beta);
    if (maxRecursionDepth <= -8) return current_eval;
    if (current_eval >= beta) return current_eval;
    if (current_eval > alpha) alpha = current_eval;

    //Data for unmaking the move
    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    auto only_capture_moves = getPseudoLegalMoves(board,board.whiteToMove,CAPTURES);
    mvv_lva_MoveOrdering(only_capture_moves);

    for (uint32_t move : only_capture_moves) {

        int captured_piece_value = abs(STATIC_EG_PIECE_VALUES[(move_decoding_bitmasks[MoveDecoding::CAPTURE] & move) >> 10]);
        if (current_eval + captured_piece_value + LAZY_EVAL_SAFETY_MARGIN < alpha) continue; // delta pruning
        if (!isLegalMove(move,board)) continue;

        board.applyPseudoLegalMove(move);
        int currentValue = -quiscenceSearch(board,maxRecursionDepth-1,(-beta),(-alpha));
        board.unmakeMove(move,enPassant,castle_rights,plies,hash_before);
        if (currentValue > alpha) {
            alpha = currentValue;
            if (alpha >= beta) {
                break;
            }
        }
    }
    return alpha;
}
