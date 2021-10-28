# version 3.11 or later of CMake or needed later for installing GoogleTest
# so let's require it now.
cmake_minimum_required(VERSION 3.11-3.18)

project(LLHttp)

file(GLOB LLHttp_SRC
"*.cpp"
"*.c"
)

# add_compile_options(-fpermissive -Wno-write-strings)
add_definitions(-D_BUR_FORMAT_BRELF)
add_definitions(-D_REPLACE_CONST)

add_library(${PROJECT_NAME} ${LLHttp_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/../includes/)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/../includes/loupe/Includes/)


#if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MYPROJECT_BUILD_TESTING))
    #enable_testing()
    #add_subdirectory(test)
    
    add_executable(LLHttp_test ${LLHttp_SRC})

    target_include_directories(LLHttp_test PUBLIC ${PROJECT_SOURCE_DIR}/../includes/)
    target_include_directories(LLHttp_test PUBLIC ${PROJECT_SOURCE_DIR}/../includes/loupe/Includes/)


    set_target_properties( LLHttp_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/ )
    #add_test(NAME testAll COMMAND test)
#endif()

target_link_libraries(LLHttp PUBLIC TCPComm)
target_link_libraries(LLHttp PUBLIC StringExt)
target_link_libraries(LLHttp PUBLIC RingBufLib)

target_link_libraries(LLHttp_test PUBLIC TCPComm)
target_link_libraries(LLHttp_test PUBLIC StringExt)
target_link_libraries(LLHttp_test PUBLIC RingBufLib)