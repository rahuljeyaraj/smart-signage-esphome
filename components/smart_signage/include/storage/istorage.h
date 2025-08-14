#pragma once

#include <stdint.h>
#include <stddef.h>
#include <etl/string.h>

namespace esphome::smart_signage::storage {

// Keep your existing aliases (15-char ETL strings)
using Key       = etl::string<15>;
using Namespace = etl::string<15>;

/** Abstract storage interface.
 *  All methods return true on success, false on failure.
 *  For loadString/loadBlob: pass buffer + capacity via inout_len.
 *    - If the provided capacity is too small, the method sets inout_len
 *      to the required size and returns false (no partial copy).
 */
class IStorage {
  public:
    // Scalars
    virtual bool storeU32(const Key &key, uint32_t value)                             = 0;
    virtual bool loadU32(const Key &key, uint32_t &value)                             = 0;
    virtual bool loadU32OrDefault(const Key &key, uint32_t &value, uint32_t defValue) = 0;

    // Raw blobs
    virtual bool storeBlob(const Key &key, const void *data, size_t len) = 0;
    virtual bool loadBlob(const Key &key, void *data, size_t &inout_len) = 0;

    // Zero-terminated strings (UTF-8)
    virtual bool storeString(const Key &key, const char *str)             = 0;
    virtual bool loadString(const Key &key, char *buf, size_t &inout_len) = 0;

    // Housekeeping
    virtual bool eraseKey(const Key &key) = 0;
    virtual bool eraseAll()               = 0;

    virtual ~IStorage() = default;
};
} // namespace esphome::smart_signage::storage
