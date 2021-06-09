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
#include <linux/limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libudev.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <any>
#include <fstream>
#include <map>
#include <vector>


#include <yaml-cpp/yaml.h>
#include "udev-mock.h"

// TODO: remove iostream
#include <iostream>
// TODO-----------------

udev_mock::ptr_t udev_mock::instance_;

class udev_list_entry
{
public:
  typedef std::shared_ptr<udev_list_entry> ptr_t;
  typedef std::pair<std::string, std::string> data_t;
  udev_list_entry()
  {
    ptr_ = list_.begin();
  }

  const char *get_name()
  {
    if (ptr_ == list_.end())
      return nullptr;
    return ptr_->first.c_str();
  }

  const char *get_value()
  {
    if (ptr_ == list_.end())
      return nullptr;
    return ptr_->second.c_str();
  }

  void reset_ptr()
  {
    ptr_ = list_.begin();
  }

  void clear()
  {
    list_.clear();
  }

  udev_list_entry *next()
  {
    if (++ptr_ == list_.end())
      return nullptr;
    return this;
  }

  void add(const std::string &name, const std::string &value)
  {
    list_.push_back(std::make_pair(name, value));
  }

private:
  std::vector<data_t> list_;
  std::vector<data_t>::iterator ptr_;
};

//struct udev_list_entry
//{
//  typedef std::shared_ptr<udev_list_entry> ptr_t;
//  udev_list_entry(const std::string &name, const std::string &value)
//  : name(name)
//  , value(value)
//  , next(nullptr)
//  {
//  }
//
//  void add_list_entry(const std::string &name, const std::string &value)
//  {
//    udev_list_entry *le = new udev_list_entry(name, value);
//    auto ptr = this;
//    while(ptr->next) ptr = ptr->next;
//    ptr->next = le;
//
//  }
//
//  std::string name;
//  std::string value;
//  udev_list_entry *next;
//};

//void free_list_entry(udev_list_entry *le)
//{
//  auto ptr = le;
//  while(ptr) {
//    auto tmp = ptr;
//    delete ptr;
//    ptr = ptr->next;
//  }
//}

class udev_device
{
public:
  typedef std::shared_ptr<udev_device> ptr_t;

  udev_device(const std::string &root, const std::string &path)
  : root_(root)
  , syspath_(path)
  , parent_(nullptr)
  , sysattr_list_(new udev_list_entry())
  , full_path_("")
  {
    auto pos = syspath_.find_last_of('/');
    if (pos != std::string::npos && pos < syspath_.size()-1)
      sysname_ = syspath_.substr(pos+1);
    full_path_ = root + "/" + path;
  }

  ~udev_device()
  {
  }

  void add_attribute(const std::string &name, const std::string &value)
  {
    attributes_[name] = value;
    sysattr_list_->add(name, value);
  }

  void add_property(const std::string &name, const std::string &value)
  {
    properties_[name] = value;
  }

  void set_parent(udev_device::ptr_t p)
  {
    parent_ = p;
  }

  udev_device::ptr_t parent()
  {
    return parent_;
  }

  const char *syspath()
  {
    return syspath_.c_str();
  }

  const char *get_sysname()
  {
      return sysname_.c_str();
  }

  std::map<std::string, std::string> &properties()
  {
    return properties_;
  }

  std::map<std::string, std::string> &attributes()
  {
    return attributes_;
  }

  void set_syspath(const std::string &value)
  {
    syspath_ = value;
  }

  void set_sysname(const std::string &value)
  {
    sysname_ = value;
  }

  void set_driver(const std::string &value)
  {
    driver_ = value;
  }

  void set_subsystem(const std::string &value)
  {
    subsystem_ = value;
  }

  void set_device_type(const std::string &value)
  {
    device_type_ = value;
  }

  void set_device_path(const std::string &value)
  {
    device_path_ = value;
  }

  void set_device_number(uint32_t value)
  {
    device_number_ = value;
  }

