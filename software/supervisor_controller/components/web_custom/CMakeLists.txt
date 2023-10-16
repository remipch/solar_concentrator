set(embed_files "./www/index_ov2640.html.gz")

set(requires
        esp32-camera    # (direct access to camera sensor parameters)
        esp_http_server
        nvs_flash
        esp_wifi
        camera  #TODO: remove ?
        motors
        supervisor)

idf_component_register(SRC_DIRS . INCLUDE_DIRS . REQUIRES ${requires} EMBED_FILES ${embed_files})

component_compile_options(-ffast-math -O3)

