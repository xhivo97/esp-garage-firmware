idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS
    REQUIRES nvs_flash esp_rainmaker app_reset app_wifi app_insights esp_insights)