  const char* get_syspath()
  {
    return syspath_.empty() ? nullptr : syspath_.c_str();
  }

  const char* get_fullpath()
  {
    return full_path_.empty() ? nullptr : full_path_.c_str();
  }

  const char* get_driver()
  {
    return driver_.empty() ? nullptr : driver_.c_str();
  }

  const char*get_subsystem()
  {
    return subsystem_.empty() ? nullptr : subsystem_.c_str();
  }

  const char*get_device_type()
  {
    return device_type_.c_str();
  }

  const char*get_device_path()
  {
    return device_path_.empty() ? nullptr: device_path_.c_str();
  }

  uint32_t get_device_number()
  {
    return device_number_;
  }

  const char *get_property_value(const char *key)
  {
    auto iter = properties_.find(key);
    if (iter == properties_.end())
      return nullptr;
    return iter->second.c_str();
  }

  const char *get_sysattr_value(const char *sysattr)
  {
    auto iter = attributes_.find(sysattr);
    if (iter == attributes_.end())
      return nullptr;
    return iter->second.c_str();
  }

  udev_list_entry *get_sysattr_list_entry()
  {
    sysattr_list_->reset_ptr();
    return sysattr_list_.get();
  }

private:
  std::string root_;
  std::string syspath_;
  std::string sysname_;
  std::string driver_;
  std::string subsystem_;
  std::string device_type_;
  std::string device_path_;
  uint32_t device_number_;
  std::map<std::string, std::string> attributes_;
  std::map<std::string, std::string> properties_;
  udev_device::ptr_t parent_;
  udev_list_entry::ptr_t sysattr_list_;
  std::string full_path_;
};


class udev_enumerate
{
public:
  udev_enumerate(udev *u)
  : udev_(u)
  , list_(new udev_list_entry())
  , subsystem_match_(nullptr)
  , subsystem_nomatch_(nullptr)
  {

  }

  ~udev_enumerate()
  {
  }

  void scan_devices()
  {
    list_->clear();
    for (auto &d : udev_mock::instance()->devices()) {
      if (matches(d.second)){
        list_->add(d.second->get_fullpath(), d.first);
      }
    }
  }


  bool matches(udev_device::ptr_t d)
  {
    if (subsystem_match_) {
      auto d_subsystem = d->get_subsystem();
      if (!d_subsystem || strcmp(subsystem_match_, d_subsystem))
        return false;
    }
    if (subsystem_nomatch_ && d->get_subsystem() == subsystem_nomatch_)
      return false;
    if (matches_properties(d) &&
        matches_attributes(d))
      return true;
    return false;
  }

  bool matches_properties(udev_device::ptr_t d)
  {
    if (properties_.empty()) return true;
    for (auto kv : properties_) {
      auto p = d->properties().find(kv.first);
      if (p != d->properties().end()){
        if (p->second == kv.second) return true;
      }
    }
    return false;
  }

  bool matches_attributes(udev_device::ptr_t d)
  {
    if (attributes_.empty()) return true;
    for (auto kv : attributes_) {
      auto p = d->attributes().find(kv.first);
      if (p != d->attributes().end()){
        if (p->second == kv.second) return true;
      }
    }
    return false;
  }

  void add_property(const char *name, const char *value)
  {
    properties_.push_back(std::make_pair(name, value));
  }

  void add_attribute(const char *name, const char *value)
  {
    attributes_.push_back(std::make_pair(name, value));
  }

  void add_match_subsystem(const char *subsystem)
  {
    subsystem_match_ = subsystem;
    properties_.push_back(std::make_pair("SUBSYSTEM", subsystem));
  }

  void add_nomatch_subsystem(const char *subsystem)
  {
    subsystem_nomatch_ = subsystem;
  }

  udev_list_entry *get_list_entry()
  {
    list_->reset_ptr();
    return list_.get();
  }

