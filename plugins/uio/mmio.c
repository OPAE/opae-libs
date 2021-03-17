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

static inline volatile uint8_t *get_user_offset(uio_handle *h,
						uint32_t mmio_num,
						uint32_t offset)
{
	UNUSED_PARAM(h);
	UNUSED_PARAM(mmio_num);
	UNUSED_PARAM(offset);

#if 0
	uint32_t user_mmio = h->token->user_mmio[mmio_num];

	return h->mmio_base + user_mmio + offset;
#endif
	return NULL;
}


fpga_result __UIO_API__ uio_fpgaWriteMMIO64(fpga_handle handle,
					    uint32_t mmio_num,
					    uint64_t offset,
					    uint64_t value)
{
	UNUSED_PARAM(handle);
	UNUSED_PARAM(mmio_num);
	UNUSED_PARAM(offset);
	UNUSED_PARAM(value);
#if 0
	uio_handle *h = handle_check(handle);

	ASSERT_NOT_NULL(h);

	uio_token *t = h->token;

	if (t->type == FPGA_DEVICE)
		return FPGA_NOT_SUPPORTED;
	if (mmio_num > t->user_mmio_count)
		return FPGA_INVALID_PARAM;
	if (pthread_mutex_lock(&h->lock)) {
		OPAE_MSG("error locking handle mutex");
		return FPGA_EXCEPTION;
	}

	*((volatile uint64_t *)get_user_offset(h, mmio_num, offset)) = value;
	pthread_mutex_unlock(&h->lock);
#endif
	return FPGA_OK;
}

fpga_result __UIO_API__ uio_fpgaReadMMIO64(fpga_handle handle,
					   uint32_t mmio_num,
					   uint64_t offset,
					   uint64_t *value)
{
	UNUSED_PARAM(handle);
	UNUSED_PARAM(mmio_num);
	UNUSED_PARAM(offset);
	UNUSED_PARAM(value);

#if 0
	uio_handle *h = handle_check(handle);

	ASSERT_NOT_NULL(h);

	uio_token *t = h->token;

	if (t->type == FPGA_DEVICE)
		return FPGA_NOT_SUPPORTED;
	if (mmio_num > t->user_mmio_count)
		return FPGA_INVALID_PARAM;
	if (pthread_mutex_lock(&h->lock)) {
		OPAE_MSG("error locking handle mutex");
		return FPGA_EXCEPTION;
	}

	*value = *((volatile uint64_t *)get_user_offset(h, mmio_num, offset));
	pthread_mutex_unlock(&h->lock);
#endif
	return FPGA_OK;
}

fpga_result __UIO_API__ uio_fpgaWriteMMIO32(fpga_handle handle,
					    uint32_t mmio_num,
					    uint64_t offset,
					    uint32_t value)
{
	UNUSED_PARAM(handle);
	UNUSED_PARAM(mmio_num);
	UNUSED_PARAM(offset);
	UNUSED_PARAM(value);

#if 0
	uio_handle *h = handle_check(handle);

	ASSERT_NOT_NULL(h);

	uio_token *t = h->token;

	if (t->type == FPGA_DEVICE)
		return FPGA_NOT_SUPPORTED;
	if (mmio_num > t->user_mmio_count)
		return FPGA_INVALID_PARAM;
	if (pthread_mutex_lock(&h->lock)) {
		OPAE_MSG("error locking handle mutex");
		return FPGA_EXCEPTION;
	}

	*((volatile uint32_t *)get_user_offset(h, mmio_num, offset)) = value;
	pthread_mutex_unlock(&h->lock);
#endif
	return FPGA_OK;
}

fpga_result __UIO_API__ uio_fpgaReadMMIO32(fpga_handle handle,
					   uint32_t mmio_num,
					   uint64_t offset,
					   uint32_t *value)
{
	UNUSED_PARAM(handle);
	UNUSED_PARAM(mmio_num);
	UNUSED_PARAM(offset);
	UNUSED_PARAM(value);

#if 0
	uio_handle *h = handle_check(handle);

	ASSERT_NOT_NULL(h);

	uio_token *t = h->token;

	if (t->type == FPGA_DEVICE)
		return FPGA_NOT_SUPPORTED;
	if (mmio_num > t->user_mmio_count)
		return FPGA_INVALID_PARAM;
	if (pthread_mutex_lock(&h->lock)) {
		OPAE_MSG("error locking handle mutex");
		return FPGA_EXCEPTION;
	}

	*value = *((volatile uint32_t *)get_user_offset(h, mmio_num, offset));
	pthread_mutex_unlock(&h->lock);
#endif
	return FPGA_OK;
}

fpga_result __UIO_API__ uio_fpgaMapMMIO(fpga_handle handle,
					uint32_t mmio_num,
					uint64_t **mmio_ptr)
{
	UNUSED_PARAM(handle);
	UNUSED_PARAM(mmio_num);
	UNUSED_PARAM(mmio_ptr);

#if 0
	uio_handle *h = handle_check(handle);

	ASSERT_NOT_NULL(h);

	uio_token *t = h->token;

	if (mmio_num > t->user_mmio_count)
		return FPGA_INVALID_PARAM;
	*mmio_ptr = (uint64_t *)get_user_offset(h, mmio_num, 0);
#endif
	return FPGA_OK;
}

fpga_result __UIO_API__ uio_fpgaUnmapMMIO(fpga_handle handle,
					  uint32_t mmio_num)
{
	UNUSED_PARAM(handle);
	UNUSED_PARAM(mmio_num);
#if 0
	uio_handle *h = handle_check(handle);

	ASSERT_NOT_NULL(h);

	uio_token *t = h->token;

	if (mmio_num > t->user_mmio_count)
		return FPGA_INVALID_PARAM;
#endif
	return FPGA_OK;
}
