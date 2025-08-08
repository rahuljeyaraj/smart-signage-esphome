#pragma once
#include <ArduinoEigenDense.h>

namespace esphome::smart_signage::imu::hal {

class IImuHal {
  public:
    virtual ~IImuHal() = default;

    virtual bool init() = 0;
    virtual bool read() = 0;

    virtual Eigen::Vector3f getAccel() = 0;
    virtual Eigen::Vector3f getGyro()  = 0;
    virtual Eigen::Vector3f getMag()   = 0;
    virtual Eigen::Vector4f getQuat()  = 0;
    virtual float           getTemp()  = 0;
};
} // namespace esphome::smart_signage::imu