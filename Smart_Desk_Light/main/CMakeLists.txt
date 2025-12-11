idf_component_register(SRCS "blink_example_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES rtc driver
                       esp_wifi
        esp_event
        esp_netif
        esp_http_server
        nvs_flash
        driver
        json)