  udev *get_udev()
  {
    return udev_;
  }
private:
  udev *udev_;
  udev_list_entry::ptr_t list_;
  const char *subsystem_match_;
  const char *subsystem_nomatch_;
  std::vector<std::pair<std::string, std::string>> properties_;
  std::vector<std::pair<std::string, std::string>> attributes_;
  std::vector<udev_device::ptr_t> matches_;
};


struct udev
{
  udev_enumerate *make_udev_enumerate()
  {
    auto ue = std::make_shared<udev_enumerate>(this);
    ues_.push_back(ue);
    return ue.get();
  }


  std::vector<std::shared_ptr<udev_enumerate>> ues_;
};

udev_mock::udev_mock()
{
  char tmp[PATH_MAX] = "mock-udev-XXXXXX";
  root_ = mkdtemp(tmp);
}

udev_mock::~udev_mock()
{

}

udev_mock::ptr_t udev_mock::instance()
{
  if (!instance_)
    instance_.reset(new udev_mock());
  return instance_;
}

std::shared_ptr<udev> udev_mock::get_udev()
{
  if (!udev_)
    udev_ =  std::make_shared<udev>();
  return udev_;
}

udev_device::ptr_t udev_mock::find(const char *syspath)
{
  std::string spath(syspath);
  if (spath.find(root_) != std::string::npos)
    spath = spath.substr(root_.size());
  if (spath.find("//") == 0)
    spath = spath.substr(1);
  auto iter = mocks_.find(spath);
  if (iter == mocks_.end())
    return nullptr;
  return iter->second;
}

bool udev_mock::load(const char *file)
{
  YAML::Node platform;
  try {
    platform = YAML::LoadFile(file);
  } catch(YAML::BadFile & err) {
    std::cerr << "Could not load file(" << file << "): "
              << err.what() << "\n";
    return false;
  }

  std::map<std::string, std::string> d_parents;
  for (auto n : platform) {
    //std::cout << n.first << "\n";
    auto m = std::make_shared<udev_device>(root_, n.first.as<std::string>());
    for (auto a : n.second["attributes"]) {
      auto name = a.first;
      auto value = a.second;
      if (name && value) {
        std::string value_str = value.Scalar();
        if (!value_str.empty()) {
          value_str.erase(value_str.find_last_not_of(" \n\r\t")+1);
        }
        m->add_attribute(name.Scalar(), value_str);
      }
    }
    for (auto a : n.second["properties"]) {
      auto name = a.first.as<std::string>();
      auto value = a.second.as<std::string>();
      m->add_property(name, value);
    }

    auto parent = n.second["parent"];
    if (parent)
      d_parents[n.first.Scalar()] = parent.Scalar();
    m->set_driver(n.second["driver"].Scalar());
    m->set_subsystem(n.second["subsystem"].Scalar());
    m->set_device_type(n.second["device_type"].Scalar());
    m->set_device_number(n.second["device_number"].as<uint32_t>());
    m->set_device_path(n.second["device_path"].Scalar());
    mocks_[n.first.as<std::string>()] = m;
  }
  for (auto kv : d_parents)
    if (!kv.second.empty())
      mocks_[kv.first]->set_parent(mocks_[kv.second]);
  return true;
}

void udev_mock::write_attr(udev_device *dev, const std::string &attr, const std::string &value)
{
  std::string attr_path = dev->get_fullpath() + std::string("/") + attr;
  auto pos = attr_path.find("/");
  while (pos != std::string::npos) {
    struct stat st;
    auto sub = attr_path.substr(0, pos);
    if (stat(sub.c_str(), &st))
      mkdir(sub.c_str(), 0777);
    pos = attr_path.find("/", pos+1);
  }
  std::ofstream attr_file;
  attr_file.open(attr_path.c_str());
  attr_file << value << "\n";
  attr_file.close();
}

udev_mock::device_list &udev_mock::devices()
{
  return mocks_;
}

struct udev *udev_ref(struct udev *udev)
{
  return udev;
}

struct udev *udev_unref(struct udev *udev)
{
  return udev;
}

