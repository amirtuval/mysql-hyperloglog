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

#if !defined(SERIALIZED_HYPERLOGLOG_HPP)
#define SERIALIZED_HYPERLOGLOG_HPP

#include "serializedhyperloglog.h"
#include "base64.h"

#endif

SerializedHyperLogLog::SerializedHyperLogLog(uint8_t b, bool legacyMode = true) : HyperLogLog128(b, legacyMode) { }

char *SerializedHyperLogLog::toString(char *result) {
    sprintf(result, "%d|%d|bin_", (legacyMode_ ? 1 : 0), b_);
    memcpy(&result[strlen(result)], &M_[0], M_.size());
    return result;
}

int SerializedHyperLogLog::stringLength() {
    return (int) (10 + M_.size());
}

SerializedHyperLogLog *SerializedHyperLogLog::fromString(const char *encoded) {
    if (encoded == NULL) return NULL;
    const char *firstSep = (const char *) memchr(encoded, '|', 4);
    if (firstSep == NULL) return NULL;

    int m;
    const char *data;
    bool legacyMode;

    const char *secondSep = (const char *) memchr(&firstSep[1], '|', 4);
    if (secondSep == NULL) { // check if string has 2 '|'
        sscanf(encoded, "%d|", &m);
        data = &firstSep[1];
        legacyMode = true;
    } else {
        int legacyModeInt;
        sscanf(encoded, "%d|%d|", &legacyModeInt, &m);
        legacyMode = legacyModeInt != 0;
        data = &secondSep[1];
    }

    SerializedHyperLogLog *result = new SerializedHyperLogLog((uint8_t) m, legacyMode);

    const unsigned char *decoded;
    unsigned char output[8192];
    bool freeDecoded;
    if (strncmp(data, "bin_", 4) == 0) {
        decoded = (const unsigned char *) &data[4];
        freeDecoded = false;
    } else {
        size_t outputLength;
        decoded = base64_decode(data, strlen(data), &outputLength);
        freeDecoded = true;
    }

    if (decoded == NULL) return NULL;

    memcpy(&result->M_[0], decoded, result->M_.size());
    if (freeDecoded)
        free((void *) decoded);

    return result;
}