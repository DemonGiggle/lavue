set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
configure_file(version.hh.in version.hh)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")
message(STATUS "LLVM library: ${llvm_libs}")
llvm_map_components_to_libnames(llvm_libs 
                                core passes instrumentation objcarcopts codegen)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(lavue 
               compiler.cc
               ast.cc
               utils.cc)

target_include_directories(lavue PUBLIC 
                           "${PROJECT_BINARY_DIR}/src")

install(TARGETS lavue DESTINATION bin)

target_link_libraries(lavue ${llvm_libs})