struct udev *udev_new(void)
{
  return udev_mock::instance()->get_udev().get();
}

//void udev_set_log_fn(struct udev *udev,
//                            void (*log_fn)(struct udev *udev,
//                                           int priority, const char *file, int line, const char *fn,
//                                           const char *format, va_list args)) __attribute__((__deprecated__))
//{
//}

//int udev_get_log_priority(struct udev *udev) __attribute__((__deprecated__))
//{
//  return 0;
//}
//
//void udev_set_log_priority(struct udev *udev, int priority) __attribute__((__deprecated__))
//{
//  return 0;
//}

//void *udev_get_userdata(struct udev *udev);
//void udev_set_userdata(struct udev *udev, void *userdata);

/*
 * udev_list
 *
 * access to libudev generated lists
 */
struct udev_list_entry *udev_list_entry_get_next(struct udev_list_entry *list_entry)
{
  return list_entry->next();
}

struct udev_list_entry *udev_list_entry_get_by_name(struct udev_list_entry *list_entry, const char *name)
{
  return NULL;
}

const char *udev_list_entry_get_name(struct udev_list_entry *list_entry)
{
  return list_entry->get_name();
}

const char *udev_list_entry_get_value(struct udev_list_entry *list_entry)
{
  return list_entry->get_value();
}


/*
 * udev_device
 *
 * access to sysfs/kernel devices
 */
//struct udev_device;
struct udev_device *udev_device_ref(struct udev_device *udev_device)
{
  return udev_device;
}

struct udev_device *udev_device_unref(struct udev_device *udev_device)
{
  return udev_device;
}

struct udev *udev_device_get_udev(struct udev_device *udev_device)
{
  return udev_mock::instance()->get_udev().get();
}

struct udev_device *udev_device_new_from_syspath(struct udev *udev, const char *syspath)
{
  auto mock = udev_mock::instance();
  auto d = mock->find(syspath);
  return d.get();
}

struct udev_device *udev_device_new_from_devnum(struct udev *udev, char type, dev_t devnum)
{
  return NULL;
}

struct udev_device *udev_device_new_from_subsystem_sysname(struct udev *udev, const char *subsystem, const char *sysname)
{
  return NULL;
}

struct udev_device *udev_device_new_from_device_id(struct udev *udev, const char *id)
{
  return NULL;
}

struct udev_device *udev_device_new_from_environment(struct udev *udev)
{
  return NULL;
}

/* udev_device_get_parent_*() does not take a reference on the returned device, it is automatically unref'd with the parent */
struct udev_device *udev_device_get_parent(struct udev_device *udev_device)
{
  return udev_device->parent().get();
}

struct udev_device *udev_device_get_parent_with_subsystem_devtype(struct udev_device *udev_device,
                                                                  const char *subsystem, const char *devtype);
/* retrieve device properties */
const char *udev_device_get_devpath(struct udev_device *udev_device)
{
  auto iter = udev_device->properties().find("DEVPATH");
  if (iter == udev_device->properties().end())
    return nullptr;
  return iter->second.c_str();
}

const char *udev_device_get_subsystem(struct udev_device *udev_device)
{
  return udev_device->get_subsystem();
}

const char *udev_device_get_devtype(struct udev_device *udev_device)
{
  return udev_device->get_device_type();
}

const char *udev_device_get_syspath(struct udev_device *udev_device)
{
  return udev_device->get_fullpath();
}

const char *udev_device_get_sysname(struct udev_device *udev_device)
{
  return udev_device->get_sysname();
}

const char *udev_device_get_sysnum(struct udev_device *udev_device)
{
  return "[udev-mock]";
}

const char *udev_device_get_devnode(struct udev_device *udev_device)
{
  return "[udev-mock]";
}

int udev_device_get_is_initialized(struct udev_device *udev_device)
{
  return 0;
}

struct udev_list_entry *udev_device_get_devlinks_list_entry(struct udev_device *udev_device)
{
  return NULL;
}

