set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_SYSROOT /Users/fanibhushan/home_assistant/sysroots/ubuntu-arm64)

set(CMAKE_C_COMPILER /opt/homebrew/opt/llvm/bin/clang)
set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm/bin/clang++)

set(CMAKE_C_FLAGS "--target=aarch64-linux-gnu -fuse-ld=/opt/homebrew/bin/ld.lld")
set(CMAKE_CXX_FLAGS "--target=aarch64-linux-gnu -fuse-ld=/opt/homebrew/bin/ld.lld")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
