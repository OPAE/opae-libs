// Copyright(c) 2021, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#define _GNU_SOURCE
#include <linux/limits.h>
#include <errno.h>
#include <glob.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <uuid/uuid.h>
#include <sys/stat.h>

#undef _GNU_SOURCE
#include <opae/fpga.h>

#include "props.h"
#include "opae_uio.h"
/* #include "dfl.h" */

void free_token_list(uio_token *tokens)
{
	while (tokens) {
		uio_token *t = tokens;

		tokens = tokens->next;
		free(t);
	}
}

uio_token *clone_token(uio_token *src)
{
	ASSERT_NOT_NULL_RESULT(src, NULL);
	if (src->magic != UIO_TOKEN_MAGIC)
		return NULL;
	uio_token *token = (uio_token *)malloc(sizeof(uio_token));

	if (!token) {
		OPAE_ERR("Failed to allocate memory for uio_token");
		return NULL;
	}
	memcpy(token, src, sizeof(uio_token));
	if (src->parent)
		token->parent = clone_token(src->parent);
	return token;
}

uio_token *token_check(fpga_token token)
{
	ASSERT_NOT_NULL_RESULT(token, NULL);
	uio_token *t = (uio_token *)token;

	if (t->magic != UIO_TOKEN_MAGIC) {
		OPAE_ERR("invalid token magic");
		return NULL;
	}
	return t;
}

uio_handle *handle_check(fpga_handle handle)
{
	ASSERT_NOT_NULL_RESULT(handle, NULL);
	uio_handle *h = (uio_handle *)handle;

	if (h->magic != UIO_HANDLE_MAGIC) {
		OPAE_ERR("invalid handle magic");
		return NULL;
	}
	return h;
}

uio_handle *handle_check_and_lock(fpga_handle handle)
{
	uio_handle *h = handle_check(handle);

	if (h && pthread_mutex_lock(&h->lock)) {
		OPAE_MSG("failed to lock handle mutex");
		return NULL;
	}
	return h;
}

uio_token *find_token(const pci_device_t *p, uint32_t region)
{
	uio_token *t = p->tokens;

	while (t) {
		if (t->region == region)
			return t;
		t = t->next;
	}
	return t;
}

uio_token *get_token(pci_device_t *p, uint32_t region, int type)
{
	uio_token *t = find_token(p, region);

	if (t)
		return t;
	t = (uio_token *)malloc(sizeof(uio_token));
	if (!t) {
		OPAE_ERR("Failed to allocate memory for uio_token");
		return NULL;
	}
	memset(t, 0, sizeof(uio_token));
	t->magic = UIO_TOKEN_MAGIC;
	t->device = p;
	t->region = region;
	t->type = type;
	t->next = p->tokens;
	p->tokens = t;
	return t;
}

fpga_result __UIO_API__ uio_fpgaCloneToken(fpga_token src, fpga_token *dst)
{
	UNUSED_PARAM(src);
	UNUSED_PARAM(dst);
#if 0
	uio_token *_src = (uio_token *)src;
	uio_token *_dst;

	if (!src || !dst) {
		OPAE_ERR("src or dst is NULL");
		return FPGA_INVALID_PARAM;
	}
	if (_src->magic != VFIO_TOKEN_MAGIC) {
		OPAE_ERR("Invalid src");
		return FPGA_INVALID_PARAM;
	}

	_dst = malloc(sizeof(uio_token));
	if (!_dst) {
		OPAE_ERR("Failed to allocate memory for uio_token");
		return FPGA_NO_MEMORY;
	}
	memcpy(_dst, _src, sizeof(uio_token));
	*dst = _dst;
#endif
	return FPGA_OK;
}

fpga_result __UIO_API__ uio_fpgaDestroyToken(fpga_token *token)
{
	UNUSED_PARAM(token);
#if 0
	if (!token || !*token) {
		OPAE_ERR("invalid token pointer");
		return FPGA_INVALID_PARAM;
	}
	uio_token *t = (uio_token *)*token;

	if (t->magic == UIO_TOKEN_MAGIC) {
		free(t);
		return FPGA_OK;
	}
	return FPGA_INVALID_PARAM;
#endif
	return FPGA_OK;
}
