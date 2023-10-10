set(embed_files "./www/index_ov2640.html.gz")

set(requires
        esp32-camera
        esp_http_server
        nvs_flash
        esp_wifi
        camera_custom
        image_processing
        motors)

idf_component_register(SRC_DIRS . INCLUDE_DIRS . REQUIRES ${requires} EMBED_FILES ${embed_files})

component_compile_options(-ffast-math -O3)

