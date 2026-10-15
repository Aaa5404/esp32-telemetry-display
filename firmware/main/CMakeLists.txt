#idf_component_register(SRCS "main.c" "graphics.cpp" "display.c"
                    #PRIV_REQUIRES esp_wifi nvs_flash esp_driver_spi esp_driver_gpio
                    #INCLUDE_DIRS "." "components")
idf_component_register(
    SRCS
        "main.c"
        "display.c"
        "components/graphics/graphics.cpp"
        "components/graph/graph_buffer.cpp"
    PRIV_REQUIRES esp_wifi nvs_flash esp_driver_spi esp_driver_gpio esp_timer
    INCLUDE_DIRS
        "."
        "components/graphics"
        "components/graph"
)