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
#include <cassert>

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

#include "Network.h" // lib nnue




int total_nodes_searched = 0;
int highest_depth = 0;


inline std::chrono::time_point<std::chrono::system_clock> deadline; // ATTENTION - engine uses currently only one search thread (no race conditions)

void iterativeDeepening(GameBoard & board, int timeLimit, int max_depth) {
    stop_search = false;
    search_ongoing = true;

    auto start = std::chrono::high_resolution_clock::now();
    deadline = start + std::chrono::milliseconds(timeLimit);
    auto dontStartNewDepthTime = start + (std::chrono::milliseconds((timeLimit)/3));

    total_nodes_searched = 0;
    nnue.reset_to(board);

    pair<Move,int> currentBestMove = {};
    pair<Move,int> incompleteMoveCalculation = {}; // result is first stored in here. When timeIsUp the result might be incomplete
                                                        // if not timeIsUp in the next iteration we know that the last value is usable
    for (int depth = 0; depth <= max_depth; depth++) {
        if (stop_search) break;

        //debug
        nodes_at_depth = {};
        qnodes_at_depth = {};
        cutoffs_at_move_number_cutnodes = {};
        cutoffs_at_move_number_allnodes = {};
        cutoffs_at_move_gen_phase = {};
        pv_changes_at_depth = {};
        total_researches = 0;
        nmp_prunes = 0;
        razoring_prunes = 0;
        exact_tt_hits = 0;
        bound_tt_hits = 0;
        qsearch_tt_hits = 0;

        currentBestMove = incompleteMoveCalculation;
        highest_depth = 0;
        incompleteMoveCalculation = getOptimalMoveNegaMax(board,depth);
        if (!stop_search) {
            std::string pv = reconstructPV(board);
            printAnalysisData(incompleteMoveCalculation,depth, highest_depth ,start,total_nodes_searched,pv);
        }

        decreaseAllMoveScores();

        // Not worth starting a new depth or last depth reached
        if (!stop_search && (depth == max_depth || std::chrono::high_resolution_clock::now() >= dontStartNewDepthTime)) {
            currentBestMove = incompleteMoveCalculation;
            stop_search = true;
            break;
        }

    }
    if (currentBestMove.first.value == 0) currentBestMove = incompleteMoveCalculation; // in case timeIsUp after depth 0, currentBestMove is invalid
    std::cout << "bestmove " << longAlgebraicNotation(currentBestMove.first) << std::endl;
    search_ongoing = false;
}



