/*
 * mock_driver.h
 * Copyright (C) 2021 rrojo <rrojo@rrojo-MOBL1>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef MOCK_DRIVER_H
#define MOCK_DRIVER_H


class mock_driver
{
  const char *get_attribute(const char *name)
  {
    return nullptr;
  }
};

class mock_dfl : public mock_driver
{

};

#endif /* !MOCK_DRIVER_H */