struct udev_list_entry *udev_device_get_properties_list_entry(struct udev_device *udev_device)
{
  return NULL;
}

struct udev_list_entry *udev_device_get_tags_list_entry(struct udev_device *udev_device)
{
  return NULL;
}

struct udev_list_entry *udev_device_get_sysattr_list_entry(struct udev_device *udev_device)
{
  return udev_device->get_sysattr_list_entry();
}

const char *udev_device_get_property_value(struct udev_device *udev_device, const char *key)
{
  return udev_device->get_property_value(key);
}

const char *udev_device_get_driver(struct udev_device *udev_device)
{
  return udev_device->get_driver();
}

dev_t udev_device_get_devnum(struct udev_device *udev_device)
{
  return 0;
}

const char *udev_device_get_action(struct udev_device *udev_device)
{
  return "[udev-mock]";
}

unsigned long long int udev_device_get_seqnum(struct udev_device *udev_device)
{
  return 0;
}

unsigned long long int udev_device_get_usec_since_initialized(struct udev_device *udev_device)
{
  return 0;
}

const char *udev_device_get_sysattr_value(struct udev_device *udev_device, const char *sysattr)
{
  return udev_device->get_sysattr_value(sysattr);
}

int udev_device_set_sysattr_value(struct udev_device *udev_device, const char *sysattr, const char *value)
{
  return 0;
}

int udev_device_has_tag(struct udev_device *udev_device, const char *tag)
{
  return 0;
}


/*
 * udev_monitor
 *
 * access to kernel uevents and udev events
 */
struct udev_monitor;
struct udev_monitor *udev_monitor_ref(struct udev_monitor *udev_monitor)
{
  return NULL;
}

struct udev_monitor *udev_monitor_unref(struct udev_monitor *udev_monitor)
{
  return NULL;
}

struct udev *udev_monitor_get_udev(struct udev_monitor *udev_monitor)
{
  return NULL;
}

/* kernel and udev generated events over netlink */
struct udev_monitor *udev_monitor_new_from_netlink(struct udev *udev, const char *name)
{
  return NULL;
}

/* bind socket */
int udev_monitor_enable_receiving(struct udev_monitor *udev_monitor)
{
  return 0;
}

int udev_monitor_set_receive_buffer_size(struct udev_monitor *udev_monitor, int size)
{
  return 0;
}

int udev_monitor_get_fd(struct udev_monitor *udev_monitor)
{
  return 0;
}

struct udev_device *udev_monitor_receive_device(struct udev_monitor *udev_monitor)
{
  return NULL;
}

/* in-kernel socket filters to select messages that get delivered to a listener */
int udev_monitor_filter_add_match_subsystem_devtype(struct udev_monitor *udev_monitor,
                                                    const char *subsystem, const char *devtype);
int udev_monitor_filter_add_match_tag(struct udev_monitor *udev_monitor, const char *tag)
{
  return 0;
}

int udev_monitor_filter_update(struct udev_monitor *udev_monitor)
{
  return 0;
}

int udev_monitor_filter_remove(struct udev_monitor *udev_monitor)
{
  return 0;
}



/*
 * udev_enumerate
 *
 * search sysfs for specific devices and provide a sorted list
 */
struct udev_enumerate *udev_enumerate_ref(struct udev_enumerate *ue)
{
  return ue;
}

struct udev_enumerate *udev_enumerate_unref(struct udev_enumerate *ue)
{
  return ue;
}

struct udev *udev_enumerate_get_udev(struct udev_enumerate *ue)
{
  return ue->get_udev();
}

struct udev_enumerate *udev_enumerate_new(struct udev *u)
{
  return u->make_udev_enumerate();
}

/* device properties filter */
int udev_enumerate_add_match_subsystem(struct udev_enumerate *ue, const char *subsystem)
{
  ue->add_match_subsystem(subsystem);
  return 0;
}

int udev_enumerate_add_nomatch_subsystem(struct udev_enumerate *ue, const char *subsystem)
{
  return 0;
}

