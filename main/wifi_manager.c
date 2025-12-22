#include "wifi_manager.h"

static const char *TAG = "WIFI_MGR";
static httpd_handle_t server = NULL;
static bool time_synced = false;

// Forward declarations
static void sync_time_from_ntp(void);
static void initialize_sntp(void);
static httpd_handle_t start_webserver(void);

// NTP sync
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
            ESP_LOGI(TAG, "Time synced from NTP!");
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

// Web server
static esp_err_t root_handler(httpd_req_t *req) {
    extern rtc_time_t g_current_time;
    
    const char* html = 
        "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>ESP32 Pomodoro Timer</title><style>"
        "*{margin:0;padding:0;box-sizing:border-box}"
        "body{font-family:'Segoe UI',sans-serif;background:linear-gradient(135deg,#1a1a2e 0%,#16213e 100%);"
        "color:#eee;min-height:100vh;display:flex;justify-content:center;align-items:center;padding:20px}"
        ".container{max-width:600px;width:100%;background:#16213e;padding:40px;border-radius:20px;"
        "box-shadow:0 10px 40px rgba(0,0,0,0.5)}"
        "h1{color:#e94560;margin-bottom:40px;font-size:2.5em;text-align:center}"
        "h2{color:#e94560;margin:30px 0 20px;font-size:1.5em;text-align:center}"
        ".time{font-size:72px;font-weight:bold;color:#e94560;text-align:center;margin:20px 0}"
        ".btn{padding:18px 45px;font-size:18px;border:none;border-radius:12px;cursor:pointer;"
        "margin:10px;background:linear-gradient(135deg,#27ae60 0%,#2ecc71 100%);color:white}"
        ".light-btn{background:linear-gradient(135deg,#f39c12 0%,#e67e22 100%)}"
        ".settings{background:#1a1a2e;padding:20px;border-radius:12px;margin:20px 0}"
        ".setting-row{display:flex;justify-content:space-between;align-items:center;margin:15px 0}"
        ".setting-row label{font-size:16px;color:#aaa}"
        ".setting-row input{width:80px;padding:8px;font-size:16px;border:2px solid #e94560;"
        "border-radius:8px;background:#16213e;color:#fff;text-align:center}"
        ".save-btn{width:100%;padding:15px;font-size:18px;background:linear-gradient(135deg,#e74c3c 0%,#c0392b 100%);"
        "color:white;border:none;border-radius:12px;cursor:pointer;margin-top:15px}"
        ".info{text-align:center;margin-top:20px;color:#aaa}"
        "</style></head><body><div class='container'>"
        "<h1>⏰ Pomodoro Timer</h1>"
        "<div class='time' id='time'>--:--:--</div>"
        "<div class='time' id='timer'>25:00</div>"
        "<div class='info' id='state'>Ready</div>"
        "<div style='text-align:center'>"
        "<button class='btn' onclick='fetch(\"/start\")'>▶️ Start/Stop</button>"
        "<button class='btn' onclick='fetch(\"/reset\")'>🔄 Reset</button><br>"
        "<button class='btn light-btn' onclick='fetch(\"/light\")'>💡 Đèn</button>"
        "</div>"
        "<div class='info' id='lightMode'>Đèn: Tắt</div>"
        
        "<h2>⚙️ Cài Đặt Thời Gian</h2>"
        "<div class='settings'>"
        "<div class='setting-row'>"
        "<label>⏱️ Thời gian làm việc (phút):</label>"
        "<input type='number' id='workTime' value='25' min='1' max='60'>"
        "</div>"
        "<div class='setting-row'>"
        "<label>☕ Thời gian nghỉ ngắn (phút):</label>"
        "<input type='number' id='breakTime' value='5' min='1' max='30'>"
        "</div>"
        "<div class='setting-row'>"
        "<label>🌴 Thời gian nghỉ dài (phút):</label>"
        "<input type='number' id='longBreakTime' value='15' min='1' max='60'>"
        "</div>"
        "<button class='save-btn' onclick='saveSettings()'>💾 Lưu Cài Đặt</button>"
        "</div>"
        
        "<script>"
        "function saveSettings(){"
        "const work=parseInt(document.getElementById('workTime').value);"
        "const brk=parseInt(document.getElementById('breakTime').value);"
        "const lng=parseInt(document.getElementById('longBreakTime').value);"
        "fetch('/setpomodoro',{method:'POST',headers:{'Content-Type':'application/json'},"
        "body:JSON.stringify({work:work,break:brk,longBreak:lng})})"
        ".then(r=>r.json()).then(d=>{alert(d.status=='ok'?'✓ Đã lưu!':'✗ Lỗi!')});"
        "}"
        "function loadSettings(){"
        "fetch('/getpomodoro').then(r=>r.json()).then(d=>{"
        "document.getElementById('workTime').value=d.work;"
        "document.getElementById('breakTime').value=d.break;"
        "document.getElementById('longBreakTime').value=d.longBreak;"
        "});"
        "}"
        "loadSettings();"
        "setInterval(()=>{"
        "fetch('/data').then(r=>r.json()).then(d=>{"
        "document.getElementById('time').innerText=d.time;"
        "document.getElementById('timer').innerText=d.timer;"
        "document.getElementById('state').innerText=d.state;"
        "document.getElementById('lightMode').innerText='Đèn: '+d.lightMode;"
        "})},1000)"
        "</script></div></body></html>";
    
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t data_handler(httpd_req_t *req) {
    extern rtc_time_t g_current_time;
    char json[512];
    pomodoro_t *pomo = get_pomodoro();
    
    uint8_t light_mode = touch_get_mode();
    const char* light_names[] = {"Tắt", "Trắng", "Vàng", "Vàng nhạt"};
    
    const char* state_name = "Ready";
    if (pomo->is_running) {
        if (pomo->state == POMODORO_WORK) {
            state_name = "💼 Work Time";
        } else if (pomo->state == POMODORO_BREAK) {
            state_name = "☕ Short Break";
        }
    }
    
    snprintf(json, sizeof(json),
        "{\"time\":\"%02d:%02d:%02d\","
        "\"timer\":\"%02d:%02d\","
        "\"running\":%s,"
        "\"state\":\"%s\","
        "\"lightMode\":\"%s\"}",
        g_current_time.hours, g_current_time.minutes, g_current_time.seconds,
        pomo->time_left / 60, pomo->time_left % 60,
        pomo->is_running ? "true" : "false",
        state_name,
        light_names[light_mode]
    );
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t start_handler(httpd_req_t *req) {
    pomodoro_start_stop();
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

static esp_err_t reset_handler(httpd_req_t *req) {
    pomodoro_reset();
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

static esp_err_t light_handler(httpd_req_t *req) {
    uint8_t current_mode = touch_get_mode();
    uint8_t next_mode = (current_mode + 1) > MODE ? 0 : (current_mode + 1);
    touch_set_mode(next_mode);
    
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

static esp_err_t getpomodoro_handler(httpd_req_t *req) {
    pomodoro_config_t *config = pomodoro_get_config();
    
    char json[256];
    snprintf(json, sizeof(json),
        "{\"work\":%d,\"break\":%d}",
        config->work_duration / 60,
        config->break_duration / 60
    );
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t setpomodoro_handler(httpd_req_t *req) {
    if (req->method == HTTP_OPTIONS) {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "POST, OPTIONS");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }
    
    char buf[256];
    int ret = httpd_req_recv(req, buf, req->content_len);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = '\0';
    
    int work = 25, brk = 5, lng = 15;
    char *ptr;
    
    if ((ptr = strstr(buf, "\"work\":"))) work = atoi(ptr + 7);
    if ((ptr = strstr(buf, "\"break\":"))) brk = atoi(ptr + 8);
    if ((ptr = strstr(buf, "\"longBreak\":"))) lng = atoi(ptr + 12);
    
    if (work < 1 || work > 60) work = 25;
    if (brk < 1 || brk > 30) brk = 5;
    if (lng < 1 || lng > 60) lng = 15;
    
    esp_err_t err = pomodoro_set_durations(work * 60, brk * 600);
    
    ESP_LOGI(TAG, "Pomodoro settings: Work=%dm, Break=%dm", work, brk);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, err == ESP_OK ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}", 
                   HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t settime_handler(httpd_req_t *req) {
    if (req->method == HTTP_OPTIONS) {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "POST, OPTIONS");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
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
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, err == ESP_OK ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}", 
                   HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 15;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &root);
        
        httpd_uri_t data = {
            .uri = "/data",
            .method = HTTP_GET,
            .handler = data_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &data);
        
        httpd_uri_t start = {
            .uri = "/start",
            .method = HTTP_GET,
            .handler = start_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &start);
        
        httpd_uri_t reset = {
            .uri = "/reset",
            .method = HTTP_GET,
            .handler = reset_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &reset);
        
        httpd_uri_t light = {
            .uri = "/light",
            .method = HTTP_GET,
            .handler = light_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &light);
        
        httpd_uri_t getpomo = {
            .uri = "/getpomodoro",
            .method = HTTP_GET,
            .handler = getpomodoro_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &getpomo);
        
        httpd_uri_t setpomo_post = {
            .uri = "/setpomodoro",
            .method = HTTP_POST,
            .handler = setpomodoro_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &setpomo_post);
        
        httpd_uri_t setpomo_opt = {
            .uri = "/setpomodoro",
            .method = HTTP_OPTIONS,
            .handler = setpomodoro_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &setpomo_opt);
        
        httpd_uri_t settime_post = {
            .uri = "/settime",
            .method = HTTP_POST,
            .handler = settime_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &settime_post);
        
        httpd_uri_t settime_opt = {
            .uri = "/settime",
            .method = HTTP_OPTIONS,
            .handler = settime_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &settime_opt);
        
        ESP_LOGI(TAG, "Web server started");
        return server;
    }
    return NULL;
}

// Wifi event handler
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