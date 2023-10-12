set(requires
        esp32-camera)

idf_component_register(SRC_DIRS . INCLUDE_DIRS . REQUIRES ${requires})

component_compile_options(-ffast-math -O3)

