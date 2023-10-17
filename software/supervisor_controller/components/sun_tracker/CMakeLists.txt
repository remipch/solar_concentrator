
idf_component_register( SRC_DIRS .
                        INCLUDE_DIRS include
                        REQUIRES
                            camera
                            target_detector
                            image
                            motors
                        )

component_compile_options(-ffast-math -O3)
