# CMakeList.txt: wbssh 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("wbssh")

message("$ENV{VCPKG_ROOT}")
message("system: ${CMAKE_SYSTEM_NAME}")
message("cmake current_source: ${CMAKE_CURRENT_SOURCE_DIR}")

set(vcpkg "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
if(NOT CMAKE_TOOLCHAIN_FILE AND EXISTS "${vcpkg}")
  set(CMAKE_TOOLCHAIN_FILE "${vcpkg}" CACHE FILEPATH "CMake toolchain file")
  message(STATUS "vcpkg toolchain found: ${CMAKE_TOOLCHAIN_FILE}")
endif()

message("cmake_toolchain_file: ${CMAKE_TOOLCHAIN_FILE}")

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	link_directories("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/lib")
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/include")
	set(ZLIB_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/vcpkg/pkgs/zlib_x64-windows")
	set(ZLIB_LIBRARY "${CMAKE_CURRENT_SROUCE_DIR}/vcpkg_installed/x64-windows/lib")
	set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/include")
	set(OPENSSL_ROOT_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/vcpkg/pkgs/openssl_x64-windows")
	set(OPENSSL_CRYPTO_LIBRARY  "${CMAKE_CURRENT_SROUCE_DIR}/vcpkg_installed/x64-windows/lib")
	set(OPENSSL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x64-windows/include" )
endif()

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)


add_subdirectory(libssh2)
include_directories(libssh2/include)

add_subdirectory(src/shell)

# 将源代码添加到此项目的可执行文件。
include_directories(include)
add_executable (wbssh "wbssh.cpp" "wbssh.h" "src/wb/wb.cpp" "src/poll.cpp" "include/shell/shell.h" "src/shell/shell_interactive.c" "src/shell/shell_no_interactive.c" "src/shell/shell.c")
target_link_libraries(wbssh PRIVATE zlib libssh2::libssh2_static)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET wbssh PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
