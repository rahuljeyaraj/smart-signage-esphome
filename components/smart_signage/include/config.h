// #pragma once
// #include <cstdint>
// #include "nvs_flash.h"
// #include "nvs.h"

// /* ----------------------------------------------------------------------
//  * A single macro expands to:
//  *   static bool get_<NAME>(ns, &out)
//  *   static bool set_<NAME>(ns, value)
//  * The NVS key is just #NAME as a string.
//  * --------------------------------------------------------------------*/
// #define CONFIG_FIELD(NAME, TYPE, NVS_GET, NVS_SET) \
//     static bool get_##NAME(const char *ns, TYPE &v) { \
//         nvs_handle_t h; \
//         if (nvs_open(ns, NVS_READONLY, &h) != ESP_OK) return false; \
//         esp_err_t e = NVS_GET(h, #NAME, &v); \
//         nvs_close(h); \
//         return e == ESP_OK; \
//     } \
//     static bool set_##NAME(const char *ns, TYPE v) { \
//         nvs_handle_t h; \
//         if (nvs_open(ns, NVS_READWRITE, &h) != ESP_OK) return false; \
//         esp_err_t e = NVS_SET(h, #NAME, v); \
//         nvs_commit(h); \
//         nvs_close(h); \
//         return e == ESP_OK; \
//     }

// /* ---------- Config class: macro is even simpler ---------------------- */
// class Config {
//   public:
//     CONFIG_FIELD(radarDist, uint16_t, nvs_get_u16, nvs_set_u16)
//     CONFIG_FIELD(radarSampInt, uint32_t, nvs_get_u32, nvs_set_u32)
//     CONFIG_FIELD(imuFallAng, uint16_t, nvs_get_u16, nvs_set_u16)
//     CONFIG_FIELD(imuConfCnt, uint16_t, nvs_get_u16, nvs_set_u16)
//     CONFIG_FIELD(imuSampInt, uint32_t, nvs_get_u32, nvs_set_u32)
//     CONFIG_FIELD(ledBright, uint8_t, nvs_get_u8, nvs_set_u8)
//     CONFIG_FIELD(audioVol, uint8_t, nvs_get_u8, nvs_set_u8)

//     CONFIG_FIELD(radarDist, uint16_t, nvs_get_u16, nvs_set_u16)
//     CONFIG_FIELD(radarSampInt, uint32_t, nvs_get_u32, nvs_set_u32)
//     CONFIG_FIELD(imuFallAng, uint16_t, nvs_get_u16, nvs_set_u16)
//     CONFIG_FIELD(imuConfCnt, uint16_t, nvs_get_u16, nvs_set_u16)
//     CONFIG_FIELD(imuSampInt, uint32_t, nvs_get_u32, nvs_set_u32)
//     CONFIG_FIELD(ledBright, uint8_t, nvs_get_u8, nvs_set_u8)
//     CONFIG_FIELD(audioVol, uint8_t, nvs_get_u8, nvs_set_u8)
// };

// /* Clean up macro */
// #undef CONFIG_FIELD