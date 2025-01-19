#include "Wifi_Manager.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "cstring"
#include "esp_ping.h"
#include "ping/ping_sock.h"

static const char *TAG = "WiFiManager";

EventGroupHandle_t wifi_event_group;

WiFiManager::WiFiManager()
{
  wifi_event_group = xEventGroupCreate();
}

void WiFiManager::init()
{
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Initialize TCP/IP and Wi-Fi
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();
  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &eventHandler, this, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &eventHandler, this, &instance_any_id));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
}

void WiFiManager::startSTA(const std::string &ssid, const std::string &password)
{
  configureSTA(ssid, password);
  ESP_LOGI(TAG, "Starting Wi-Fi STA...");
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());

  esp_netif_dns_info_t dns_info;
  ESP_ERROR_CHECK(esp_netif_get_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_MAIN, &dns_info));
  ESP_LOGI(TAG, "DNS Server: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));

  esp_netif_dns_info_t new_dns;
  IP4_ADDR(&new_dns.ip.u_addr.ip4, 8, 8, 8, 8); // Set to Google's DNS server
  new_dns.ip.type = ESP_IPADDR_TYPE_V4;

  ESP_ERROR_CHECK(esp_netif_set_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_MAIN, &new_dns));

  ESP_ERROR_CHECK(esp_netif_get_dns_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), ESP_NETIF_DNS_MAIN, &dns_info));
  ESP_LOGI(TAG, "DNS Server: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
}

void WiFiManager::startAP(const std::string &ssid, const std::string &password)
{
  configureAP(ssid, password);
  ESP_LOGI(TAG, "Starting Wi-Fi AP...");
  ESP_ERROR_CHECK(esp_wifi_start());
}

void WiFiManager::configureSTA(const std::string &ssid, const std::string &password)
{
  wifi_config_t sta_config = {};
  strncpy((char *)sta_config.sta.ssid, ssid.c_str(), sizeof(sta_config.sta.ssid) - 1);
  strncpy((char *)sta_config.sta.password, password.c_str(), sizeof(sta_config.sta.password) - 1);
  sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
}

void WiFiManager::configureAP(const std::string &ssid, const std::string &password)
{
  wifi_config_t ap_config = {};
  strncpy((char *)ap_config.ap.ssid, ssid.c_str(), sizeof(ap_config.ap.ssid) - 1);
  strncpy((char *)ap_config.ap.password, password.c_str(), sizeof(ap_config.ap.password) - 1);
  ap_config.ap.ssid_len = ssid.length();
  ap_config.ap.max_connection = 10; // Max 10 clients
  ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

  if (password.empty())
  {
    ap_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
}

void WiFiManager::eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT)
  {
    if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
      ESP_LOGI(TAG, "Wi-Fi STA connected.");
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
      ESP_LOGI(TAG, "Wi-Fi STA disconnected.");
      xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    }
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
  }
}

void WiFiManager::checkInternetConnection()
{
  ESP_LOGI(TAG, "Checking Wi-Fi and internet connection...");
  const TickType_t max_wait = pdMS_TO_TICKS(10000); // 10-second timeout
  TickType_t start_tick = xTaskGetTickCount();

  while (!(xEventGroupGetBits(wifi_event_group) & CONNECTED_BIT))
  {
    if ((xTaskGetTickCount() - start_tick) > max_wait)
    {
      ESP_LOGE(TAG, "Wi-Fi not connected within timeout.");
      return;
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // Check every 100ms
  }
  ESP_LOGI(TAG, "Wi-Fi connected. Checking internet connectivity...");

  // Initialize Ping configuration
  esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
  ip4_addr_t target_ip;
  ip4addr_aton("8.8.8.8", &target_ip);                                  // Convert string to IP
  ping_config.target_addr = *reinterpret_cast<ip_addr_t *>(&target_ip); // Assign to target_addr

  // Ping event callback
  esp_ping_callbacks_t cbs = {
      .cb_args = nullptr,
      .on_ping_success = [](esp_ping_handle_t hdl, void *args)
      {
            uint32_t elapsed_time;
            esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
            ESP_LOGI(TAG, "Ping success! RTT: %lu ms", elapsed_time); },
      .on_ping_timeout = nullptr,
      .on_ping_end = [](esp_ping_handle_t hdl, void *args)
      {
            ESP_LOGI(TAG, "Ping complete.");
            esp_ping_stop(hdl);
            esp_ping_delete_session(hdl); }};

  // Create and start a ping session
  esp_ping_handle_t ping_handle;
  ESP_ERROR_CHECK(esp_ping_new_session(&ping_config, &cbs, &ping_handle));
  ESP_ERROR_CHECK(esp_ping_start(ping_handle));
}
