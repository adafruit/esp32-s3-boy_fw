/*
 * Copyright (c) 2012 Dave Vasilevsky <dave@vasilevsky.ca>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "squash/decompress.h"
#include "squash/squashfs_fs.h"

#include "lz4.h"
#include <assert.h>
#include <stdio.h>
#define SQFS_COMP_LZ4 5

static sqfs_err sqfs_decompressor_lz4(void *in, size_t insz,
		void *out, size_t *outsz) {
        printf("decompress_safe(%p, %zd, %p, %zd)\n", in, insz, out, *outsz);
	int zout = LZ4_decompress_safe(in, out, (int)insz, (int)*outsz);
        printf("->%d", zout);
	if (zout < 0)
		return SQFS_ERR;
	*outsz = zout;
	return SQFS_OK;
}

sqfs_decompressor sqfs_decompressor_get(sqfs_compression_type type) {
        printf("decompressor_get type=%d\n", type);
	assert(SQFS_COMP_LZ4 == type);
	return &sqfs_decompressor_lz4;
}
