//
// Created by salom on 08.10.2025.
//

#include <algorithm>
#include <fstream>
#include <iostream>
#include <cassert>

#include "Network.h"

#include "MoveGenerationConstants.h"

#include "Utils.h" // lib utils

unsigned calculateInputIndex(unsigned piece, unsigned position, bool whites_perspective) {
    unsigned piece_index = (piece-1)/2; // maps my piece enum to pawn: 0 knight: 1 bishop: 2 rook: 3 queen: 4
    bool is_white_piece = piece % 2 == 1;

    if (!whites_perspective) {
        position = position%8 + 8*(7 - position/8);
    }

    unsigned offset = (is_white_piece == whites_perspective) ? 0 : 6*64;

    return offset + 64*piece_index + position;
}

float relu(float x) {
    return x > 0 ? x : 0.0f;
}

std::array<float, NNUE_INPUT_SIZE> board_to_vector(GameBoard const &board, bool white_to_move) {
    std::array<float, NNUE_INPUT_SIZE> input_activations = {};
    // loop over piece types pawn to queen
    for (unsigned piece_type = 1; piece_type < 11; ++piece_type) {
        // loop over all pieces of that type
        forEachPiece(static_cast<Constants::Piece>(piece_type), board, [&](unsigned position, GameBoard const & game_board) {
            unsigned index = calculateInputIndex(piece_type, position, white_to_move);
            input_activations[index] = 1.0f;
        });
    }
    return input_activations;
}


NNUE_WEIGHTS::NNUE_WEIGHTS() : hidden_layer_1_weights({}), hidden_layer_2_weights({}), hidden_layer_1_biases({}), hidden_layer_2_biases({}), output_weights({}), output_bias({}) {
    try {
        std::ifstream weights_reader("./nnue_weights_float_768net.bin", std::ios::binary);
        if (!weights_reader.is_open()) {
            std::cout << "info string could not open nnue file" << std::endl;
            loaded_ok = false;
            return;
        }
        auto read_array = [&](auto& arr) {
            weights_reader.read(reinterpret_cast<char*>(arr.data()),
                                sizeof(typename std::remove_reference_t<decltype(arr)>::value_type) * arr.size());
        };

        // Unfortunately I need to weights to be transposed for cache efficiency
        std::vector<float> weights_hl1_flat(NNUE_INPUT_SIZE * NNUE_HIDDEN_LAYER_1_SIZE);  // flacher Speicher zum Einlesen
        std::vector<float> weights_hl2_flat(NNUE_HIDDEN_LAYER_1_SIZE * NNUE_HIDDEN_LAYER_2_SIZE);
        std::vector<float> weights_ol_flat(NNUE_HIDDEN_LAYER_2_SIZE * NNUE_OUTPUT_SIZE);


        // Read in order
        read_array(weights_hl1_flat);
        read_array(hidden_layer_1_biases);
        read_array(weights_hl2_flat);
        read_array(hidden_layer_2_biases);
        read_array(weights_ol_flat);
        read_array(output_bias);

        if (weights_reader.good()) {
            loaded_ok = true;
            // initialize weight arrays transposed
            for (int j = 0; j < NNUE_HIDDEN_LAYER_1_SIZE; ++j) {
                for (int k = 0; k < NNUE_INPUT_SIZE; ++k) {
                    hidden_layer_1_weights[k][j] = weights_hl1_flat[j*NNUE_INPUT_SIZE + k];
                }
            }
            for (int j = 0; j < NNUE_HIDDEN_LAYER_2_SIZE; ++j) {
                for (int k = 0; k < NNUE_HIDDEN_LAYER_1_SIZE; ++k) {
                    hidden_layer_2_weights[k][j] =  weights_hl2_flat[j*NNUE_HIDDEN_LAYER_1_SIZE + k];
                }
            }
            for (int j = 0; j < NNUE_OUTPUT_SIZE; ++j) {
                for (int k = 0; k < NNUE_HIDDEN_LAYER_2_SIZE; ++k) {
                    output_weights[k][j] =  weights_ol_flat[j*NNUE_HIDDEN_LAYER_2_SIZE + k];
                }
            }



            std::cout << "info string nnue weights successfully loaded.\n";
        } else {
            std::cerr << "info string error while reading nnue weights file.\n";
            loaded_ok = false;
        }
    } catch (std::exception const & e) {
        std::cerr << "info string exception while loading weights: " << e.what() << "\n";
        loaded_ok = false;
    }
}


