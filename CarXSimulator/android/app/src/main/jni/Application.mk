# Application.mk for CarX Simulator
# For use with AIDE+ Pro NDK build

# Target all common ABIs
APP_ABI := armeabi-v7a arm64-v8a x86 x86_64

# Use C++17
APP_STL := c++_shared
APP_CPPFLAGS := -std=c++17 -fexceptions -frtti

# Minimum Android API level
APP_PLATFORM := android-24

# Optimization
APP_OPTIM := release
