#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "main.h"
#include "clock.h"
#include "display.h"
#include "oled.h"


// ===== TIMEZONE CONFIGURATION =====
#define TIMEZONE       "UTC-7"  // Vietnam (UTC+7)

esp_err_t wifi_manager_init(void);
bool wifi_is_time_synced(void);

#endif