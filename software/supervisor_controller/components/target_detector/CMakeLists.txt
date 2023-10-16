
idf_component_register( SRC_DIRS . capstone_detector
                        INCLUDE_DIRS include
                        PRIV_INCLUDE_DIRS capstone_detector
                        REQUIRES
                            image
                            camera # use camera size at compile time
                        )

component_compile_options(-ffast-math -O3)

