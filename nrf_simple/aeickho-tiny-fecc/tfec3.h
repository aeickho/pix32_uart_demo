/*
 * Copyright 2012 Sebastian Gesemann. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY SEBASTIAN GESEMANN ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SEBASTIAN GESEMANN OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
 
/*
 * This is an implementation of a simple "erasure code" one can use for
 * forward error correction. The code can take n data blocks (n<=15) and
 * produce k recovery blocks (k<=3, named P for parity, Q and R). The block
 * size is a user-selectable multiple of 32 bits. But for all blocks in a
 * coding group the block size is the same. If for some reason k or less
 * blocks (including recovery blocks) are lost, tiny-fecc will be able to
 * recover all missing data blocks.
 * 
 * The mathematics is based on Linux' Raid-6 algorithm[1] which works for the
 * same reason why Reed-Solomon codes work. I changed the Galois field to be
 * smaller (to speed up multiplication and reduce the lookup table's size)
 * and added a third kind of recovery block called R. The irreducible
 * polynomial of degree 16 I used is x^16+x+1. It results in a field where
 * x (0x02) is a generator of the multiplicative group. The recovery block
 * R is computed just like Q with the order of data blocks reversed. Thus,
 * the fast encoding trick for Q described in the paper by Peter Anvin is
 * also applicable to R. P is just the parity, so it's already very fast.
 *
 * The tiny in "tiny-fecc" is supposed to reflect the size of the Galois field
 * and its consequences: a small lookup table (only 8 bytes) for reciprocals
 * and the limitation of 15 data blocks per coding group.
 *
 * [1] http://kernel.org/pub/linux/kernel/people/hpa/raid6.pdf
 */

#ifndef TINY_FECC_TFEC3_H_INCLUDED
#define TINY_FECC_TFEC3_H_INCLUDED

#include <limits.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t tfec3_u32;


/** 
 * Forward Error Correction encoding
 *
 * Takes n blocks of data (1<=n<=15) and
 * computes k blocks of redundancy (1<=k<=3).
 *
 * @param words
 *   number of words per block
 * @param n
 *   number of data blocks (1...15)
 * @param k
 *   number of recovery blocks to compute (1...3)
 * @param io
 *   array of n+k block pointers. The first n blocks are left untouched
 *   but they are used to compute the remaining k recovery blocks.
 *   Some of the final k pointers may be null. This way you can avoid
 *   computing some recovery blocks.
 */
extern void tfec3_encode(int words, int n, int k, tfec3_u32 *io[]);

/**
 * Forward Error Correction decoding
 *
 * Recovers up to k of n data blocks that are missing using
 * k of the recovery blocks.
 *
 * Although this is called "error correction", we don't "correct errors" but
 * recover missing blocks. It is up to you to detect missing/damaged blocks
 * and signal this via the parameter 'valid'.
 *
 * @param words
 *   number of words per block
 * @param n
 *   number of data blocks (1...15)
 * @param k
 *   number of recovery blocks (1...3)
 * @param valid
 *   array of n+k chars that specify which block is valid (1)
 *   and which is not (0).
 * @param io
 *   array of n+k block pointers. The last k blocks are left untouched
 *   but they (if available) are used to recover missing blocks.
 * @return
 *   returns 1 on success, 0 on failure. If more than k blocks
 *   (including recovery blocks) are missing, missing data blocks
 *   cannot be recoverd. Otherwise, recovery will succeed.
 */
extern int tfec3_decode(int words, int n, int k,
	const unsigned char valid[], tfec3_u32 *io[]);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
