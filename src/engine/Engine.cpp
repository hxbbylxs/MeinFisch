//
// Created by salom on 13.07.2025.
//
#include <iostream>
#include <vector>
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
#include "Conversions.h"

#include "Output.h" // lib io




int total_nodes_searched = 0;
int highest_depth = 0;
int futility_attempts = 0;
int futility_researches = 0;
int lmr_attempts = 0;
int lmr_researches = 0;
std::array<int,NUM_MOVE_GEN_PHASES> cutoffs = {};





std::atomic<bool> timeIsUp(false);
pair<uint32_t,int> iterativeDeepening(GameBoard & board, int timeLimit, int max_depth) {
    timeIsUp = false;
    std::thread timeGuard( startTimeLimit,timeLimit);
    auto start = std::chrono::high_resolution_clock::now();
    auto dontStartNewDepthTime = start + (std::chrono::milliseconds((timeLimit)/3));

    total_nodes_searched = 0;

    pair<uint32_t,int> currentBestMove = {};
    pair<uint32_t,int> incompleteMoveCalculation = {}; // result is first stored in here. When timeIsUp the result might be incomplete
                                                        // if not timeIsUp in the next iteration we know that the last value is usable
    for (int depth = 0; depth <= max_depth; depth++) {
        if (timeIsUp) break;

        //debug
        lmr_attempts = 0;
        lmr_researches = 0;
        futility_attempts = 0;
        futility_researches = 0;



        currentBestMove = incompleteMoveCalculation;
        highest_depth = 0;
        incompleteMoveCalculation = getOptimalMoveNegaMax(board,depth);
        if (!timeIsUp) {
            std::string pv = reconstructPV(board);
            printAnalysisData(incompleteMoveCalculation,depth, highest_depth ,start,total_nodes_searched,pv);
        }

        decreaseAllMoveScores();

        // Not worth starting a new depth or last depth reached
        if (!timeIsUp && (depth == max_depth || std::chrono::high_resolution_clock::now() >= dontStartNewDepthTime)) {
            currentBestMove = incompleteMoveCalculation;
            timeIsUp = true;
            break;
        }

    }
    if (currentBestMove.first == 0) currentBestMove = incompleteMoveCalculation; // in case timeIsUp after depth 0, currentBestMove is invalid

    timeGuard.join();
    return currentBestMove;
}

void startTimeLimit(int timeLimit) {
    auto deadline = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(timeLimit);
    while (deadline > std::chrono::high_resolution_clock::now() && !timeIsUp) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    timeIsUp = true;
}


// the first call of the negamax algorithm but instead of just returning an evaluation
// it returns the best move with the evaluation so the computer knows which move to make
pair<uint32_t,int> getOptimalMoveNegaMax(GameBoard & board, int maxRecursionDepth) {

    total_nodes_searched++;
    //cutoffs = {};

    Data savedData = getData(board.zobristHash);

    int alpha = -CHECKMATE_VALUE;
    int beta = CHECKMATE_VALUE;
    int max = -CHECKMATE_VALUE;

    uint32_t currentBestMove = 0;

    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;


    auto moves = getPseudoLegalMoves(board,board.whiteToMove,ALL);
    staticMoveOrdering(moves,board);

    for (uint32_t move : moves) {
        if (!isLegalMove(move, board)) continue;


        board.applyPseudoLegalMove(move);
        int currentValue = -negaMax(board,maxRecursionDepth-1,updateAlphaBetaValue(-beta),updateAlphaBetaValue(-alpha),1,move);
        board.unmakeMove(move, enPassant,castle_rights,plies,hash_before);

        if (currentValue > max) {
            max = currentValue;
            currentBestMove = move;
            if (max > alpha) {
                alpha = max;
            }
        }

    }

    if (!timeIsUp) tryMakeNewEntry(EXACT,maxRecursionDepth,max,currentBestMove,board);
    return {currentBestMove,max};
}

