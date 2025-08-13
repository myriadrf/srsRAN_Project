
set(SOURCES_LIMESUITENG
        radio_config_limesuiteng_validator.cpp
        radio_limesuiteng_impl.cpp
        radio_limesuiteng_rx_stream.cpp
        radio_limesuiteng_tx_stream.cpp)

add_library(srsran_radio_limesuiteng STATIC ${SOURCES_LIMESUITENG})

target_link_libraries(srsran_radio_limesuiteng limesuiteng::limesuiteng)

install(TARGETS srsran_radio_limesuiteng EXPORT srsran_export)