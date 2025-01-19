#ifndef FOTA_SERVICE_H
#define FOTA_SERVICE_H

#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_log.h"
#include <string>

class FotaService {
public:
    FotaService(const std::string& firmwareUrl);
    void performUpdate();

private:
    std::string firmwareUrl;
    static const char* TAG;

    void downloadAndUpdate();
};

#endif // FOTA_SERVICE_H
