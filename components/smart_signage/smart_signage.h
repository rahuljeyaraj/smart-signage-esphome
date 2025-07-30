#pragma once

#include "esphome/core/component.h"

namespace esphome
{
  namespace smart_signage
  {

    class SmartSignage : public Component
    {
    public:
      void setup() override;
      void loop() override;
      void dump_config() override;
    };

  } // namespace smart_signage
} // namespace esphome