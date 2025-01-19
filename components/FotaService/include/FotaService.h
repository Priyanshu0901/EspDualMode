#ifndef FOTA_SERVICE_H
#define FOTA_SERVICE_H

#include <string>

class FotaService
{
public:
  FotaService(const std::string &firmwareUrl);
  void performUpdate();

private:
  std::string firmwareUrl;
  static const char *TAG;

  void downloadAndUpdate();
};

#endif // FOTA_SERVICE_H