// the first call of the negamax algorithm but instead of just returning an evaluation
// it returns the best move with the evaluation so the computer knows which move to make
pair<Move,int> getOptimalMoveNegaMax(GameBoard & board, int remaining_depth) {

    total_nodes_searched++;
    nodes_at_depth[0]++;

    Data savedData = getData(board.zobristHash);

    int alpha = -CHECKMATE_VALUE;
    int beta = CHECKMATE_VALUE;
    int max = -CHECKMATE_VALUE;

    Move currentBestMove = 0;

    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    MoveGenPhase phase = TTMove;
    std::vector<Move> bad_moves_for_later = {};
    NodeType child_node_type = NodeType::PV_NODE;

    bool isCheck = board.isCheck(board.whiteToMove);

    Move killer_candidate = 0;
    Move counter_candidate = 0;

    while (phase != Done) {

        auto moves = pickNextMoves(savedData,counter_candidate ,killer_candidate ,board,phase);
        if (phase == Bad_Moves) moves = bad_moves_for_later;


        int move_number = 0;
        for (Move move : moves) {

            if (!isLegalMove(move, board)) continue;

            if (move == savedData.bestMove && phase > TTMove) continue;
            if (move == counter_candidate && phase > Counter) continue; // Counter and Killer are not captures!!
            if (move == killer_candidate && phase > Killer) continue;

            if ((phase == Good_Captures || phase == Good_Quiets) && static_exchange_evaluation(move,board) < 0) {
                bad_moves_for_later.push_back(move);
                continue;
            }
            
            move_number++;

            board.applyPseudoLegalMove(move);
            nnue.update(board,move,false,enPassant);

            int currentValue;
            int search_window_alpha = alpha;
            int search_window_beta = beta;
            int depth_reduction = 0;
            if (child_node_type == NodeType::CUT_NODE) {
                search_window_beta = alpha+1; // Null Window
            }

            switch (phase) {
                case TTMove:
                    depth_reduction = -1;
                    break;
                case Good_Captures:
                    [[fallthrough]];
                case Killer:
                    [[fallthrough]];
                case Counter:
                    break;
                case Good_Quiets:
                    if (!isCheck && move_number > 5 && lmr_active && remaining_depth > 2) {
                        // late move reduction
                        depth_reduction = 1;
                    }
                    break;
                case Bad_Moves:
                    if (!isCheck && lmr_active && remaining_depth > 2) {
                        depth_reduction = 1;
                    }
                default:
                    break;
            }
            currentValue = -negaMax(board,
                                        remaining_depth-1-depth_reduction,
                                        updateAlphaBetaValue(-search_window_beta),updateAlphaBetaValue(-search_window_alpha),
                                        1,move,true,child_node_type);
            if (child_node_type == NodeType::CUT_NODE && currentValue > alpha) {
                currentValue = -negaMax(board,
                                            remaining_depth-1,
                                                updateAlphaBetaValue(-(beta)),updateAlphaBetaValue(-alpha),
                                                1,move,true,NodeType::PV_NODE);
            }

            child_node_type = NodeType::CUT_NODE; // every node after the first one is an expected cut node
            board.unmakeMove(move, enPassant,castle_rights,plies,hash_before);
            nnue.update(board,move,true,enPassant);

            if (currentValue > max) {
                max = currentValue;
                currentBestMove = move;
                if (max > alpha) {
                    alpha = max;
                }
            }
        }

        phase = static_cast<MoveGenPhase>(phase +1);
    }

    if (!stop_search) tryMakeNewEntry(EXACT,remaining_depth,max,currentBestMove,board);
    return {currentBestMove,max};
}

