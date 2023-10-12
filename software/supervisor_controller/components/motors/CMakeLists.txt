
idf_component_register( SRC_DIRS .
                        INCLUDE_DIRS include
                        REQUIRES
                            esp_event
                            esp_timer
                            driver # (for UART)
                        )

