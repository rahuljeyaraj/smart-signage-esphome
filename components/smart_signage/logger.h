#pragma once

#include "esphome/core/log.h"

// Wrapper macros
#define LOGD(tag, fmt, ...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)      /**< Debug log */
#define LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)      /**< Info  log */
#define LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)      /**< Warn  log */
#define LOGE(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)      /**< Error log */
#define LOGC(tag, fmt, ...) ESP_LOGCONFIG(tag, fmt, ##__VA_ARGS__) /**< Config log */
