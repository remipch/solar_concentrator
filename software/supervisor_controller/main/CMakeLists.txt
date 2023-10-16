set(requires
                supervisor
                camera
                image
                target_detector
                motors
                web_custom)

idf_component_register(SRCS "app_main.cpp" REQUIRES ${requires} )

message(STATUS "CONFIG_CAMERA_MODULE_AI_THINKER = ${CONFIG_CAMERA_MODULE_AI_THINKER}")
message(STATUS "CONFIG_CAMERA_MODULE_ESP32_CAM_BOARD = ${CONFIG_CAMERA_MODULE_ESP32_CAM_BOARD}")
