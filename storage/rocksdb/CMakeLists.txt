# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 定义项目名称
project(test_rocksdb_ops)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 定义编译模式（默认为 Debug）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# 包含路径
set(INCLUDE_PATH /usr/local/rocksdb/include)

# 库路径
set(LIB_PATH /usr/local/rocksdb/lib64)

# 根据编译模式设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
else()
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

# 添加可执行文件
add_executable(${PROJECT_NAME} test_rocksdb_ops.cpp)

# 包含头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_PATH})

# 链接静态库：显式指定 .a 文件
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIB_PATH}/librocksdb.a
    pthread
)

# 如果是 Release 模式，添加调试信息分离的后处理步骤
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # 提取调试信息到单独的文件
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND objcopy --only-keep-debug $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE:${PROJECT_NAME}>.debug
        COMMENT "Extracting debug information to $<TARGET_FILE:${PROJECT_NAME}>.debug"
    )

    # 移除主文件中的调试信息
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND objcopy --strip-debug $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Stripping debug information from $<TARGET_FILE:${PROJECT_NAME}>"
    )

    # 添加调试链接到主文件
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND objcopy --add-gnu-debuglink=$<TARGET_FILE:${PROJECT_NAME}>.debug $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Adding debug link to $<TARGET_FILE:${PROJECT_NAME}>"
    )
endif()

# 清理规则
set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${PROJECT_NAME}.debug")
