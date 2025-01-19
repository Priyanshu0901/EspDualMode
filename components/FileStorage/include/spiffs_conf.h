#ifndef SPIFFS_CONF_H
#define SPIFFS_CONF_H

// #include <sys/unistd.h>
// #include <sys/stat.h>
// #include <dirent.h>
#include "esp_spiffs.h"
#include "esp_log.h"

extern esp_vfs_spiffs_conf_t conf;

extern const char rec_file[22];

#define ESP_SPIFFS_TAG      "ESP_SPIFFS"
#endif //SPIFFS_CONF_H