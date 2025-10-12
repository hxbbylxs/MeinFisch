//
// Created by salom on 16.07.2025.
//
#include "MoveGenerationConstants.h"
#include <random>
#include <cstdint>
#include <iostream>
#include <unordered_map>

#include "Utils.h"

using Constants::BOARD_SIZE;
using Constants::NUM_SQUARES;

// these functions are for initialization of constants


std::array<uint64_t,MAX_NUM_BLOCKER_COMBINATIONS> generateSliderBlockerCombos(int index, bool diagonalSlider) {

    uint64_t const blockerBitMask = diagonalSlider ? diagonalSliderBlockerBitMasks[index] : nonDiagonalSliderBlockerBitMasks[index];
    uint64_t const numberOfCombinations =  1ULL << popcountll(blockerBitMask);
    std::vector<int> const positionIndices = bitmaskToIndices(blockerBitMask);
    std::array<uint64_t,MAX_NUM_BLOCKER_COMBINATIONS> blockerCombinations = {};

    uint64_t currentCombination = 0;
    for (int i = 0; i < numberOfCombinations; i++) {
        blockerCombinations[i] = currentCombination;
        currentCombination = nextCombination(currentCombination,positionIndices);
    }
    return blockerCombinations;
}

// Generates the next subset of bits at the given positions
uint64_t nextCombination(uint64_t blockerBitMask, std::vector<int> const & positions) {
    for (int position : positions) {
        blockerBitMask ^= (1ULL << position);
        if (blockerBitMask & (1ULL << position)) return blockerBitMask;
    }
    return blockerBitMask;
}

std::vector<int> bitmaskToIndices(uint64_t bitmask) {
    std::vector<int> indices;
    while (bitmask != 0) {
        indices.push_back(counttzll(bitmask));
        bitmask &= bitmask-1;
    }
    return indices;
}

uint64_t generateSolutionToBlockerCombinationForSliders(uint64_t blockerCombination, int index, bool diagonalSlider) {
    uint64_t solution = 0;
    auto directions = diagonalSlider ? DIAGONAL_DIRECTIONS : NON_DIAGONAL_DIRECTIONS;

    //Sliding from square = index in each direction until blocker or one before edge of the board
    for (auto const & [delta,yLimit,xLimit] : directions) {
        int square = index;
        while (square/BOARD_SIZE != yLimit && square%BOARD_SIZE != xLimit) {
            square += delta;
            solution |= (1ULL << square);
            if (blockerCombination & (1ULL << square)) break;
        }
    }

    return solution;
}


std::array<uint64_t,NUM_SQUARES> findMagicNumbers(bool diagonalSlider) {
    std::array<uint64_t,NUM_SQUARES> magicNumbers = {};
    for (int board_pos = 0; board_pos < NUM_SQUARES; board_pos++) {
        std::array<uint64_t,MAX_NUM_BLOCKER_COMBINATIONS> blockerCombinations = generateSliderBlockerCombos(board_pos,diagonalSlider);
        int shift = NUM_SQUARES - popcountll(diagonalSlider ? diagonalSliderBlockerBitMasks[board_pos]:nonDiagonalSliderBlockerBitMasks[board_pos]);
        while (true) {
            uint64_t magicNumberCandidate = generateRandomMagicNumberCandidate(shift);
            if (checkMagicNumberCandidate(shift,blockerCombinations,board_pos,diagonalSlider,magicNumberCandidate)) {
                magicNumbers[board_pos] = magicNumberCandidate;
                printProgress(board_pos,diagonalSlider);
                break;
            }
        }
    }
    return magicNumbers;
}

bool checkMagicNumberCandidate(int shift,std::array<uint64_t,MAX_NUM_BLOCKER_COMBINATIONS> const & blockerCombinations, int board_pos, bool diagonalSlider,uint64_t magicNumberCandidate) {
    bool isValidMagicNumber = true;
    std::unordered_map<int,uint64_t> usedPositions = {std::make_pair(0,generateSolutionToBlockerCombinationForSliders(0,board_pos,diagonalSlider))};
    for (uint64_t blockerCombination : blockerCombinations) {
        if (blockerCombination == 0) continue;
        uint64_t index = (blockerCombination * magicNumberCandidate >> shift);
        if (index >= MAX_NUM_BLOCKER_COMBINATIONS) {
            isValidMagicNumber = false;
            break;
        }
        auto it = usedPositions.find(static_cast<int>(index));
        if (it != usedPositions.end() && it->second != generateSolutionToBlockerCombinationForSliders(blockerCombination,board_pos,diagonalSlider)) {
            isValidMagicNumber = false;
            break;
        }
        usedPositions.insert(std::make_pair(static_cast<int>(index), generateSolutionToBlockerCombinationForSliders(blockerCombination,board_pos,diagonalSlider)));
    }
    return isValidMagicNumber;
}

uint64_t generateRandomMagicNumberCandidate(int shift) {
        static std::random_device rd;  // Seed
        static std::mt19937_64 gen(rd());  // 64-bit Mersenne Twister Generator
        static std::uniform_int_distribution<uint64_t> dis(1ULL << shift, UINT64_MAX);
        uint64_t magicNumber = dis(gen) & dis(gen) & dis(gen);
        return magicNumber;
}

void initializeSliderAttackBitMask() {
    for (int board_pos = 0; board_pos < NUM_SQUARES; board_pos++) {
        setSliderAttackBitMasks(false,board_pos);
        setSliderAttackBitMasks(true,board_pos);
    }
}

void setSliderAttackBitMasks(bool diagonalSlider, int board_pos) {
    auto const & blockerBitMasks = diagonalSlider ? diagonalSliderBlockerBitMasks : nonDiagonalSliderBlockerBitMasks;
    auto const & magicNumbers = diagonalSlider ? magicNumbersForDiagonalSliders : magicNumbersForNonDiagonalSliders;
    if (magicNumbers[1] == 0) { // magic Numbers empty
        std::cout << "Error: Called setSliderAttackBitMasks but magicNumbers are not initialized" << std::endl;
        return;
    }
    auto & attackBitMasks = diagonalSlider ? diagonalSlidersAttackBitMask : nonDiagonalSlidersAttackBitMask;

    int shift = NUM_SQUARES - popcountll(blockerBitMasks[board_pos]);
    std::array<uint64_t,MAX_NUM_BLOCKER_COMBINATIONS> allPossibleBlockerCombinations = generateSliderBlockerCombos(board_pos,diagonalSlider);
    for (uint64_t blockerCombination : allPossibleBlockerCombinations) {
        uint64_t j = blockerCombination * magicNumbers[board_pos] >> shift;
        attackBitMasks[board_pos][j] = generateSolutionToBlockerCombinationForSliders(blockerCombination,board_pos,diagonalSlider);
    }
}

void printProgress(int board_pos, bool diagonalSliders) {
    std::cout << static_cast<int>(((diagonalSliders? 64:0)+board_pos+1)/1.28) << "%" <<  std::endl;
}
