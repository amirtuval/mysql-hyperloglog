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

#ifndef MYSQLHLL128_HYPERLOGLOG128_H
#define MYSQLHLL128_HYPERLOGLOG128_H

#include <vector>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include <stdint.h>

#define HLL_HASH_SEED 12345678L
#define LONGSIZE 8

class HyperLogLog128 {
    public:
        HyperLogLog128(uint8_t b, bool legacyMode = true) throw(std::invalid_argument);
        void add(const char *str, uint32_t len);
        double estimate(void) const;
        void merge(const HyperLogLog128 &other) throw(std::invalid_argument);
        void clear(void);
        uint32_t registerSize(void) const;

    protected:
        uint8_t b_; ///< register bit width
        uint32_t m_; ///< register size
        std::vector<uint8_t> M_; ///< registers
        bool legacyMode_;

    private:
        double alpha_;
        double alphaMM_; ///< alpha * m^2
        bool bitsetContains(uint8_t *in, int x);
        int jLoop(int pos, int accum, uint8_t *bsl, int bits);
        int j(uint8_t *bsl, int bits);
        void pairLong2uint8_ts(uint8_t *l0, uint8_t *l1, uint8_t *buffer);
        void extract_uint64_be(const long value, uint8_t *buffer);
        uint8_t rhoW(uint8_t *bsl, int bits);
        uint8_t rhoLoop(int pos, uint8_t zeros, uint8_t *bsl, int bits);
};

#endif // MYSQLHLL128_HYPERLOGLOG128_H
