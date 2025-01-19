#include "WiFi_Manager.h"

extern "C" {
  #include "spiffs_mngr.h"
}

extern "C" void app_main()
{
  init_spiffs();

  WiFiManager wifiManager;

  // Initialize Wi-Fi
  wifiManager.init();

  // Start STA mode (replace with your credentials)
  wifiManager.startSTA("myCrib", "8697017290");

  // Start AP mode
  wifiManager.startAP("ESP32_AP", "12345678");

  wifiManager.checkInternetConnection();
}
