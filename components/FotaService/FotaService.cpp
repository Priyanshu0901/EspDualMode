#include "FotaService.h"

const char* FotaService::TAG = "FOTA_SERVICE";

FotaService::FotaService(const std::string& firmwareUrl) : firmwareUrl(firmwareUrl) {}

void FotaService::performUpdate() {
    ESP_LOGI(TAG, "Starting OTA update...");
    downloadAndUpdate();
}

void FotaService::downloadAndUpdate() {
    esp_http_client_config_t config = {
        .url = firmwareUrl.c_str(),
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_open(client, 0);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to connect to the server: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return;
    }

    esp_ota_handle_t ota_handle;
    const esp_partition_t* ota_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%lu",
             ota_partition->subtype, ota_partition->address);

    if (esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed");
        esp_http_client_cleanup(client);
        return;
    }

    int data_read;
    char buffer[1024];
    while ((data_read = esp_http_client_read(client, buffer, sizeof(buffer))) > 0) {
        if (esp_ota_write(ota_handle, buffer, data_read) != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_write failed");
            esp_ota_end(ota_handle);
            esp_http_client_cleanup(client);
            return;
        }
    }

    if (data_read < 0) {
        ESP_LOGE(TAG, "Error reading firmware");
    } else {
        if (esp_ota_end(ota_handle) == ESP_OK) {
            if (esp_ota_set_boot_partition(ota_partition) == ESP_OK) {
                ESP_LOGI(TAG, "OTA update complete. Rebooting...");
                esp_restart();
            } else {
                ESP_LOGE(TAG, "Failed to set boot partition");
            }
        } else {
            ESP_LOGE(TAG, "esp_ota_end failed");
        }
    }

    esp_http_client_cleanup(client);
}