int udev_enumerate_add_match_sysattr(struct udev_enumerate *ue, const char *sysattr, const char *value)
{
  return 0;
}

int udev_enumerate_add_nomatch_sysattr(struct udev_enumerate *ue, const char *sysattr, const char *value)
{
  return 0;
}

int udev_enumerate_add_match_property(struct udev_enumerate *ue, const char *property, const char *value)
{
  ue->add_property(property, value);
  return 0;
}

int udev_enumerate_add_match_sysname(struct udev_enumerate *ue, const char *sysname)
{
  return 0;
}

int udev_enumerate_add_match_tag(struct udev_enumerate *ue, const char *tag)
{
  return 0;
}

int udev_enumerate_add_match_parent(struct udev_enumerate *ue, struct udev_device *parent)
{
  return 0;
}

int udev_enumerate_add_match_is_initialized(struct udev_enumerate *ue)
{
  return 0;
}

int udev_enumerate_add_syspath(struct udev_enumerate *ue, const char *syspath)
{
  return 0;
}

/* run enumeration with active filters */
int udev_enumerate_scan_devices(struct udev_enumerate *ue)
{
  ue->scan_devices();
  return 0;
}

int udev_enumerate_scan_subsystems(struct udev_enumerate *ue)
{
  return 0;
}

/* return device list */
struct udev_list_entry *udev_enumerate_get_list_entry(struct udev_enumerate *ue)
{
  return ue->get_list_entry();
}


/*
 * udev_queue
 *
 * access to the currently running udev events
 */
struct udev_queue;
struct udev_queue *udev_queue_ref(struct udev_queue *udev_queue)
{
  return NULL;
}

struct udev_queue *udev_queue_unref(struct udev_queue *udev_queue)
{
  return NULL;
}

struct udev *udev_queue_get_udev(struct udev_queue *udev_queue)
{
  return NULL;
}

struct udev_queue *udev_queue_new(struct udev *udev)
{
  return NULL;
}

//unsigned long long int udev_queue_get_kernel_seqnum(struct udev_queue *udev_queue) __attribute__((__deprecated__))
//{
//  return 0;
//}
//
//unsigned long long int udev_queue_get_udev_seqnum(struct udev_queue *udev_queue) __attribute__((__deprecated__))
//{
//  return 0;
//}

int udev_queue_get_udev_is_active(struct udev_queue *udev_queue)
{
  return 0;
}

int udev_queue_get_queue_is_empty(struct udev_queue *udev_queue)
{
  return 0;
}

//int udev_queue_get_seqnum_is_finished(struct udev_queue *udev_queue, unsigned long long int seqnum) __attribute__((__deprecated__))
//{
//  return 0;
//}

//int udev_queue_get_seqnum_sequence_is_finished(struct udev_queue *udev_queue,
//                                               unsigned long long int start, unsigned long long int end) __attribute__((__deprecated__))
//{
//  return 0;
//}

int udev_queue_get_fd(struct udev_queue *udev_queue)
{
  return 0;
}

int udev_queue_flush(struct udev_queue *udev_queue)
{
  return 0;
}

//struct udev_list_entry *udev_queue_get_queued_list_entry(struct udev_queue *udev_queue) __attribute__((__deprecated__))
//{
//  return NULL;
//}


/*
 *  udev_hwdb
 *
 *  access to the static hardware properties database
 */
struct udev_hwdb;
struct udev_hwdb *udev_hwdb_new(struct udev *udev)
{
  return NULL;
}

struct udev_hwdb *udev_hwdb_ref(struct udev_hwdb *hwdb)
{
  return NULL;
}

struct udev_hwdb *udev_hwdb_unref(struct udev_hwdb *hwdb)
{
  return NULL;
}

struct udev_list_entry *udev_hwdb_get_properties_list_entry(struct udev_hwdb *hwdb, const char *modalias, unsigned flags)
{
  return NULL;
}

