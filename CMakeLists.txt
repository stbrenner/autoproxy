cmake_minimum_required(VERSION 2.8.1) 
project(autoproxy) 

add_definitions(-DUNICODE -D_UNICODE)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  add_definitions(/W4)
  add_definitions(/WX)
  add_executable(autoproxy-helper main.c logic.c logic-windows.c) 
  target_link_libraries(autoproxy-helper Winhttp)
  add_custom_command(TARGET autoproxy-helper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/autoproxy.cmd" $<TARGET_FILE_DIR:autoproxy-helper>)

elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
  add_executable(autoproxy-helper main.c logic.c logic-linux.c) 
  add_custom_command(TARGET autoproxy-helper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/autoproxy" $<TARGET_FILE_DIR:autoproxy-helper>)
endif()
