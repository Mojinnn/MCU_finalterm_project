#include "wifi_manager.h"

static const char *TAG = "WIFI_MGR";
static httpd_handle_t server = NULL;
static bool time_synced = false;

// ===== FORWARD DECLARATIONS =====
static void sync_time_from_ntp(void);
static void initialize_sntp(void);
static httpd_handle_t start_webserver(void);

// ===== NTP SYNC =====
static void sync_time_from_ntp(void) {
    ESP_LOGI(TAG, "Waiting for NTP time sync...");
    
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 15;
    
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
    time(&now);
    localtime_r(&now, &timeinfo);
    
    if (timeinfo.tm_year > (2024 - 1900)) {
        rtc_time_t new_time = {
            .seconds = timeinfo.tm_sec,
            .minutes = timeinfo.tm_min,
            .hours = timeinfo.tm_hour,
            .day = timeinfo.tm_wday + 1,
            .date = timeinfo.tm_mday,
            .month = timeinfo.tm_mon + 1,
            .year = timeinfo.tm_year + 1900
        };
        
        if (clock_set_time(&new_time) == ESP_OK) {
            ESP_LOGI(TAG, "✓ Time synced from NTP!");
            time_synced = true;
        }
    }
}

static void time_sync_notification_cb(struct timeval *tv) {
    sync_time_from_ntp();
}

static void initialize_sntp(void) {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    setenv("TZ", TIMEZONE, 1);
    tzset();
    esp_sntp_init();
}

// ===== WEB SERVER =====
static esp_err_t root_handler(httpd_req_t *req) {
    extern rtc_time_t g_current_time;  // From display.c
    
    const char* html_part1 = 
        "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>ESP32 Pomodoro Timer</title><style>"
        "*{margin:0;padding:0;box-sizing:border-box}"
        "body{font-family:'Segoe UI',sans-serif;background:linear-gradient(135deg,#1a1a2e 0%,#16213e 100%);"
        "color:#eee;min-height:100vh;display:flex;justify-content:center;align-items:center;padding:20px}"
        ".container{max-width:600px;width:100%;background:#16213e;padding:40px;border-radius:20px;"
        "box-shadow:0 10px 40px rgba(0,0,0,0.5)}"
        "h1{color:#e94560;margin-bottom:40px;font-size:2.5em;text-align:center}"
        ".time{font-size:72px;font-weight:bold;color:#e94560;text-align:center;margin:20px 0}"
        ".btn{padding:18px 45px;font-size:18px;border:none;border-radius:12px;cursor:pointer;"
        "margin:10px;background:linear-gradient(135deg,#27ae60 0%,#2ecc71 100%);color:white}"
        "</style></head><body><div class='container'>"
        "<h1>⏰ Pomodoro Timer</h1>"
        "<div class='time' id='time'>--:--:--</div>"
        "<div class='time' id='timer'>25:00</div>"
        "<button class='btn' onclick='fetch(\"/start\")'>▶️ Start/Stop</button>"
        "<button class='btn' onclick='fetch(\"/reset\")'>🔄 Reset</button>"
        "<script>setInterval(()=>{"
        "fetch('/data').then(r=>r.json()).then(d=>{"
        "document.getElementById('time').innerText=d.time;"
        "document.getElementById('timer').innerText=d.timer;"
        "})},1000)</script></div></body></html>";
    
    httpd_resp_send(req, html_part1, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t data_handler(httpd_req_t *req) {
    extern rtc_time_t g_current_time;
    char json[256];
    pomodoro_t *pomo = get_pomodoro();
    
    snprintf(json, sizeof(json),
        "{\"time\":\"%02d:%02d:%02d\","
        "\"timer\":\"%02d:%02d\","
        "\"running\":%s}",
        g_current_time.hours, g_current_time.minutes, g_current_time.seconds,
        pomo->time_left / 60, pomo->time_left % 60,
        pomo->is_running ? "true" : "false"
    );
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t start_handler(httpd_req_t *req) {
    pomodoro_start_stop();
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

static esp_err_t reset_handler(httpd_req_t *req) {
    pomodoro_reset();
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

static esp_err_t settime_handler(httpd_req_t *req) {
    if (req->method == HTTP_OPTIONS) {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }
    
    char buf[200];
    int ret = httpd_req_recv(req, buf, req->content_len);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = '\0';
    
    int h=0, m=0, s=0, date=1, month=1, year=2024;
    char *ptr;
    
    if ((ptr = strstr(buf, "\"h\":"))) h = atoi(ptr + 4);
    if ((ptr = strstr(buf, "\"m\":"))) m = atoi(ptr + 4);
    if ((ptr = strstr(buf, "\"s\":"))) s = atoi(ptr + 4);
    if ((ptr = strstr(buf, "\"d\":"))) date = atoi(ptr + 4);
    if ((ptr = strstr(buf, "\"mo\":"))) month = atoi(ptr + 5);
    if ((ptr = strstr(buf, "\"y\":"))) year = atoi(ptr + 4);
    
    rtc_time_t new_time = {s, m, h, 1, date, month, year};
    esp_err_t err = clock_set_time(&new_time);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, err == ESP_OK ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}", 
                   HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t routes[] = {
            {.uri = "/", .method = HTTP_GET, .handler = root_handler},
            {.uri = "/data", .method = HTTP_GET, .handler = data_handler},
            {.uri = "/start", .method = HTTP_GET, .handler = start_handler},
            {.uri = "/reset", .method = HTTP_GET, .handler = reset_handler},
            {.uri = "/settime", .method = HTTP_POST, .handler = settime_handler},
            {.uri = "/settime", .method = HTTP_OPTIONS, .handler = settime_handler}
        };
        
        for (int i = 0; i < 6; i++) {
            httpd_register_uri_handler(server, &routes[i]);
        }
        
        ESP_LOGI(TAG, "Web server started");
        return server;
    }
    return NULL;
}

// ===== WIFI EVENT HANDLER =====
static void wifi_event_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        initialize_sntp();
        start_webserver();
    }
}

// ===== PUBLIC FUNCTIONS =====
esp_err_t wifi_manager_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                    &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                    &wifi_event_handler, NULL, NULL));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi initialized");
    return ESP_OK;
}

bool wifi_is_time_synced(void) {
    return time_synced;
}