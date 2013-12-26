cmake_minimum_required(VERSION 2.8.1) 
project(autoproxy) 
enable_testing()

add_definitions(-DUNICODE -D_UNICODE)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -MTd")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -MT")
  set (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -MT")
  set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -MT")

  add_definitions(/W4)
  add_definitions(/WX)
  add_executable(autoproxy-helper main.c logic.c logic-windows.c) 
  target_link_libraries(autoproxy-helper Winhttp)
  add_custom_command(TARGET autoproxy-helper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/autoproxy.cmd" $<TARGET_FILE_DIR:autoproxy-helper>)

  add_executable(test-windows test-windows.c logic.c logic-windows.c) 
  target_link_libraries(test-windows Winhttp)
  add_test(test-windows test-windows)

elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
  add_executable(autoproxy-helper main.c logic.c logic-linux.c) 
  add_custom_command(TARGET autoproxy-helper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/autoproxy" $<TARGET_FILE_DIR:autoproxy-helper>)
endif()
