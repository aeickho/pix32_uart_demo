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

#include <assert.h>
#include "tfec3.h"

/*
 *  Generator polynomial: x^5 + x^1 + 1 (0x13)
 *  x^1 (2) generates all invertible elements.
 */

static const unsigned char inv_table[] = {
	0x01, 0x9E, 0xDB, 0x76, 0xF2, 0xC5, 0xA4, 0x38
};

static unsigned char inv(unsigned char x)
{
	unsigned char t;
	x &= 0xF;
	t = inv_table[x >> 1];
	return (x&1) ? t & 0xF : t >> 4;
}

static tfec3_u32 twice(tfec3_u32 vec)
{
	return ((vec & 0x77777777u) << 1) ^
		((0x88888888u - ((vec & 0x88888888u) >> 3)) & 0x33333333u);
}

static tfec3_u32 scale(unsigned char a, tfec3_u32 vec)
{
	int i;
	tfec3_u32 acc = 0;
	for (i=0; i<3; ++i) {
	acc ^= (-(tfec3_u32)(a & 1)) & vec; vec = twice(vec); a >>= 1;
	}
	acc ^= (-(tfec3_u32)(a & 1)) & vec;
	return acc;
}

static void block_blank(int words, tfec3_u32 x[])
{
	int i;
	for (i=0; i<words; ++i)
		x[i] = 0;
}

static void block_scale_by_2(int words, tfec3_u32 x[])
{
	int i;
	for (i=0; i<words; ++i)
		x[i] = twice(x[i]);
}

static void block_scale_by_a(int words, unsigned char a, tfec3_u32 x[])
{
	int i;
	if (a==1) return; /* nothing to do */
	for (i=0; i<words; ++i)
		x[i] = scale(a,x[i]);
}

static void block_xpy(int words, const tfec3_u32 x[], tfec3_u32 y[])
{
	int i;
	for (i=0; i<words; ++i)
		y[i] ^= x[i];
}

static void block_axpy(int words, unsigned char a, const tfec3_u32 x[], tfec3_u32 y[])
{
	int i;
	if (a==0) return; /* nothing to do */
	if (a==1) { block_xpy(words,x,y); return; }
	for (i=0; i<words; ++i)
		y[i] ^= scale(a,x[i]);
}

#if 0
static void block_xp2y(int words, const tfec3_u32 x[], tfec3_u32 y[])
{
	int i;
	for (i=0; i<words; ++i)
		y[i] = twice(y[i]) ^ x[i];
}
#else
#define block_xp2y(w,x,y) do {      \
		block_scale_by_2((w),(y));  \
		block_xpy((w),(x),(y));     \
	} while(0)
#endif

static int find_index(int of, const int inlist[])
{
	int i;
	for (i=0; ; ++i)
		if (inlist[i]==of) return i;
}

static void invert_inplace(int order, unsigned char *mat[], int words, tfec3_u32 *blocks[])
{
	int o, c, r, rr;
	unsigned char tmp;
	for (o=0; o<order; ++o) {
		if (mat[o][o]!=0) {
			tmp = inv(mat[o][o]);
			block_scale_by_a(words,tmp,blocks[o]);
			for (c=o; c<order; ++c)
				mat[o][c] = scale(tmp,mat[o][c]);
		} else {
			rr=0;
			for (r=o+1; r<order; ++r)
				if (mat[r][o]) {rr=r; break;}
			assert(rr!=0);
			tmp = inv(mat[rr][o]);
			block_axpy(words,tmp,blocks[rr],blocks[o]);
			for (c=o; c<order; ++c)
				mat[o][c] ^= scale(tmp,mat[rr][c]);
		}
		assert(mat[o][o]==1);
		for (r=0; r<order; ++r)
			if (r!=o && mat[r][o]) {
				tmp = mat[r][o];
				block_axpy(words,tmp,blocks[o],blocks[r]);
				for (c=o; c<order; ++c)
					mat[r][c] ^= scale(tmp,mat[o][c]);
			}
	}
}

