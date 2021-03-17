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

fpga_result __UIO_API__ uio_fpgaOpen(fpga_token token,
				     fpga_handle *handle,
				     int flags)
{
	UNUSED_PARAM(token);
	UNUSED_PARAM(handle);
	UNUSED_PARAM(flags);

#if 0
	fpga_result res = FPGA_EXCEPTION;
	uio_token *_token;
	vfio_handle *_handle;
	pthread_mutexattr_t mattr;

	ASSERT_NOT_NULL(token);
	ASSERT_NOT_NULL(handle);
	_token = token_check(token);
	ASSERT_NOT_NULL(_token);

	if (pthread_mutexattr_init(&mattr)) {
		OPAE_MSG("Failed to init handle mutex attr");
		return FPGA_EXCEPTION;
	}

	if (flags & FPGA_OPEN_SHARED) {
		OPAE_MSG("shared mode ignored at this time");
		//return FPGA_INVALID_PARAM;
	}


	_handle = malloc(sizeof(vfio_handle));
	if (_handle == NULL) {
		OPAE_ERR("Failed to allocate memory for handle");
		res = FPGA_NO_MEMORY;
		goto out_attr_destroy;
	}

	memset(_handle, 0, sizeof(*_handle));

	// mark data structure as valid
	if (pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE) ||
	    pthread_mutex_init(&_handle->lock, &mattr)) {
		OPAE_MSG("Failed to init handle mutex");
		res = FPGA_EXCEPTION;
		goto out_attr_destroy;
	}

	_handle->magic = VFIO_HANDLE_MAGIC;
	_handle->token = clone_token(_token);
	_handle->vfio_pair = open_vfio_pair(_token->device->addr);
	if (!_handle->vfio_pair) {
		OPAE_ERR("error opening vfio device");
		res = FPGA_EXCEPTION;
		goto out_attr_destroy;
	}
	uint8_t *mmio = NULL;
	size_t size;

	if (opae_vfio_region_get(_handle->vfio_pair->device,
				 _token->region, &mmio, &size)) {
		OPAE_ERR("error opening vfio region");
		res = FPGA_EXCEPTION;
		goto out_attr_destroy;
	}
	_handle->mmio_base = (volatile uint8_t *)(mmio);
	_handle->mmio_size = size;
	*handle = _handle;
	res = FPGA_OK;
out_attr_destroy:
	pthread_mutexattr_destroy(&mattr);
	if (res && _handle) {
		if (_handle->vfio_pair) {
			close_vfio_pair(&_handle->vfio_pair);
		}
		free(_handle);
	}
	return res;
#endif
	return FPGA_OK;
}

fpga_result __UIO_API__ uio_fpgaClose(fpga_handle handle)
{
	UNUSED_PARAM(handle);

#if 0
	fpga_result res = FPGA_OK;
	vfio_handle *h = handle_check_and_lock(handle);

	ASSERT_NOT_NULL(h);

	if (token_check(h->token))
		free(h->token);
	else
		OPAE_MSG("invalid token in handle");

	close_vfio_pair(&h->vfio_pair);
	if (pthread_mutex_unlock(&h->lock) ||
	    pthread_mutex_destroy(&h->lock)) {
		OPAE_MSG("error unlocking/destroying handle mutex");
		res = FPGA_EXCEPTION;
	}
	free(h);
#endif
	return FPGA_OK;
}
