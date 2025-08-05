cmake_minimum_required(VERSION 3.22)
project(q4nx LANGUAGES CXX VERSION 0.1.0)

# Set build type to Release
set(CMAKE_BUILD_TYPE Release)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/)


# Force output directories to be absolute
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# ———————————————————————————————————————————————
# User-tweakable XRT settings (override on cmake command-line)
# ———————————————————————————————————————————————
set(XRT_INCLUDE_DIR C:/Technical/XRT/src/runtime_src/core/include CACHE PATH   "Where XRT headers live")
set(XRT_LIB_DIR     C:/Technical/xrtNPUfromDLL     CACHE PATH   "Where XRT libs live")



# ———————————————————————————————————————————————
# Gather your sources
# ———————————————————————————————————————————————
file(GLOB SOURCES "src/*.cpp" "runner/*.cpp" "common/*.cpp" "common/*/*.cpp" "server/*.cpp" "pull/*.cpp")
file(GLOB HEADERS "include/*.hpp" "runner/*.hpp" "common/*.hpp" "common/*/*.hpp" "server/*.hpp" "pull/*.hpp")


add_executable(flm ${SOURCES} ${HEADERS})

# ———————————————————————————————————————————————
# Library specific settings
# ———————————————————————————————————————————————
target_include_directories(flm PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/runner
    ${CMAKE_SOURCE_DIR}/server
    ${CMAKE_SOURCE_DIR}/pull
    ${XRT_INCLUDE_DIR}
    C:/Program\ Files/boost/boost_1_88_0
    C:/dev/vcpkg/installed/x64-windows/include/
)

target_compile_definitions(flm PUBLIC
    DISABLE_ABI_CHECK=1
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    # Additional definitions for static linking
    CURL_STATICLIB
    BOOST_ALL_NO_LIB
    BOOST_ALL_STATIC_LINK
    # Handle legacy stdio functions
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_DEPRECATE
)

target_link_directories(flm PUBLIC
    C:/Program\ Files/boost/boost_1_88_0/stage/lib
    ${XRT_LIB_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    C:/dev/vcpkg/installed/x64-windows/lib
)

# Link static libraries first
if(MSVC)
    target_link_libraries(flm PUBLIC ${STATIC_LIBS})
endif()

# Link your custom libraries (these may still be DLLs if no static versions available)
target_link_libraries(flm PUBLIC
    xrt_coreutil
    q4_npu_eXpress
    llama_npu
    qwen_npu
    gemma_npu
    dequant
    gemm
    lm_head
    npu_utils
    libcurl
)

target_link_libraries(flm PRIVATE
    tokenizers_cpp
    tokenizers_c
    sentencepiece
    ntdll wsock32 ws2_32 Bcrypt
    iphlpapi userenv psapi
)


# ———————————————————————————————————————————————
# Copy the build flm.exe into the lib directory
# ———————————————————————————————————————————————
add_custom_command(TARGET flm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:flm>
        ${CMAKE_SOURCE_DIR}/out/flm.exe
)

# Add a custom target to check for remaining DLL dependencies
if(WIN32)
    add_custom_target(check_dependencies ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Checking for DLL dependencies..."
        COMMAND dumpbin /dependents $<TARGET_FILE:flm> | findstr /i ".dll"
        COMMENT "Checking for remaining DLL dependencies"
        DEPENDS flm
    )
endif()