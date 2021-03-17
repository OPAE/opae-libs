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

#if 0
STATIC int read_pci_link(const char *addr, const char *link, char *value, size_t max)
{
	char path[PATH_MAX];
	char fullpath[PATH_MAX];

	snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/%s", addr, link);
	if (!realpath(path, fullpath)) {
		if (errno == ENOENT)
			return 1;
		OPAE_ERR("error reading path: %s", path);
		return 2;
	}
	char *p = strrchr(fullpath, '/');

	if (!p) {
		OPAE_ERR("error finding '/' in path: %s", fullpath);
		return 2;
	}
	strncpy(value, p+1, max);
	return 0;
}
#endif

#if 0
STATIC int read_pci_attr(const char *addr, const char *attr, char *value, size_t max)
{
	int res = FPGA_OK;
	char path[PATH_MAX];

	snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/%s", addr, attr);
	FILE *fp = fopen(path, "r");

	if (!fp) {
		OPAE_ERR("error opening: %s", path);
		return FPGA_EXCEPTION;
	}
	if (!fread(value, 1, max, fp)) {
		OPAE_ERR("error reading from: %s", path);
		res = FPGA_EXCEPTION;
	}
	fclose(fp);
	return res;
}
#endif

#if 0
STATIC int read_pci_attr_u32(const char *addr, const char *attr, uint32_t *value)
{
	char str_value[64];
	char *endptr = NULL;
	int res = read_pci_attr(addr, attr, str_value, sizeof(str_value));

	if (res)
		return res;
	uint32_t v = strtoul(str_value, &endptr, 0);

	if (endptr == str_value) {
		OPAE_ERR("error parsing string: %s", str_value);
		return FPGA_EXCEPTION;
	}
	*value = v;
	return FPGA_OK;
}
#endif

#if 0
STATIC int parse_pcie_info(pci_device_t *device, char *addr)
{
	char err[128] = {0};
	regex_t re;
	regmatch_t matches[PCIE_PATH_PATTERN_GROUPS] = { {0} };
	int res = FPGA_EXCEPTION;

	int reg_res = regcomp(&re, PCIE_PATH_PATTERN, REG_EXTENDED | REG_ICASE);

	if (reg_res) {
		OPAE_ERR("Error compiling regex");
		return FPGA_EXCEPTION;
	}
	reg_res = regexec(&re, addr, PCIE_PATH_PATTERN_GROUPS, matches, 0);
	if (reg_res) {
		regerror(reg_res, &re, err, 128);
		OPAE_ERR("Error executing regex: %s", err);
		res = FPGA_EXCEPTION;
		goto out;
	} else {
		PARSE_MATCH_INT(addr, matches[1], device->bdf.segment, 16, out);
		PARSE_MATCH_INT(addr, matches[2], device->bdf.bus, 16, out);
		PARSE_MATCH_INT(addr, matches[3], device->bdf.device, 16, out);
		PARSE_MATCH_INT(addr, matches[4], device->bdf.function, 10, out);
	}
	res = FPGA_OK;

out:
	regfree(&re);
	return res;
}
#endif

#if 0
int uio_walk(pci_device_t *p)
{
	int res = 0;

	volatile uint8_t *mmio;
	size_t size;
	vfio_pair_t *pair = open_vfio_pair(p->addr);

	if (!pair) {
		OPAE_ERR("error opening vfio device: %s", p->addr);
		return 1;
	}
	struct opae_vfio *v = pair->device;
	// look for legacy FME guids in BAR 0
	if (opae_vfio_region_get(v, 0, (uint8_t **)&mmio, &size)) {
		OPAE_ERR("error getting BAR 0");
		res = 2;
		goto close;
	}

	// get the GUID at offset 0x8
	fpga_guid b0_guid;

	res = get_guid(((uint64_t *)mmio)+1, b0_guid);
	if (res) {
		OPAE_ERR("error reading guid");
		goto close;
	}

	// walk our known list of FME guids
	// and compare each one to the one read into b0_guid
	for (const char **u = fme_drivers; *u; u++) {
		fpga_guid uuid;

		res = uuid_parse(*u, uuid);
		if (res) {
			OPAE_ERR("error parsing uuid: %s", *u);
			goto close;
		}
		if (!uuid_compare(uuid, b0_guid)) {
			// we found a legacy FME in BAR0, walk it
			res = walk_fme(p, v, mmio, 0);
			goto close;
		}
	}

	// treat all of BAR0 as an FPGA_ACCELERATOR
	uio_token *t = get_token(p, 0, FPGA_ACCELERATOR);

	t->mmio_size = size;
	t->user_mmio_count = 1;
	t->user_mmio[0] = 0;
	get_guid(1+(uint64_t *)mmio, t->guid);

	// now let's check other BARs
	for (uint32_t i = 1; i < BAR_MAX; ++i) {
		if (!opae_vfio_region_get(v, i, (uint8_t **)&mmio, &size)) {
			uio_token *t = get_token(p, i, FPGA_ACCELERATOR);

			get_guid(1+(uint64_t *)mmio, t->guid);
			t->mmio_size = size;
			t->user_mmio_count = 1;
			t->user_mmio[0] = 0;
		}
	}

close:
	close_vfio_pair(&pair);
	return res;
}
#endif

