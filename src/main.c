#include <string.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "esp_insights.h"

#include <app_wifi.h>
#include <app_reset.h>
#include <app_insights.h>

#include "esp_rmaker_utils.h"

#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>

#include "garage_control.h"

esp_rmaker_device_t *garage_device;

static esp_err_t garage_rainmaker_callback(const esp_rmaker_device_t *device,
    const esp_rmaker_param_t *param, const esp_rmaker_param_val_t val,
    void *priv_data,esp_rmaker_write_ctx_t *ctx) {
    if (timer_enabled[SW_BUTTON_COOLDOWN_TIMER])
        return ESP_OK;

    if (strcmp(esp_rmaker_param_get_name(param), "Button") == 0) {
        if (val.val.b) {
            timer_enabled[SW_BUTTON_COOLDOWN_TIMER] = true;
        }
    }

    return ESP_OK;
}

void update_status(char *state) {
    ESP_DIAG_EVENT(TAG, "%s", state);
    esp_rmaker_param_t* status = esp_rmaker_device_get_param_by_name(garage_device, "Status");
    esp_rmaker_param_update_and_report(status, esp_rmaker_str(state));
}

void send_notification(char *message) {
    ESP_DIAG_EVENT(TAG, "%s", message);
    ESP_LOGI(TAG, "Sending notification: {%s}", message);
    //esp_rmaker_raise_alert(message);
}

void app_main() {
    esp_err_t err = garage_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not initialize board. Aborting!!!");
        abort();
    }

    app_reset_button_register( app_reset_button_create(9, 0), 3, 10);

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    app_wifi_init();

    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg,
        "ESP RainMaker Garage Opener", "Garage Opener");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(1000/portTICK_PERIOD_MS);
        abort();
    }

    garage_device = esp_rmaker_device_create("Garage", NULL, NULL);
    if (!garage_device) {
        ESP_LOGE(TAG, "Could not create rainmaker device. Aborting!!!");
        vTaskDelay(1000/portTICK_PERIOD_MS);
        abort();
    }
    esp_rmaker_device_add_cb(garage_device, garage_rainmaker_callback, NULL);
    esp_rmaker_device_add_param(garage_device,
        esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, "Garage"));

    esp_rmaker_param_t *garage_status = esp_rmaker_param_create("Status", NULL,
        esp_rmaker_str(garage_state_name[garage_state]), PROP_FLAG_READ);
    esp_rmaker_param_add_ui_type(garage_status, ESP_RMAKER_UI_TEXT);

    esp_rmaker_param_t *garage_param = esp_rmaker_param_create("Button", NULL,
        esp_rmaker_bool(false), PROP_FLAG_READ | PROP_FLAG_WRITE);
    esp_rmaker_param_add_ui_type(garage_param, ESP_RMAKER_UI_TRIGGER);

    esp_rmaker_device_add_param(garage_device, garage_status);
    esp_rmaker_device_add_param(garage_device, garage_param);
    
    esp_rmaker_device_assign_primary_param(garage_device, garage_param);

    esp_rmaker_node_add_device(node, garage_device);

    esp_rmaker_ota_enable_default();

    /* Enable Insights. Requires CONFIG_ESP_INSIGHTS_ENABLED=y */
    app_insights_enable();

    esp_rmaker_start();

    err = app_wifi_start(POP_TYPE_RANDOM);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }
}