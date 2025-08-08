#pragma once
#include "FastIMU.h"
#include "iimu_hal.h"
#include "log.h"

namespace esphome::smart_signage::imu::hal {

class I2cImuHal : public IImuHal {
  public:
    I2cImuHal(IMUBase &imu, TwoWire &wire, uint8_t address, int sda_pin, int scl_pin,
        const calData &calib = calData{})
        : imu_(imu), wire_(wire), address_(address), sda_pin_(sda_pin), scl_pin_(scl_pin),
          calib_(calib), isInitDone_(false) {}

    bool init() override {
        wire_.begin(sda_pin_, scl_pin_);
        if (imu_.init(calib_, address_) == 0) {
            isInitDone_ = true;
            LOGD("IMU init succeeded (addr=0x%02X)", address_);
        } else {
            isInitDone_ = false;
            LOGE("IMU init FAILED (addr=0x%02X)", address_);
        }
        return isInitDone_;
    }

    bool read() override {
        if (!isInitDone_) return false;
        imu_.update();
        return true;
    }

    Eigen::Vector3f getAccel() override {
        if (!isInitDone_) return Eigen::Vector3f::Zero();
        AccelData in;
        imu_.getAccel(&in);
        return Eigen::Vector3f(in.accelX, in.accelY, in.accelZ);
    }

    Eigen::Vector3f getGyro() override {
        if (!isInitDone_) return Eigen::Vector3f::Zero();
        GyroData in;
        imu_.getGyro(&in);
        return Eigen::Vector3f(in.gyroX, in.gyroY, in.gyroZ);
    }

    Eigen::Vector3f getMag() override {
        if (!isInitDone_ || !imu_.hasMagnetometer()) return Eigen::Vector3f::Zero();
        MagData in;
        imu_.getMag(&in);
        return Eigen::Vector3f(in.magX, in.magY, in.magZ);
    }

    Eigen::Vector4f getQuat() override {
        if (!isInitDone_ || !imu_.hasQuatOutput()) return Eigen::Vector4f::Zero();
        Quaternion in;
        imu_.getQuat(&in);
        return Eigen::Vector4f(in.qW, in.qX, in.qY, in.qZ);
    }

    float getTemp() override {
        if (!isInitDone_ || !imu_.hasTemperature()) return NAN;
        return imu_.getTemp();
    }

  private:
    IMUBase &imu_;
    TwoWire &wire_;
    uint8_t  address_;
    int      sda_pin_;
    int      scl_pin_;
    calData  calib_;
    bool     isInitDone_{false};

    static constexpr char TAG[] = "ImuHal";
};

} // namespace esphome::smart_signage::imu::hal
