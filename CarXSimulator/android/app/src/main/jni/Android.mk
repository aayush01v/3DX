# Android.mk for CarX Simulator Native Library
# For use with AIDE+ Pro NDK build

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := carx-native
LOCAL_SRC_FILES := main.cpp

# C++ flags
LOCAL_CPPFLAGS  := -std=c++17 -O2 -fexceptions

# Link against Android log library
LOCAL_LDLIBS    := -llog -landroid

# Build as shared library
include $(BUILD_SHARED_LIBRARY)
