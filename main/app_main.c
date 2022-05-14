/* MQTT over SSL Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"
#include "tuya_cacert.h"
#include "tuya_log.h"
#include "tuya_error_code.h"
#include "system_interface.h"
#include "mqtt_client_interface.h"
#include "tuyalink_core.h"

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "relay_ctl.h"

static const char *TAG = "TUYA_LINK_EXAMPLE";

const char *productId = CONFIG_PRODUCT_ID;
const char *deviceId = CONFIG_DEVICE_ID;
const char *deviceSecret = CONFIG_DEVICE_SECRET;

tuya_mqtt_context_t client_instance;

void on_connected(tuya_mqtt_context_t* context, void* user_data)
{
    TY_LOGI("on connected");
    char data[100];
    sprintf(data, "[{\"deviceId\":\"%s\",\"Payload\":\"Hello\"}]", deviceId);
    tuyalink_subdevice_bind(context, data);
    tuyalink_subdevice_topo_get(context);
}

void on_disconnect(tuya_mqtt_context_t* context, void* user_data)
{
    TY_LOGI("on disconnect");
}

void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
    TY_LOGI("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
    if(msg->data_string) {
        switch (msg->type) {
            case THING_TYPE_PROPERTY_SET:
                // cJSON* dps = msg->data_json->string;                
                // char *dps = msg->data_json->string;              
                TY_LOGI("set payload:%s\r\n", msg->data_string);
                cJSON* dps = cJSON_Parse(msg->data_string);
                if (dps == NULL) {
                    TY_LOGE("JSON parsing error, exit!");
                    return;
                }
                /* Process dp data */
                cJSON* switch_obj = cJSON_GetObjectItem(dps, SWITCH_DP_ID_KEY);
                if (cJSON_IsTrue(switch_obj)) {
                    set_relay_status(RELAY_PIN , true);
                } else if (cJSON_IsFalse(switch_obj)) {
                    set_relay_status(RELAY_PIN , false);
                }
                /* relese cJSON DPS object */
                cJSON_Delete(dps);
                // cJSON_Delete(switch_obj);
                break;
            case THING_TYPE_DEVICE_SUB_BIND_RSP:
                TY_LOGI("bind response:%s\r\n", msg->data_string);
                break;
            case THING_TYPE_DEVICE_TOPO_GET_RSP:
                TY_LOGI("get topo response:%s\r\n", msg->data_string);
                break;
            default:
                break;
        }
    }
    printf("\r\n");
}

static void tuya_link_app_task(void *pvParameters)
{
    int ret = OPRT_OK;

    tuya_mqtt_context_t* client = &client_instance;

    ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t) {
        .host = "m2.tuyacn.com",
        .port = 8883,
        .cacert = (const uint8_t *)tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = deviceId,
        .device_secret = deviceSecret,
        .keepalive = 60,
        .timeout_ms = 2000,
        .on_connected = on_connected,
        .on_disconnect = on_disconnect,
        .on_messages = on_messages
    });
    assert(ret == OPRT_OK);

    ret = tuya_mqtt_connect(client);
    assert(ret == OPRT_OK);

    for (;;) {
        /* Loop to receive packets, and handles client keepalive */
        tuya_mqtt_loop(client);
    }

    return ret;
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    init_relay_io(RELAY_PIN);

    xTaskCreate(tuya_link_app_task, "tuya_link", 1024 * 6, NULL, 4, NULL);
}