int negaMax(GameBoard & board, int remaining_depth, int alpha, int beta, int depth, Move previous_move, bool null_move_allowed, NodeType node_type) {

    if (std::chrono::high_resolution_clock::now() > deadline) stop_search = true;
    if (stop_search) return Constants::TIME_IS_UP_FLAG;

    total_nodes_searched++;
    nodes_at_depth[depth]++;

    int position_repetitions = board.board_positions[board.zobristHash];
    if (position_repetitions >= 3) return 0; // threefold repetition
    if (alpha > CHECKMATE_VALUE) return updateReturnValue(alpha); // found earlier checkmate

    bool isCheck = board.isCheck(board.whiteToMove);
    if (isCheck) remaining_depth++; // check extension
    if (remaining_depth <= 0) return updateReturnValue(quiscenceSearch(board,remaining_depth,(alpha),(beta),depth));

    Data savedData = getData(board.zobristHash);
    if (savedData.evaluationFlag != EMPTY && position_repetitions < 2 && savedData.depth >= remaining_depth) {
        if (savedData.evaluationFlag == EXACT) {
            exact_tt_hits++;
            return updateReturnValue(savedData.evaluation); // mate in ... evaluation becomes mate in ...+ 1 ply
        }
        if (savedData.evaluationFlag == UPPER_BOUND && savedData.evaluation <= alpha) {
            bound_tt_hits++;
            return updateReturnValue(savedData.evaluation);
        }
        if (savedData.evaluationFlag == LOWER_BOUND && savedData.evaluation >= beta) {
            bound_tt_hits++;

            // reward fail high tt entries
            if (!(savedData.bestMove.capture())) {
                killer_moves[depth] = savedData.bestMove;
                counter_moves[previous_move.from()][previous_move.to()] = savedData.bestMove;
            }
            increaseMoveScore(savedData.bestMove,remaining_depth);

            return updateReturnValue(savedData.evaluation);
        }
    }


    int max =  -CHECKMATE_VALUE-100;
    bool foundLegalMove = false;
    int originalAlpha = alpha;
    Move bestMove = 0;

    //Data for unmaking the move
    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;


    // Null move pruning
    // if the side to move has a piece left that is not a pawn or the king the danger of zugzwang is low
    bool zugzwang_danger = !(board.whiteToMove ?  (board.white_pieces & ~board.pieces[Constants::WHITE_KING] & ~board.pieces[Constants::WHITE_PAWN])
                                                : (board.black_pieces & ~board.pieces[Constants::BLACK_KING] & ~board.pieces[Constants::BLACK_PAWN]));
    if (!isCheck && null_move_allowed && remaining_depth > 3 && !zugzwang_danger && nmp_active) {
        int depth_reduction = 2 + remaining_depth/6;
        board.makeNullMove();
        int null_move_evaluation = -negaMax(board,
                                                remaining_depth-depth_reduction,
                                                updateAlphaBetaValue(-beta),updateAlphaBetaValue(-(beta-1)),
                                                depth,0,false,NodeType::CUT_NODE);
        board.unmakeNullMove(enPassant, hash_before);
        if (null_move_evaluation >= beta) {
            nmp_prunes++;
            return updateReturnValue(null_move_evaluation);
        }
    }

    // Razoring
    // if we are in a leaf node and the static_eval is way worse than alpha, only captures or promotions could help lifting the eval above alpha
    // therefore we go directly into quiescence search
    if (remaining_depth == 1 && !isCheck && razoring_active) {
        int static_eval = evaluate(board);
        if (static_eval + 75 < alpha) {
            razoring_prunes++;
            return updateReturnValue(quiscenceSearch(board,0,alpha,beta,depth+1));
        }
    }


    MoveGenPhase phase = TTMove;
    std::vector<Move> bad_moves_for_later = {};

    NodeType child_node_type;
    switch (node_type) {
        case NodeType::CUT_NODE:
            child_node_type = NodeType::ALL_NODE;
            break;
        case NodeType::ALL_NODE:
            child_node_type = NodeType::CUT_NODE;
            break;
        case NodeType::PV_NODE:
            child_node_type = NodeType::PV_NODE;
            break;
    }

    Move killer_candidate = killer_moves[depth];
    Move counter_candidate = counter_moves[previous_move.from()][previous_move.to()];

    int total_move_number = 0;

    // the the tt move is a capture with sufficient depth, reduce all the other moves
    int singular_tt_move_reduction = 0;
    if (savedData.bestMove.capture()) {
        singular_tt_move_reduction = savedData.depth / 6;
    }

    while (phase != Done) {
        bool breakWhile = false;

        auto moves = pickNextMoves(savedData,counter_candidate ,killer_candidate ,board,phase);
        if (phase == Bad_Moves) moves = bad_moves_for_later;


        int move_number = 0;
        for (Move move : moves) {

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
            total_move_number++;

            board.applyPseudoLegalMove(move);
            nnue.update(board,move,false,enPassant);

            int currentValue = -CHECKMATE_VALUE;

            int search_window_alpha = alpha;
            int search_window_beta = beta;
            int reduction = 0;
            if (child_node_type != NodeType::PV_NODE) {
                search_window_beta = alpha +1;
            }

            switch (phase) {
                case TTMove:
                    break;
                case Good_Captures:
                    if (!isCheck && move_number > 4 && lmr_active && remaining_depth > 3) {
                        reduction = 1 + singular_tt_move_reduction;
                    }
                    break;
                case Killer:
                    reduction = singular_tt_move_reduction;
                    break;
                case Counter:
                    reduction = singular_tt_move_reduction;
                    break;
                case Good_Quiets:
                    if (!isCheck && move_number > 2 && lmr_active && remaining_depth > 2) {
                        reduction = static_cast<int>(0.7844 + log(depth) * log(total_move_number) / 2.4696) + singular_tt_move_reduction;
                    }
                    break;
                case Bad_Moves:
                    if (!isCheck && lmr_active) {
                        reduction = static_cast<int>(0.7844 + log(depth) * log(total_move_number) / 2.4696) + singular_tt_move_reduction;
                    }
                    break;
                default:
                    break;
            }
            currentValue = -negaMax(board,
                                            remaining_depth-1-reduction,
                                            updateAlphaBetaValue(-(search_window_beta)),updateAlphaBetaValue(-search_window_alpha),
                                            depth+1,move,true,child_node_type);
            // research if
            // - child was searched with null_window or depth was reduced
            // - and fails high
            if (currentValue > search_window_alpha && (child_node_type == NodeType::CUT_NODE || reduction >= 1)) {
                total_researches++;
                currentValue = -negaMax(board,
                                            remaining_depth-1,
                                            updateAlphaBetaValue(-(beta)),updateAlphaBetaValue(-alpha),
                                            depth+1,move,true,NodeType::PV_NODE);
            }
            if (child_node_type != NodeType::PV_NODE && node_type == NodeType::PV_NODE && currentValue > alpha) {
                pv_changes_at_depth[depth]++;
            }

            child_node_type = NodeType::CUT_NODE; // every further child node is a cut node
            board.unmakeMove(move, enPassant,castle_rights,plies,hash_before);
            nnue.update(board,move,true,enPassant);

            if (!stop_search) {
                assert(currentValue < CHECKMATE_VALUE+100 && currentValue > -CHECKMATE_VALUE-100);
            }

            if (currentValue > max) {
                max = currentValue;
                bestMove = move;
                if (max > alpha) {
                    alpha = max;
                }
            }
            if (alpha >= beta) {
                if (!(move.capture())) {
                    killer_moves[depth] = move;
                    counter_moves[previous_move.from()][previous_move.to()] = move;
                }
                increaseMoveScore(move,remaining_depth);

                cutoffs_at_move_gen_phase[phase]++;
                if (node_type == NodeType::ALL_NODE) cutoffs_at_move_number_allnodes[total_move_number]++;
                if (node_type == NodeType::CUT_NODE) cutoffs_at_move_number_cutnodes[total_move_number]++;

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
        if (!stop_search) tryMakeNewEntry(evaluation_flag,remaining_depth,(max),bestMove,board);
        return updateReturnValue(max);
    }
    int mate_evaluation = board.isCheck(board.whiteToMove) ?  - CHECKMATE_VALUE : 0;
    //if (!timeIsUp) tryMakeNewEntry(EXACT,Constants::INFINITE,(mate_evaluation),bestMove,board);


    return updateReturnValue(mate_evaluation);
}

int quiscenceSearch(GameBoard & board, int remaining_depth, int alpha, int beta, int depth) {

    if (stop_search) return Constants::TIME_IS_UP_FLAG;

    total_nodes_searched++;
    qnodes_at_depth[depth]++;
    if (depth > highest_depth) highest_depth = depth;

    Data savedData = getData(board.zobristHash);
    if (savedData.evaluationFlag != EMPTY && savedData.depth >= remaining_depth) {
        if (savedData.evaluationFlag == EXACT) {
            qsearch_tt_hits++;
            return (savedData.evaluation); // mate in ... evaluation becomes mate in ...+ 1 ply
        }
        if (savedData.evaluationFlag == UPPER_BOUND && savedData.evaluation <= alpha) {
            qsearch_tt_hits++;
            return (savedData.evaluation);
        }
        if (savedData.evaluationFlag == LOWER_BOUND && savedData.evaluation >= beta) {
            qsearch_tt_hits++;
            return (savedData.evaluation);
        }
    }

    int current_eval = evaluate(board);
    if (remaining_depth <= -8) return current_eval;
    if (current_eval >= beta) return current_eval;
    if (current_eval > alpha) alpha = current_eval;

    //Data for unmaking the move
    int plies = board.plies;
    auto castle_rights = board.castleInformation;
    int enPassant = board.enPassant;
    uint64_t hash_before = board.zobristHash;

    MoveGenPhase phase = QCaptures;
    while (phase != QDone) {
        bool break_while = false;
        std::vector<Move> moves = pickNextMoves(savedData,0,0,board,phase);
        for (Move move : moves) {

            int move_gain = abs(STATIC_EG_PIECE_VALUES[move.capture()]) + abs(STATIC_EG_PIECE_VALUES[move.promotion()]);
            if (current_eval + move_gain + 45 < alpha) continue; // delta pruning
            if (!isLegalMove(move,board)) continue;

            board.applyPseudoLegalMove(move);
            nnue.update(board,move,false,enPassant);

            int currentValue = -quiscenceSearch(board,remaining_depth-1,(-beta),(-alpha),depth+1);

            board.unmakeMove(move,enPassant,castle_rights,plies,hash_before);
            nnue.update(board,move,true,enPassant);

            if (currentValue > alpha) {
                alpha = currentValue;
                if (alpha >= beta) {
                    break_while = true;
                    break;
                }
            }
        }
        if (break_while) break;
        phase = static_cast<MoveGenPhase>(phase +1);
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