void NNUE::reset_to(GameBoard const &board) {
    white_to_move_hidden_layer_inputs = network_weights.hidden_layer_1_biases;
    black_to_move_hidden_layer_inputs = network_weights.hidden_layer_1_biases;

    // loop over piece types pawn to queen
    for (unsigned piece_type = 1; piece_type < 13; ++piece_type) {
        // loop over all pieces of that type
        forEachPiece(static_cast<Constants::Piece>(piece_type), board, [&](unsigned position, GameBoard const & game_board) {
            unsigned white_index = calculateInputIndex(piece_type, position, true);
            unsigned black_index = calculateInputIndex(piece_type, position, false);

            for (size_t j = 0; j < NNUE_HIDDEN_LAYER_1_SIZE; j++) {
                white_to_move_hidden_layer_inputs[j] += network_weights.hidden_layer_1_weights[white_index][j];
                black_to_move_hidden_layer_inputs[j] += network_weights.hidden_layer_1_weights[black_index][j];
            }
        });
    }
}

int NNUE::evaluate(bool white_to_move) {
    auto accumulator = white_to_move ? white_to_move_hidden_layer_inputs : black_to_move_hidden_layer_inputs;
    relu(accumulator);
    auto hl2_result = forward(accumulator,network_weights.hidden_layer_2_weights,network_weights.hidden_layer_2_biases);
    relu(hl2_result);
    auto output_layer_result = forward(hl2_result,network_weights.output_weights,network_weights.output_bias);
    return static_cast<int>(output_layer_result[0]);
}

void NNUE::update(GameBoard const &board, Move move, bool unmake, int en_passant) {
    float sign = unmake ? -1.0f : 1.0f;

    // indices to remove piece from its position
    unsigned white_index = calculateInputIndex(move.piece(),move.from(),true);
    unsigned black_index = calculateInputIndex(move.piece(),move.from(),false);

    for (size_t j = 0; j < NNUE_HIDDEN_LAYER_1_SIZE; j++) {
        white_to_move_hidden_layer_inputs[j] -= sign * network_weights.hidden_layer_1_weights[white_index][j];
        black_to_move_hidden_layer_inputs[j] -= sign * network_weights.hidden_layer_1_weights[black_index][j];
    }

    // indices to put piece on new square
    unsigned piece = move.promotion() ? move.promotion() : move.piece();
    white_index = calculateInputIndex(piece,move.to(),true);
    black_index = calculateInputIndex(piece,move.to(),false);

    for (size_t j = 0; j < NNUE_HIDDEN_LAYER_1_SIZE; j++) {
        white_to_move_hidden_layer_inputs[j] += sign * network_weights.hidden_layer_1_weights[white_index][j];
        black_to_move_hidden_layer_inputs[j] += sign * network_weights.hidden_layer_1_weights[black_index][j];
    }
    // remove captured piece
    if (move.capture()) {
        // offset in en passant case
        int offset = move.to() == en_passant ? (move.capture() == Constants::BLACK_PAWN ? Constants::SOUTH : Constants::NORTH) : 0;
        white_index = calculateInputIndex(move.capture(),move.to()+offset,true);
        black_index = calculateInputIndex(move.capture(),move.to()+offset,false);
        for (size_t j = 0; j < NNUE_HIDDEN_LAYER_1_SIZE; j++) {
            white_to_move_hidden_layer_inputs[j] -= sign * network_weights.hidden_layer_1_weights[white_index][j];
            black_to_move_hidden_layer_inputs[j] -= sign * network_weights.hidden_layer_1_weights[black_index][j];
        }
    }
    // move rook in case of castling
    if (move.castle()) {
        piece = move.piece()%2 == 1 ? Constants::WHITE_ROOK : Constants::BLACK_ROOK;
        unsigned from = counttzll(castle_rook_positions[move.castle()][unmake]);
        unsigned to = counttzll(castle_rook_positions[move.castle()][!unmake]);

        // no unmake sign because the positions from and to are already swapped
        white_index = calculateInputIndex(piece,from,true);
        black_index = calculateInputIndex(piece,from,false);

        for (size_t j = 0; j < NNUE_HIDDEN_LAYER_1_SIZE; j++) {
            white_to_move_hidden_layer_inputs[j] -= network_weights.hidden_layer_1_weights[white_index][j];
            black_to_move_hidden_layer_inputs[j] -= network_weights.hidden_layer_1_weights[black_index][j];
        }

        white_index = calculateInputIndex(piece,to,true);
        black_index = calculateInputIndex(piece,to,false);
        for (size_t j = 0; j < NNUE_HIDDEN_LAYER_1_SIZE; j++) {
            white_to_move_hidden_layer_inputs[j] += network_weights.hidden_layer_1_weights[white_index][j];
            black_to_move_hidden_layer_inputs[j] += network_weights.hidden_layer_1_weights[black_index][j];
        }
    }
}


NNUE::NNUE() : white_to_move_hidden_layer_inputs({}), black_to_move_hidden_layer_inputs({}) {

}






