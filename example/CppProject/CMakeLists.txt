# version 3.11 or later of CMake or needed later for installing GoogleTest
# so let's require it now.
cmake_minimum_required(VERSION 3.11-3.18)

# project(LLHttp_test)

file(GLOB LLHttp_SRC
"*.cpp"
"*.c"
)

# add_compile_options(-fpermissive -Wno-write-strings)
add_definitions(-D_BUR_FORMAT_BRELF)
add_definitions(-D_REPLACE_CONST)

set(CMAKE_BUILD_TYPE Debug)

add_executable(LLHttp_example ${LLHttp_SRC})

target_include_directories(LLHttp_example PUBLIC ${PROJECT_SOURCE_DIR}/../../includes/)
target_include_directories(LLHttp_example PUBLIC ${PROJECT_SOURCE_DIR}/../../includes/loupe/Includes/)
target_include_directories(LLHttp_example PUBLIC ${PROJECT_SOURCE_DIR}/../)
set_target_properties( LLHttp_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/ )

#add_test(NAME testAll COMMAND test)
#endif()

target_link_libraries(LLHttp_example PUBLIC LLHttp)