#pragma once

#include "esphome/core/log.h"

// Expect each .cpp to define, for example:
//   static constexpr char TAG[] = "ctrl";
//   static constexpr char TAG[] = "radar";
// before including this header.

// Wrapper macros that pick up the TAG from the local scope:
#define LOGD(fmt, ...) ESP_LOGD(TAG, fmt, ##__VA_ARGS__)      /**< Debug log */
#define LOGI(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)      /**< Info  log */
#define LOGW(fmt, ...) ESP_LOGW(TAG, fmt, ##__VA_ARGS__)      /**< Warn  log */
#define LOGE(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)      /**< Error log */
#define LOGC(fmt, ...) ESP_LOGCONFIG(TAG, fmt, ##__VA_ARGS__) /**< Config log */

#define LOGDT(tag, fmt, ...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)      /**< Debug log */
#define LOGIT(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)      /**< Info  log */
#define LOGWT(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)      /**< Warn  log */
#define LOGET(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)      /**< Error log */
#define LOGCT(tag, fmt, ...) ESP_LOGCONFIG(tag, fmt, ##__VA_ARGS__) /**< Config log */
