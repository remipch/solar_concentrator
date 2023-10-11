set(requires
        camera_custom
        esp32-camera
        capstone_detector)

idf_component_register(SRC_DIRS . INCLUDE_DIRS . REQUIRES ${requires})

component_compile_options(-ffast-math -O3)