fpga_result get_guid(uint64_t *h, fpga_guid guid)
{
	ASSERT_NOT_NULL(h);
	size_t sz = 16;
	uint8_t *ptr = ((uint8_t *)h)+sz;

	for (size_t i = 0; i < sz; ++i) {
		guid[i] = *--ptr;
	}
	return FPGA_OK;
}

#if 0
void print_dfh(uint32_t offset, dfh *h)
{
	printf("0x%x: 0x%lx\n", offset, *(uint64_t *)h);
	printf("id: %d, rev: %d, next: %d, eol: %d, afu_minor: %d, version: %d, type: %d\n",
	       h->id, h->major_rev, h->next, h->eol, h->minor_rev, h->version, h->type);
}
#endif

#if 0
bool pci_matches_filter(const fpga_properties *filter, pci_device_t *dev)
{
	struct _fpga_properties *_prop = (struct _fpga_properties *)filter;

	if (FIELD_VALID(_prop, FPGA_PROPERTY_SEGMENT))
		if (_prop->segment != dev->bdf.segment)
			return false;
	if (FIELD_VALID(_prop, FPGA_PROPERTY_BUS))
		if (_prop->bus != dev->bdf.bus)
			return false;
	if (FIELD_VALID(_prop, FPGA_PROPERTY_DEVICE))
		if (_prop->device != dev->bdf.device)
			return false;
	if (FIELD_VALID(_prop, FPGA_PROPERTY_FUNCTION))
		if (_prop->function != dev->bdf.function)
			return false;
	if (FIELD_VALID(_prop, FPGA_PROPERTY_SOCKETID))
		if (_prop->socket_id != dev->numa_node)
			return false;
	return true;

}

bool pci_matches_filters(const fpga_properties *filters, uint32_t num_filters,
		 pci_device_t *dev)
{
	if (!filters)
		return true;
	for (uint32_t i = 0; i < num_filters; ++i) {
		if (pci_matches_filter(filters[i], dev))
			return true;
	}
	return false;
}

bool matches_filter(const fpga_properties *filter, uio_token *t)
{
	struct _fpga_properties *_prop = (struct _fpga_properties *)filter;

	if (FIELD_VALID(_prop, FPGA_PROPERTY_PARENT)) {
		if (t->type == FPGA_DEVICE)
			return false;
		uio_token *t_parent = (uio_token *)t->parent;
		uio_token *f_parent = (uio_token *)_prop->parent;

		if (!t_parent)
			return false;
		if (t_parent->device->bdf.bdf != f_parent->device->bdf.bdf)
			return false;
		if (t_parent->region != f_parent->region)
			return false;
	}

	if (FIELD_VALID(_prop, FPGA_PROPERTY_OBJTYPE))
		if (_prop->objtype != t->type)
			return false;
	if (FIELD_VALID(_prop, FPGA_PROPERTY_GUID)) {
		if (memcmp(_prop->guid, t->guid, sizeof(fpga_guid)))
			return false;
	}
	return true;
}

bool matches_filters(const fpga_properties *filters, uint32_t num_filters,
		     uio_token *t)
{
	if (!filters)
		return true;
	for (uint32_t i = 0; i < num_filters; ++i) {
		if (matches_filter(filters[i], t))
			return true;
	}
	return false;
}
#endif

void dump_csr(uint8_t *begin, uint8_t *end, uint32_t index)
{
	char fname[PATH_MAX] = { 0 };
	char str_value[64] = { 0 };

	snprintf(fname, sizeof(fname), "csr_%d.dat", index);
	FILE *fp = fopen(fname, "w");

	if (!fp) {
		OPAE_ERR("could not open file: %s", fname);
		return;
	}
	for (uint8_t *ptr = begin; ptr < end; ptr += 8) {
		uint64_t value = *(uint64_t *)ptr;

		if (value) {
			snprintf(str_value, sizeof(str_value), "0x%lx: 0x%lx\n", ptr-begin, value);
			fwrite(str_value, 1, strlen(str_value), fp);
		}
	}
	fclose(fp);

}

#if 0
dfh *next_feature(dfh *h)
{
	if (!h->next)
		return NULL;
	if (!h->version) {
		dfh0 *h0 = (dfh0 *)((uint8_t *)h + h->next);

		while (h0->next && h0->type != 0x4)
			h0 = (dfh0 *)((uint8_t *)h0 + h0->next);
		return h0->next ? (dfh *)h0 : NULL;
	}
	return NULL;
}
#endif

fpga_result __UIO_API__ uio_fpgaEnumerate(const fpga_properties *filters,
					  uint32_t num_filters,
					  fpga_token *tokens,
					  uint32_t max_tokens,
					  uint32_t *num_matches)
{
	UNUSED_PARAM(filters);
	UNUSED_PARAM(num_filters);
	UNUSED_PARAM(tokens);
	UNUSED_PARAM(max_tokens);
	UNUSED_PARAM(num_matches);
#if 0
	pci_device_t *dev = _pci_devices;
	uint32_t matches = 0;

	while (dev) {
		if (pci_matches_filters(filters, num_filters, dev)) {
			vfio_walk(dev);
			uio_token *ptr = dev->tokens;

			while (ptr) {
				if (matches_filters(filters, num_filters, ptr)) {
					if (matches < max_tokens) {
						tokens[matches] =
							clone_token(ptr);
					}
					++matches;
				}
				ptr = ptr->next;
			}
		}
		dev = dev->next;
	}
	*num_matches = matches;
#endif
	return FPGA_OK;
}
