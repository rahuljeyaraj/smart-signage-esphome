// config_manager.h
#pragma once

#include <cstdint>
#include <string>

/** Abstract interface for 32-bit key/value and string config storage. */
class ConfigManager {
  public:
    using Namespace   = std::string;
    using Key         = std::string;
    using ValueString = std::string;

    // Integer getters/setters
    virtual bool setValue(const Namespace &ns, const Key &key, uint32_t value)  = 0;
    virtual bool getValue(const Namespace &ns, const Key &key, uint32_t &value) = 0;
    virtual bool getValue(
        const Namespace &ns, const Key &key, uint32_t &value, uint32_t defaultValue) = 0;

    // String getters/setters
    virtual bool setString(const Namespace &ns, const Key &key, const ValueString &value) = 0;
    virtual bool getString(const Namespace &ns, const Key &key, ValueString &value)       = 0;
    virtual bool getString(const Namespace &ns, const Key &key, ValueString &value,
        const ValueString &defaultValue)                                                  = 0;

    virtual ~ConfigManager() = default;
};
