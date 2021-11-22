#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_event.h>
#include <nvs_flash.h>

#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_ota.h>
#include <esp_rmaker_schedule.h>
#include <esp_rmaker_console.h>

#include <esp_rmaker_common_events.h>

#include <app_reset.h>
#include <app_wifi.h>
#include <app_insights.h>

#include "garage_opener.h"

#define BOOT_GPIO           9
#define BUTTON_ACTIVE_LEVEL 0
#define WIFI_RESET_BUTTON_TIMEOUT       3
#define FACTORY_RESET_BUTTON_TIMEOUT    10

static const char *TAG = "app_main";

esp_rmaker_device_t *garage_device;

static esp_err_t test_callback(
    const esp_rmaker_device_t *device,
    const esp_rmaker_param_t *param,
    const esp_rmaker_param_val_t val,
    void *priv_data,
    esp_rmaker_write_ctx_t *ctx
)
{
    if (ctx) {
        ESP_LOGI(
            TAG,
            "Received write request via : %s",
            esp_rmaker_device_cb_src_to_str(ctx->src)
        );
    }
    if (strcmp(esp_rmaker_param_get_name(param), "Button") == 0) {
        ESP_LOGI(
            TAG,
            "Received value : %s for %s - %s",
            val.val.b ? "true" : "false",
            esp_rmaker_device_get_name(device),
            esp_rmaker_param_get_name(param)
        );
        queue_state_machine();
    }
    return ESP_OK;
}

void update_status() {
    esp_rmaker_param_t* status = esp_rmaker_device_get_param_by_name(garage_device, "Status");
    esp_rmaker_param_update_and_report(status, esp_rmaker_str(get_garage_state()));
}

void door_open_notification() {
    char* minute = GARAGE_OPEN_ALERT_MINUTES > 1 ? "minutes" : "minute";
    char status[128];
    sprintf(
        status,
        "Garage has been open for %d %s\n",
        GARAGE_OPEN_ALERT_MINUTES,
        minute
    );
    esp_rmaker_raise_alert(status);
}

void app_main(void)
{
    setup_gpios();
    app_reset_button_register(
        app_reset_button_create(BOOT_GPIO, BUTTON_ACTIVE_LEVEL),
        WIFI_RESET_BUTTON_TIMEOUT,
        FACTORY_RESET_BUTTON_TIMEOUT
    );
    /* Initialize NVS. */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    /* Initialize Wi-Fi. Note that, this should be called before esp_rmaker_node_init()
     */
    app_wifi_init();

    /* Initialize the ESP RainMaker Agent.
     * Note that this should be called after app_wifi_init() but before app_wifi_start()
     * */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "ESP RainMaker Device", "Temperature Sensor");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

    garage_device = esp_rmaker_device_create("Garage", NULL, NULL);
    esp_rmaker_device_add_cb(garage_device, test_callback, NULL);
    esp_rmaker_device_add_param(garage_device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, "Garage"));

    esp_rmaker_param_t *garage_status = esp_rmaker_param_create(
        "Status",
        NULL,
        esp_rmaker_str(get_garage_state()),
        PROP_FLAG_READ
    );
    if (garage_status) {
        esp_rmaker_param_add_ui_type(garage_status, ESP_RMAKER_UI_TEXT);
    }
    esp_rmaker_device_add_param(garage_device, garage_status);

    esp_rmaker_param_t *garage_param = esp_rmaker_param_create(
        "Button",
        NULL,
        esp_rmaker_bool(false),
        PROP_FLAG_READ | PROP_FLAG_WRITE
    );
    if (garage_param) {
        esp_rmaker_param_add_ui_type(garage_param, "esp.ui.trigger");
    }
    esp_rmaker_device_add_param(garage_device, garage_param);
    esp_rmaker_device_assign_primary_param(garage_device, garage_param);

    esp_rmaker_node_add_device(node, garage_device);

    /* Enable OTA */
    esp_rmaker_ota_config_t ota_config = {
        .server_cert = ESP_RMAKER_OTA_DEFAULT_SERVER_CERT,
    };
    esp_rmaker_ota_enable(&ota_config, OTA_USING_PARAMS);

    /* Enable Insights. Requires CONFIG_ESP_INSIGHTS_ENABLED=y */
    app_insights_enable();

    /* Start the ESP RainMaker Agent */
    esp_rmaker_start();

    /* Start the Wi-Fi.
     * If the node is provisioned, it will start connection attempts,
     * else, it will start Wi-Fi provisioning. The function will return
     * after a connection has been successfully established
     */
    err = app_wifi_start(POP_TYPE_RANDOM);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }
}
