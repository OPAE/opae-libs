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

fpga_result __UIO_API__ uio_fpgaUpdateProperties(fpga_token token,
						 fpga_properties prop)
{
	UNUSED_PARAM(token);
	UNUSED_PARAM(prop);
#if 0
	uio_token *t = token_check(token);

	ASSERT_NOT_NULL(t);

	struct _fpga_properties *_prop = (struct _fpga_properties *)prop;

	if (!_prop) {
		return FPGA_EXCEPTION;
	}
	if (_prop->magic != FPGA_PROPERTY_MAGIC) {
		OPAE_ERR("Invalid properties object");
		return FPGA_INVALID_PARAM;
	}
	_prop->vendor_id = t->device->vendor;
	SET_FIELD_VALID(_prop, FPGA_PROPERTY_VENDORID);

	_prop->device_id = t->device->device;
	SET_FIELD_VALID(_prop, FPGA_PROPERTY_DEVICEID);

	_prop->segment = t->device->bdf.segment;
	SET_FIELD_VALID(_prop, FPGA_PROPERTY_SEGMENT);

	_prop->bus = t->device->bdf.bus;
	SET_FIELD_VALID(_prop, FPGA_PROPERTY_BUS);

	_prop->device = t->device->bdf.device;
	SET_FIELD_VALID(_prop, FPGA_PROPERTY_DEVICE);

	_prop->function = t->device->bdf.function;
	SET_FIELD_VALID(_prop, FPGA_PROPERTY_FUNCTION);

	_prop->socket_id = t->device->numa_node;
	SET_FIELD_VALID(_prop, FPGA_PROPERTY_SOCKETID);

	_prop->object_id = ((uint64_t)t->device->bdf.bdf) << 32 | t->region;
	SET_FIELD_VALID(_prop, FPGA_PROPERTY_OBJECTID);


	_prop->objtype = t->type;
	SET_FIELD_VALID(_prop, FPGA_PROPERTY_OBJTYPE);

	if (t->type == FPGA_ACCELERATOR) {
		if (t->parent) {
			_prop->parent = clone_token(t->parent);
			SET_FIELD_VALID(_prop, FPGA_PROPERTY_PARENT);
		}
		memcpy(_prop->guid, t->guid, sizeof(fpga_guid));
		SET_FIELD_VALID(_prop, FPGA_PROPERTY_GUID);

		_prop->u.accelerator.num_mmio = t->user_mmio_count;
		SET_FIELD_VALID(_prop, FPGA_PROPERTY_NUM_MMIO);
	} else {
		memcpy(_prop->guid, t->compat_id, sizeof(fpga_guid));
		SET_FIELD_VALID(_prop, FPGA_PROPERTY_GUID);

		_prop->u.fpga.bbs_id = t->bitstream_id;
		SET_FIELD_VALID(_prop, FPGA_PROPERTY_BBSID);

		_prop->u.fpga.bbs_version.major =
			FPGA_BBS_VER_MAJOR(t->bitstream_id);
		_prop->u.fpga.bbs_version.minor =
			FPGA_BBS_VER_MINOR(t->bitstream_id);
		_prop->u.fpga.bbs_version.major =
			FPGA_BBS_VER_PATCH(t->bitstream_id);
		SET_FIELD_VALID(_prop, FPGA_PROPERTY_BBSVERSION);

		_prop->u.fpga.num_slots = t->num_ports;
		SET_FIELD_VALID(_prop, FPGA_PROPERTY_NUM_SLOTS);
	}
#endif

	return FPGA_OK;
}

fpga_result __UIO_API__ uio_fpgaGetProperties(fpga_token token,
					      fpga_properties *prop)
{
	ASSERT_NOT_NULL(prop);
	struct _fpga_properties *_prop = NULL;
	fpga_result result = FPGA_OK;

	result = fpgaGetProperties(NULL, (fpga_properties *)&_prop);
	if (result)
		return result;
	if (token) {
		result = uio_fpgaUpdateProperties(token, _prop);
		if (result)
			goto out_free;
	}
	*prop = (fpga_properties)_prop;

	return result;
out_free:
	free(_prop);
	return result;
}

fpga_result __UIO_API__ uio_fpgaGetPropertiesFromHandle(fpga_handle handle,
							fpga_properties *prop)
{
	ASSERT_NOT_NULL(prop);
	uio_handle *h = handle_check(handle);

	ASSERT_NOT_NULL(h);

	uio_token *t = h->token;

	return uio_fpgaGetProperties(t, prop);
}
