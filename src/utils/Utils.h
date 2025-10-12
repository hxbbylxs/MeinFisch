//
// Created by salom on 12.10.2025.
//

#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <cassert>

// returns the number of bits set to 1 in a 64 bit unsigned integer
inline int popcountll(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(x);
#elif defined(_MSC_VER)
#include <intrin.h>
    return static_cast<int>(__popcnt64(x));
#else
    int count = 0;
    while (x) {
        x &= (x - 1);
        ++count;
    }
    return count;
#endif
}

// returns the position of the least significant bit
// UB if input = 0
inline int counttzll(uint64_t x) {
    assert(x != 0);
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(x);
#elif defined(_MSC_VER)
#include <intrin.h>
    unsigned long index;
    _BitScanForward64(&index, x);
    return static_cast<int>(index);
#else
    // Fallback
    int count = 0;
    while ((x & 1) == 0 && count < 64) {
        x >>= 1;
        ++count;
    }
    return count;
#endif
}



#endif //UTILS_H
