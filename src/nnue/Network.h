//
// Created by salom on 08.10.2025.
//

#ifndef NETWORK_H
#define NETWORK_H

#include <array>
#include <cstdint>
#include <iostream>

#include "GameBoard.h" // lib game_logic
#include "Move.h"


inline constexpr unsigned NNUE_INPUT_SIZE = 64*12;
inline constexpr unsigned NNUE_HIDDEN_LAYER_1_SIZE = 256;
inline constexpr unsigned NNUE_HIDDEN_LAYER_2_SIZE = 32;
inline constexpr unsigned NNUE_OUTPUT_SIZE = 1;

struct alignas(32) NNUE_WEIGHTS {
    // weights and biases
    alignas(32) std::array<std::array<float,NNUE_HIDDEN_LAYER_1_SIZE>, NNUE_INPUT_SIZE> hidden_layer_1_weights;
    alignas(32) std::array<float, NNUE_HIDDEN_LAYER_1_SIZE> hidden_layer_1_biases;

    alignas(32) std::array<std::array<float,NNUE_HIDDEN_LAYER_2_SIZE>, NNUE_HIDDEN_LAYER_1_SIZE> hidden_layer_2_weights;
    alignas(32) std::array<float, NNUE_HIDDEN_LAYER_2_SIZE> hidden_layer_2_biases;

    alignas(32) std::array<std::array<float,NNUE_OUTPUT_SIZE>, NNUE_HIDDEN_LAYER_2_SIZE> output_weights;
    alignas(32) std::array<float, NNUE_OUTPUT_SIZE> output_bias;

    bool loaded_ok = false;

    NNUE_WEIGHTS();
};

struct alignas(32) NNUE {
    // accumulator input
    alignas(32) std::array<float, NNUE_HIDDEN_LAYER_1_SIZE> white_to_move_hidden_layer_inputs;
    alignas(32) std::array<float, NNUE_HIDDEN_LAYER_1_SIZE> black_to_move_hidden_layer_inputs;
    // constructor
    NNUE();
    // member functions
    template <size_t SIZE_IN,size_t SIZE_OUT>
    std::array<float,SIZE_OUT> forward( std::array<float,SIZE_IN> const & input_activations,
                                                std::array<std::array<float,SIZE_OUT>,SIZE_IN> const & weights,
                                                std::array<float,SIZE_OUT> const & biases);

    void update(GameBoard const & board, Move move, bool unmake, int en_passant);
    void reset_to(GameBoard const & board);
    int evaluate(bool white_to_move);
};

template <size_t SIZE_IN,size_t SIZE_OUT>
std::array<float,SIZE_OUT> NNUE::forward( std::array<float,SIZE_IN> const & input_activations,
                                            std::array<std::array<float,SIZE_OUT>,SIZE_IN> const & weights,
                                            std::array<float,SIZE_OUT> const & biases) {

    std::array<float,SIZE_OUT> output = biases;

    for (size_t k = 0; k < SIZE_IN; k++) {
        for (size_t j = 0; j < SIZE_OUT; j++) {
            output[j] += input_activations[k] * weights[k][j];
        }
    }
    return output;
}


unsigned calculateInputIndex(unsigned piece, unsigned position, bool whites_perspective);
std::array<float,NNUE_INPUT_SIZE> board_to_vector(GameBoard const & board, bool white_to_move);
float relu(float x);
template <size_t ARRAY_SIZE>
void relu(std::array<float,ARRAY_SIZE> & array) {
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        array[i] = relu(array[i]);
    }
}

inline NNUE_WEIGHTS network_weights = NNUE_WEIGHTS();
inline NNUE nnue = NNUE();

#endif //NETWORK_H
