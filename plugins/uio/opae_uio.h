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
#ifndef __OPAE_UIO_PLUGIN_H__
#define __OPAE_UIO_PLUGIN_H__
#include <opae/uio.h>
#include <opae/fpga.h>

#define BAR_MAX 6
#define FPGA_BBS_VER_MAJOR(i) (((i) >> 56) & 0xf)
#define FPGA_BBS_VER_MINOR(i) (((i) >> 52) & 0xf)
#define FPGA_BBS_VER_PATCH(i) (((i) >> 48) & 0xf)

#define GUIDSTR_MAX 36

typedef union _bdf {
	struct {
		uint16_t segment;
		uint8_t bus;
		uint8_t device : 5;
		uint8_t function : 3;
	};
	uint32_t bdf;
} bdf_t;

struct _uio_token;

#define PCIADDR_MAX 16
typedef struct _pci_device {
	char addr[PCIADDR_MAX];
	bdf_t bdf;
	uint32_t vendor;
	uint32_t device;
	uint32_t numa_node;
	struct _uio_token *tokens;
	struct _pci_device *next;
} pci_device_t;

typedef struct _uio_ops {
#if 0
	fpga_result(*reset)(const pci_device_t *p, volatile uint8_t *mmio);
#endif
} uio_ops;

#define USER_MMIO_MAX 8
#define UIO_TOKEN_MAGIC 0xEF1010FE
typedef struct _uio_token {
	uint32_t magic;
	fpga_guid guid;
	fpga_guid compat_id;
	pci_device_t *device;
	uint32_t region;
	uint32_t offset;
	uint32_t mmio_size;
	uint32_t pr_control;
	uint32_t user_mmio_count;
	uint32_t user_mmio[USER_MMIO_MAX];
	uint64_t bitstream_id;
	uint64_t bitstream_mdata;
	uint8_t num_ports;
	uint32_t type;
	struct _uio_token *parent;
	struct _uio_token *next;
	uio_ops ops;
} uio_token;

#define UIO_HANDLE_MAGIC ~UIO_TOKEN_MAGIC
typedef struct _uio_handle {
	uint32_t magic;
	struct _uio_token *token;
#if 0
	vfio_pair_t *vfio_pair;

	volatile uint8_t *mmio_base;
	size_t mmio_size;
#endif
	pthread_mutex_t lock;
} uio_handle;

int pci_discover(void);
int features_discover(void);
pci_device_t *get_pci_device(char addr[PCIADDR_MAX]);
void free_device_list(void);
void free_buffer_list(void);
uio_token *get_token(pci_device_t *p, uint32_t region, int type);
fpga_result get_guid(uint64_t *h, fpga_guid guid);


void free_token_list(uio_token *tokens);
uio_token *clone_token(uio_token *src);
uio_token *token_check(fpga_token token);
uio_handle *handle_check(fpga_handle handle);
uio_handle *handle_check_and_lock(fpga_handle handle);
uio_token *find_token(const pci_device_t *p, uint32_t region);
uio_token *get_token(pci_device_t *p, uint32_t region, int type);

#endif // __OPAE_UIO_PLUGIN_H__
