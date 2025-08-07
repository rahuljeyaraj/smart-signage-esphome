#pragma once
#include "esphome.h"
#include <esphome/components/number/number.h>
#include <esphome/components/select/select.h>
#include <map>
#include <vector>
#include "config/nvs_config_manager.h"
#include "user_intf.h"

namespace esphome::smart_signage {

class SmartSignage : public Component {
  public:
    explicit SmartSignage(const UiHandles &ui);

    void setup() override;
    void loop() override;
    void dump_config() override;

  private:
    NVSConfigManager nvsConfigManager_;
    UserIntf         userIntf_;

    static constexpr char TAG[] = "SmartSignage";
};

} // namespace esphome::smart_signage
