/*
 * The MIT License
 *
 * Copyright (c) 2015 - 2016 Amir Tuval, Hideaki Ohno, J. Renero
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to
 * deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom
 * the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#if !defined(HYPERLOGLOG128)
#define HYPERLOGLOG128

#include <vector>
#include <cmath>
#include <sstream>
#include <stdio.h>
#include <string.h>

#include "murmurhash3.h"
#include "hyperloglog128.h"

/**
 * Constructor
 * @param[in] b bit width (register size will be 2 to the b power).
 *            This value must be in the range[4,16].
 * @exception std::invalid_argument the argument is out of range.
 */
HyperLogLog128::HyperLogLog128(uint8_t b, bool legacyMode) throw(std::invalid_argument) :
        b_(b), m_((uint32_t) (1 << b)), M_(m_, 0), legacyMode_(legacyMode) {

    if (b < 4 || 16 < b) {
        throw std::invalid_argument("bit width must be in the range [4,16]");
    }

    switch (m_) {
        case 16:
            alpha_ = 0.673;
            break;
        case 32:
            alpha_ = 0.697;
            break;
        case 64:
            alpha_ = 0.709;
            break;
        default:
            alpha_ = 0.7213 / (1.0 + 1.079 / m_);
            break;
    }
    alphaMM_ = alpha_ * m_ * m_;
}

/**
 * Adds element to the estimator
 *
 * @param[in] str string to add
 * @param[in] len length of string
 */
void HyperLogLog128::add(const char *str, uint32_t len) {
    uint8_t uint8_tBuffer[len];
    long pairLongs[2];
    uint8_t hash[LONGSIZE];

    memset(uint8_tBuffer, 0, len);
    for(int i=0;i<strlen(str);i++)
        uint8_tBuffer[i] = static_cast<unsigned char>(str[i]);

    MurmurHash3_x64_128(str, 0, len, HLL_HASH_SEED, (void *) &pairLongs);
    pairLong2uint8_ts((uint8_t *) &pairLongs[0], (uint8_t *) &pairLongs[1], &hash[0]);

    int index = j((uint8_t *) hash, b_);
    uint8_t rho = rhoW(hash, b_);
    if (M_[index] < rho) {
        M_[index] = rho;
    }
}


/**
 * Estimates cardinality value.
 * @return Estimated cardinality value.
 */
double HyperLogLog128::estimate(void) const {
    double E, Z, z = 0.0;

    /* Multiply the inverse of E for alpha_m * m^2 to have the raw estimate. */
    int zeroCount = 0;
    for (int i = 0; i < m_; i++) {
        z += 1.0 / pow(2.0, M_[i]);
        zeroCount += (M_[i] == 0);
    }
    Z = 1 / (zeroCount + z);
    E = alphaMM_ * Z;
    double smallE = 5 * (double) m_ / 2.0;

    /*
     I apply tha same correction that Algebird's implementation does.
     */
    if (E <= smallE) {
        if (zeroCount == 0) {
            return (E);
        } else {
            return (m_ * log((double) m_ / (double) zeroCount));
        }
    } else {
        return E;
    }
}

/**
 * Merges the estimate from 'other' into this object, returning the estimate of their union.
 * The number of registers in each must be the same.
 *
 * @param[in] other HyperLogLog instance to be merged
 *
 * @exception std::invalid_argument number of registers doesn't match.
 */
void HyperLogLog128::merge(const HyperLogLog128 &other) throw(std::invalid_argument) {
    if (m_ != other.m_) {
        std::stringstream ss;
        ss << "number of registers doesn't match: " << m_ << " != " << other.m_;
        throw std::invalid_argument(ss.str().c_str());
    }

    uint8_t * myArr = &M_[0];
    const uint8_t *otherArr = &other.M_[0];
    for (uint32_t r = 0; r < m_; ++r) {
        if (myArr[r] < otherArr[r]) {
            myArr[r] = otherArr[r];
        }
    }
}

/**
 * Clears all internal registers.
 */
void HyperLogLog128::clear(void) {
    std::fill(M_.begin(), M_.end(), 0);
}

/**
 * Returns size of register.
 * @return Register size
 */
uint32_t HyperLogLog128::registerSize() const {
    return m_;
}

/**
 * the value 'j' is equal to <w_0, w_1 ... w_(bits-1)>
 */
int HyperLogLog128::jLoop(int pos, int accum, uint8_t *bsl, int bits) {
    if (pos >= bits) {
        return (accum);
    } else if (bitsetContains(bsl, pos)) {
        return (jLoop(pos + 1, accum + (1 << pos), bsl, bits));
    } else {
        return (jLoop(pos + 1, accum, bsl, bits));
    }
}

int HyperLogLog128::j(uint8_t *bsl, int bits) {
    return (jLoop(0, 0, bsl, bits));
}


uint8_t HyperLogLog128::rhoLoop(int pos, uint8_t zeros, uint8_t *bsl, int bits) {
    if (bitsetContains(bsl, pos)) return (zeros);
    else return (rhoLoop(pos + 1, (uint8_t) (zeros + 1), bsl, bits));
}

uint8_t HyperLogLog128::rhoW(uint8_t *bsl, int bits) {
    return (rhoLoop(bits, 1, bsl, bits));
}

void HyperLogLog128::extract_uint64_be(const long value, uint8_t *buffer) {
    memset(buffer, 0, LONGSIZE);
    uint8_t mask = (uint8_t) 255;
    buffer[7] = (uint8_t) (mask & ((value << (uint8_t) 56) >> (uint8_t) 56));
    buffer[6] = (uint8_t) (mask & ((value << (uint8_t) 48) >> (uint8_t) 56));
    buffer[5] = (uint8_t) (mask & ((value << (uint8_t) 40) >> (uint8_t) 56));
    buffer[4] = (uint8_t) (mask & ((value << (uint8_t) 32) >> (uint8_t) 56));
    buffer[3] = (uint8_t) (mask & ((value << (uint8_t) 24) >> (uint8_t) 56));
    buffer[2] = (uint8_t) (mask & ((value << (uint8_t) 16) >> (uint8_t) 56));
    buffer[1] = (uint8_t) (mask & ((value << (uint8_t) 8) >> (uint8_t) 56));
    buffer[0] = (uint8_t) (mask & (value >> (uint8_t) 56));
}

void HyperLogLog128::pairLong2uint8_ts(uint8_t *l0, uint8_t *l1, uint8_t *buffer) {
    for (int i = 0; i < LONGSIZE; i++) {
        buffer[i] = (l0)[LONGSIZE - i - 1];
    }
    for (int i = 0; i < LONGSIZE; i++) {
        buffer[i + LONGSIZE] = (l1)[LONGSIZE - i - 1];
    }
}

/**
 * Pretend 'in' is little endian so that the bitstring b0b1b2b3 is such that if b0 == 1, then
 *  0 is in the bitset, if b1 == 1, then 1 is in the bitset.
 */
bool HyperLogLog128::bitsetContains(uint8_t *in, int x) {
    int arrayIdx = x / 8;
    int remainder = x % 8;
    return (((in[arrayIdx] >> (7 - remainder)) & 1) == 1);
}

#endif
