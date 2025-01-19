idf_component_register(SRCS "FotaService.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_client app_update Wifi_Manager
                    EMBED_TXTFILES "../../certificates/github_server_cert.pem")