int negaMax(GameBoard & board, int maxRecursionDepth, int alpha, int beta, int depth, uint32_t previous_move) {


    if (timeIsUp) return Constants::TIME_IS_UP_FLAG;


    total_nodes_searched++;

    int position_repetitions = board.board_positions[board.zobristHash];
    if (position_repetitions >= 3) return 0; // threefold repetition, early check
    if (maxRecursionDepth <= 0) return updateReturnValue(quiscenceSearch(board,maxRecursionDepth,(alpha),(beta),depth));
    if (alpha > CHECKMATE_VALUE) return updateReturnValue(alpha); // found earlier checkmate


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
    std::vector<uint32_t> bad_moves_for_later = {};

    bool isCheck = board.isCheck(board.whiteToMove);
    if (isCheck) maxRecursionDepth++; // check extension
    int num_pieces = __builtin_popcountll(board.white_pieces | board.black_pieces);

    uint32_t killer_candidate = killer_moves[depth];
    Move prev_mv = decodeMove(previous_move);
    uint32_t counter_candidate = counter_moves[prev_mv.from][prev_mv.to];


    while (phase != Done) {
        bool breakWhile = false;

        auto moves = pickNextMoves(savedData,counter_candidate ,killer_candidate ,board,phase);
        if (phase == Bad_Moves) moves = bad_moves_for_later;


        int move_number = 0;
        for (uint32_t move : moves) {

            if (!isLegalMove(move, board)) continue;

            if (move == savedData.bestMove && phase > TTMove) continue;
            if (move == counter_candidate && phase > Counter) continue; // Counter and Killer are not captures!!
            if (move == killer_candidate && phase > Killer) continue;

            if ((phase == Good_Captures || phase == Good_Quiets) && static_exchange_evaluation(move,board) < 0) {
                bad_moves_for_later.push_back(move);
                continue;
            }

            foundLegalMove = true;
            move_number++;

            board.applyPseudoLegalMove(move);
            int currentValue;

            bool research_necessary = true;
            switch (phase) {
                case TTMove:
                    [[fallthrough]];
                case Good_Captures:
                    [[fallthrough]];
                case Killer:
                    [[fallthrough]];
                case Counter:
                    research_necessary = false;
                    currentValue = -negaMax(board,maxRecursionDepth-1,updateAlphaBetaValue(-(beta)),updateAlphaBetaValue(-alpha),depth+1,move);
                    break;
                case Good_Quiets:
                    if (!isCheck && move_number > 2 && num_pieces >= 14) {
                        // late move reduction
                        lmr_attempts++;
                        currentValue = -negaMax(board,maxRecursionDepth-2,updateAlphaBetaValue(-(alpha+1)),updateAlphaBetaValue(-alpha),depth+1,move);
                    } else {
                        currentValue = -negaMax(board,maxRecursionDepth-1,updateAlphaBetaValue(-(alpha+1)),updateAlphaBetaValue(-alpha),depth+1,move);
                    }
                    if (currentValue <= alpha) research_necessary = false;
                    break;
                case Bad_Moves:
                    if (!isCheck && (maxRecursionDepth < 3 || depth > 6) && num_pieces >= 14) {
                        lmr_attempts++;
                        currentValue = -quiscenceSearch(board,0,(-(alpha+1)),(-alpha),depth+1);
                        if (currentValue <= alpha) research_necessary = false;
                    } else if (!isCheck && num_pieces >= 14) {
                        lmr_attempts++;
                        currentValue = -negaMax(board,maxRecursionDepth-2,updateAlphaBetaValue(-(alpha+1)),updateAlphaBetaValue(-alpha),depth+1,move);
                        if (currentValue <= alpha) research_necessary = false;
                    }
                default:
                    break;
            }
            if (research_necessary) {
                currentValue = -negaMax(board,maxRecursionDepth-1,updateAlphaBetaValue(-(beta)),updateAlphaBetaValue(-alpha),depth+1,move);
                lmr_researches++;
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
                if (!(move & move_decoding_bitmasks[MoveDecoding::CAPTURE])) {
                    killer_moves[depth] = move;
                    counter_moves[prev_mv.from][prev_mv.to] = move;
                }
                increaseMoveScore(move,maxRecursionDepth);
                //cutoffs[phase]++;
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

int quiscenceSearch(GameBoard & board, int maxRecursionDepth, int alpha, int beta, int depth) {

    if (timeIsUp) return Constants::TIME_IS_UP_FLAG;

    total_nodes_searched++;
    if (depth > highest_depth) highest_depth = depth;

    Data savedData = getData(board.zobristHash);
    if (savedData.evaluationFlag != EMPTY && savedData.depth >= maxRecursionDepth) {
        if (savedData.evaluationFlag == EXACT) {
            return (savedData.evaluation); // mate in ... evaluation becomes mate in ...+ 1 ply
        }
        if (savedData.evaluationFlag == UPPER_BOUND && savedData.evaluation <= alpha) {
            return (savedData.evaluation);
        }
        if (savedData.evaluationFlag == LOWER_BOUND && savedData.evaluation >= beta) {
            return (savedData.evaluation);
        }
    }

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
        int currentValue = -quiscenceSearch(board,maxRecursionDepth-1,(-beta),(-alpha),depth+1);
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


std::string reconstructPV(GameBoard & board) {

    // get best move from transposition table
    Data savedData = getData(board.zobristHash);
    if (savedData.evaluationFlag != EMPTY && board.board_positions[board.zobristHash] < 3) {

        //Data for unmaking the move
        int plies = board.plies;
        auto castle_rights = board.castleInformation;
        int enPassant = board.enPassant;
        uint64_t hash_before = board.zobristHash;

        // make move
        board.applyPseudoLegalMove(savedData.bestMove);

        // recursively add the next pv move
        std::string pv = longAlgebraicNotation(savedData.bestMove) + " " + reconstructPV(board);

        board.unmakeMove(savedData.bestMove,enPassant,castle_rights,plies,hash_before);

        return pv;
    }
    return ""; // no valid tt entry (pv end)
}