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

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t rotl64 ( uint64_t v, int8_t n )
{
    return (v << n) | (v >> (64 - n));
}

#define ROTL64(x,y)	rotl64(x,y)
#define BIG_CONSTANT(x) (x##LLU)

uint64_t getblock64 ( const uint64_t * p, int i )
{
    return p[i];
}

uint64_t fmix64 ( uint64_t k )
{
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xff51afd7ed558ccd);
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    k ^= k >> 33;
    
    return k;
}

void MurmurHash3_x64_128 (uint8_t *data, int offset, const int len, const uint32_t seed, void *out)
{
    int i;
    const int nblocks = len >> 4;
    
    uint64_t h1 = seed;
    uint64_t h2 = seed;
    
    const uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
    const uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);
    
    //----------
    // body

    const uint64_t * blocks = (const uint64_t *)(data);
    
    for(i = 0; i < nblocks; i++)
    {
        uint64_t k1 = getblock64(blocks,i*2+0);
        uint64_t k2 = getblock64(blocks,i*2+1);
        
        k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
        
        h1 = ROTL64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;
        
        k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;
        
        h2 = ROTL64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
    }
    
    //----------
    // tail

    offset += nblocks * 16;
    
    uint64_t k1 = 0;
    uint64_t k2 = 0;
    
    switch(len & 15)
    {
        case 15: k2 ^= ((uint64_t)(data)[offset+14]) << 48;
        case 14: k2 ^= ((uint64_t)(data)[offset+13]) << 40;
        case 13: k2 ^= ((uint64_t)(data)[offset+12]) << 32;
        case 12: k2 ^= ((uint64_t)(data)[offset+11]) << 24;
        case 11: k2 ^= ((uint64_t)(data)[offset+10]) << 16;
        case 10: k2 ^= ((uint64_t)(data)[offset+9]) << 8;
        case  9: k2 ^= ((uint64_t)(data)[offset+8]) << 0;
            k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;
            
        case  8: k1 ^= ((uint64_t)(data)[offset+7]) << 56;
        case  7: k1 ^= ((uint64_t)(data)[offset+6]) << 48;
        case  6: k1 ^= ((uint64_t)(data)[offset+5]) << 40;
        case  5: k1 ^= ((uint64_t)(data)[offset+4]) << 32;
        case  4: k1 ^= ((uint64_t)(data)[offset+3]) << 24;
        case  3: k1 ^= ((uint64_t)(data)[offset+2]) << 16;
        case  2: k1 ^= ((uint64_t)(data)[offset+1]) <<  8;
        case  1: k1 ^= ((uint64_t)(data)[offset+0]) <<  0;
            k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
    };

    //--------------
    // finalization
    
    h1 ^= len; h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;

    ((uint64_t*)out)[0] = h1;
    ((uint64_t*)out)[1] = h2;
}

#ifdef __cplusplus
}
#endif
