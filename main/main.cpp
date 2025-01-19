#include "esp_log.h"

#include "WiFi_Manager.h"
#include "FotaService.h"

extern "C"
{
#include "spiffs_mngr.h"
}

const char *main_tag = "main";

extern "C" void app_main()
{
  init_spiffs();

  WiFiManager wifiManager;

  // Initialize Wi-Fi
  wifiManager.init();

  ESP_LOGI(main_tag, "STA");
  // Start STA mode (replace with your credentials)
  wifiManager.startSTA("myCrib", "8697017290");

  ESP_LOGI(main_tag, "Going for FOTA");

  std::string firmwareUrl = "https://github.com/Priyanshu0901/EspDualMode/releases/download/v0.0.0/EspDualMode.bin";
  FotaService fotaService(firmwareUrl);
  fotaService.performUpdate();

  ESP_LOGI(main_tag, "AP");
  // Start AP mode
  wifiManager.startAP("ESP32_AP", "12345678");

  wifiManager.checkInternetConnection();
}
