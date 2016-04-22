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

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hyperloglog128.h"

using namespace std;

int main(int argc, char *argv[])
{
    int i;
    if(argc < 2) {
        printf("Usage: %s <string>\n", argv[0]);
        exit(1);
    }

    char value[1024], byteArray[1024];
    for(int i=1;i<argc;i++) {
        strcat(value, argv[i]);
        if (i<argc-1) strcat(value, " ");
    }
    printf("HLL(%s)\n", value);

    HyperLogLog128 e = HyperLogLog128(12, false);
    e.add(value, (uint32_t) strlen(value));
    for(i=0;i<20000;i++) {
        char str[15];
        sprintf(str, "%d", i);
        e.add(str, (uint32_t) strlen(str));
    }

    long long c = (long long)e.estimate();
    printf("Estimated Cardinality = %lld\n", c);
}
