set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
configure_file(version.hh.in version.hh)

add_executable(lavue compiler.cc)

target_include_directories(lavue PUBLIC 
                           "${PROJECT_BINARY_DIR}/src")

install(TARGETS lavue DESTINATION bin)
