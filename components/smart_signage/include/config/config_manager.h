#pragma once

#include <stdint.h>
#include <etl/string.h>

using Key       = etl::string<15>;
using Namespace = etl::string<15>;

/** Abstract interface for 32-bit key/value config storage. */
class ConfigManager {
  public:
    virtual bool setValue(const Key &key, uint32_t value)                         = 0;
    virtual bool getValue(const Key &key, uint32_t &value)                        = 0;
    virtual bool getValue(const Key &key, uint32_t &value, uint32_t defaultValue) = 0;
    virtual bool eraseAll()                                                       = 0;
    virtual ~ConfigManager()                                                      = default;
};
