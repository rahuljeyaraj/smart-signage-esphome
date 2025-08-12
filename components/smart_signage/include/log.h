#pragma once

#include "esphome/core/log.h"

// Wrapper macros that pick up the TAG from the local scope:
#define SS_LOGD(fmt, ...) ESP_LOGD(TAG, fmt, ##__VA_ARGS__)      /**< Debug log */
#define SS_LOGI(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)      /**< Info  log */
#define SS_LOGW(fmt, ...) ESP_LOGW(TAG, fmt, ##__VA_ARGS__)      /**< Warn  log */
#define SS_LOGE(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)      /**< Error log */
#define SS_LOGC(fmt, ...) ESP_LOGCONFIG(TAG, fmt, ##__VA_ARGS__) /**< Config log */
