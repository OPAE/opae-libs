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
#include <config.h>
#include <opae/dfl.h>
#include <opae/fpga.h>
#include "udev-mock.h"
#include "gtest/gtest.h"

extern "C" {
size_t udev_direct_read_attr(struct udev_device *dev, const char *attr, char **buffer);
int udev_direct_read_attr64(struct udev_device *dev, const char *attr, uint64_t *value);
}

class libdfl : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    mock_ = udev_mock::instance();
    ASSERT_TRUE(mock_->load("n3000-x2.yaml"));
    dfl_initialize();
    devices_ = dfl_device_enum();
    ASSERT_NE(devices_, nullptr);
  }

  virtual void TearDown()
  {
    dfl_device_destroy(devices_);
    dfl_finalize();
  }

  udev_mock::ptr_t mock_;
  dfl_device *devices_;

};

TEST_F(libdfl, dfl_device_enum)
{
  for (dfl_device *e = devices_; e; e = e->next) {
    fpga_guid guid;
    if (e->type == FPGA_DEVICE){
      EXPECT_NE(dfl_device_get_afu_id(e, guid), 0);
      EXPECT_EQ(dfl_device_get_compat_id(e, guid), 0);
    } else {
      EXPECT_EQ(dfl_device_get_afu_id(e, guid), 0);
      EXPECT_NE(dfl_device_get_compat_id(e, guid), 0);
    }
  }
}

TEST_F(libdfl, udev_direct_read_attr)
{
  char *buffer = nullptr;
  std::string bitstream_id("bitstream_id");
  std::string bitstream_id_value("0x2300011001030f");
  for (dfl_device *e = devices_; e; e = e->next) {
    if (e->type == FPGA_DEVICE){
      mock_->write_attr(e->dev, bitstream_id, bitstream_id_value);
      auto sz = udev_direct_read_attr(e->dev, bitstream_id.c_str(), &buffer);
      ASSERT_TRUE(sz) << "Buffer not created";
      // write_attr will append a \n so let's trim it
      *(buffer + --sz) = '\0';
      EXPECT_EQ(sz, bitstream_id_value.size());
      EXPECT_STREQ(buffer, bitstream_id_value.c_str());
      free(buffer);
    }
  }
}


TEST_F(libdfl, udev_direct_read_attr64)
{
  std::string bitstream_id("bitstream_id");
  std::string bitstream_id_value("0x2300011001030f");
  uint64_t value64 = 0x2300011001030f;
  for (dfl_device *e = devices_; e; e = e->next) {
    if (e->type == FPGA_DEVICE){
      uint64_t value = 0xff;
      mock_->write_attr(e->dev, bitstream_id, bitstream_id_value);
      ASSERT_EQ(udev_direct_read_attr64(e->dev, bitstream_id.c_str(), &value), FPGA_OK);
      EXPECT_EQ(value64, value);
    }
  }
}


TEST_F(libdfl, dfl_device_get_parent)
{
  for (dfl_device *e = devices_; e; e = e->next) {
    if (e->type == FPGA_ACCELERATOR){
      auto p = dfl_device_get_parent(e);
      ASSERT_NE(p, nullptr);
      dfl_device_destroy(p);
    }
  }
}

TEST_F(libdfl, dfl_device_get_ports_num)
{
  for (dfl_device *e = devices_; e; e = e->next) {
    if (e->type == FPGA_DEVICE){
      uint32_t value = 0;
      ASSERT_EQ(dfl_device_get_ports_num(e, &value), FPGA_OK);
      EXPECT_NE(value, 0);
    }
  }
}

TEST_F(libdfl, dfl_device_get_bbs_id)
{
  for (dfl_device *e = devices_; e; e = e->next) {
    if (e->type == FPGA_DEVICE){
      uint64_t value = 0;
      ASSERT_EQ(dfl_device_get_bbs_id(e, &value), FPGA_OK);
      EXPECT_NE(value, 0);
    }
  }
}
