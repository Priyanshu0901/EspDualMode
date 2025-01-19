//spiffs_hal.c

#include "spiffs_hal.h"

void init_spiffs(){
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if(ret != ESP_OK){
        ESP_LOGE(ESP_SPIFFS_TAG, "%d\n", ret);
    }

    size_t total = 0, used = 0;
    esp_spiffs_info(conf.partition_label, &total, &used);
    ESP_LOGI(ESP_SPIFFS_TAG, "Partition size: total: %d, used: %d", total, used);

    if(esp_spiffs_mounted(conf.partition_label)){
        ESP_LOGI(ESP_SPIFFS_TAG,"Mounted");
    }

}