void tfec3_encode(int words, int blocks, int redundancy, tfec3_u32 *io[])
{
	int b;
	tfec3_u32 *p, *q, *r;
	assert(blocks<=15);
	assert(redundancy<=3);
	p = redundancy>=1 ? io[blocks  ] : 0;
	q = redundancy>=2 ? io[blocks+1] : 0;
	r = redundancy>=3 ? io[blocks+2] : 0;
	if (p) block_blank(words,p);
	if (r) block_blank(words,r);
	for (b=0; b<blocks; ++b) {
		if (p) block_xpy (words,io[b],p);
		if (r) block_xp2y(words,io[b],r);
	}
	if (q) {
		block_blank(words,q);
		for (b=blocks; b-->0; ) {
			block_xp2y(words,io[b],q);
		}
	}
}

int tfec3_decode(int words, int blocks, int redundancy,
	const unsigned char valid[], tfec3_u32 *io[])
{
	int b, c;
	int misslist[3] = {0}; /* list of missing data blocks */
	int missctr = 0;       /* number of missing data blocks */
	int order = 0;         /* order of equation system that needs to be solved */
	tfec3_u32 *partial_pqr[3] = {0};
	unsigned char matrix[3][3] = {{0}};
	unsigned char *mat[3] = {0};

	assert(blocks<=15);
	assert(redundancy<=3);

	/* check which data blocks are missing... */
	for (b=0; b<blocks; ++b) {
		if (!valid[b]) {
			if (missctr==redundancy) return 0; /* already too many missing */
			misslist[missctr++] = b;
		}
	}
	if (missctr==0) return 1; /* nothing to do */

	/* check which redundancy blocks we can use for recovery... */
	for (b=0; b<redundancy && order<missctr; ++b) {
		if (valid[blocks+b]) {
			partial_pqr[b] = io[misslist[order]];
			mat[order] = matrix[b];
			++order;
		}
	}
	if (order<missctr) return 0; /* too many missing */

	/* compute partial P, Q, R (excluding missing data blocks)
	 * in place of missing blocks and keep track of how the
	 * missing blocks would have contributed to P, Q, R in matrix...
	 */
	for (b=0; b<3; ++b)
		if (partial_pqr[b]) block_blank(words,partial_pqr[b]);
	if (partial_pqr[0] || partial_pqr[2])
		for (b=0; b<blocks; ++b) {
			for (c=0; c<missctr; ++c) matrix[2][c] = twice(matrix[2][c]);
			if (valid[b]) {
				if (partial_pqr[0]) block_xpy (words,io[b],partial_pqr[0]);
				if (partial_pqr[2]) block_xp2y(words,io[b],partial_pqr[2]);
			} else {
				c = find_index(b,misslist);
				matrix[0][c] ^= 1;
				matrix[2][c] ^= 1;
				if (partial_pqr[2]) block_scale_by_2(words,partial_pqr[2]);
			}
		}
	if (partial_pqr[1])
		for (b=blocks; b-->0; ) {
			for (c=0; c<missctr; ++c) matrix[1][c] = twice(matrix[1][c]);
			if (valid[b]) {
				block_xp2y(words,io[b],partial_pqr[1]);
			} else {
				c = find_index(b,misslist);
				matrix[1][c] = 1;
				block_scale_by_2(words,partial_pqr[1]);
			}
		}

	/* compute difference between partial P/Q/R and received P/Q/R */
	for (b=0; b<3; ++b)
		if (partial_pqr[b])
			block_xpy(words,io[blocks+b],partial_pqr[b]);

	/* strip null pointers from partial_pqr array */
	for (b=0, c=0; b<order; ++b) {
		while (!partial_pqr[c]) ++c;
		partial_pqr[b] = partial_pqr[c++];
	}
	invert_inplace(order,mat,words,partial_pqr);
	return 1;
}
