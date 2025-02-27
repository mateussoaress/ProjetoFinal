# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)

# Nome do projeto atualizado
project(Placar_Digital C CXX ASM)

pico_sdk_init()

# Definição do executável com os arquivos corretos
add_executable(Placar_Digital 
    contador_votos.c   # Arquivo principal
    inc/ssd1306.c      # Biblioteca do display OLED
)

# Definição de propriedades do programa
pico_set_program_name(Placar_Digital "Placar_Digital")
pico_set_program_version(Placar_Digital "0.1")
pico_enable_stdio_uart(Placar_Digital 0)
pico_enable_stdio_usb(Placar_Digital 1)

# Vinculação com as bibliotecas do Raspberry Pi Pico
target_link_libraries(Placar_Digital pico_stdlib hardware_adc hardware_pwm hardware_i2c hardware_pio hardware_uart)

# Inclusão dos diretórios necessários
target_include_directories(Placar_Digital PRIVATE ${CMAKE_CURRENT_LIST_DIR} inc)

# Geração de binários adicionais
pico_add_extra_outputs(Placar_Digital)
