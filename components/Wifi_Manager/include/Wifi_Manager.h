#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <string>
#include "esp_event.h"

class WiFiManager
{
public:
  WiFiManager();
  void init(); // Initialize Wi-Fi
  void startSTA(const std::string &ssid, const std::string &password);
  void startAP(const std::string &ssid, const std::string &password);
  void checkInternetConnection(); // Check for internet connectivity

private:
  static void eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
  void configureSTA(const std::string &ssid, const std::string &password);
  void configureAP(const std::string &ssid, const std::string &password);
};

#endif // WIFI_MANAGER_H
