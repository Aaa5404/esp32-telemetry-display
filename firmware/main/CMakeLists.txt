#idf_component_register(SRCS "graphics.cpp" "display.c" "station_example_main.c"
                    #PRIV_REQUIRES esp_wifi nvs_flash esp_driver_spi esp_driver_gpio
                    #INCLUDE_DIRS "." "components")
idf_component_register(
    SRCS
        "station_example_main.c"
        "display.c"
        "components/graphics/graphics.cpp"
        "components/graph/graph_buffer.cpp"
    PRIV_REQUIRES esp_wifi nvs_flash esp_driver_spi esp_driver_gpio
    INCLUDE_DIRS
        "."
        "components/graphics"
        "components/graph"
)