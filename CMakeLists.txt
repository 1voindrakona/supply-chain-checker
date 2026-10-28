cmake_minimum_required(VERSION 3.16)
project(SupplyChainChecker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(supply-chain-checker 
    src/main.cpp 
    src/parser.cpp 
    src/security.cpp
)

add_custom_command(TARGET supply-chain-checker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/config $<TARGET_FILE_DIR:supply-chain-checker>/config
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/policy.yml $<TARGET_FILE_DIR:supply-chain-checker>/policy.yml
)
