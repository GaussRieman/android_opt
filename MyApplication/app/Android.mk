LOCAL_PATH := $(call my-dir)

SRC_DIR := $(LOCAL_PATH)/src/main/cpp
EIGEN_DIR := $(LOCAL_PATH)/src/main/include/eigen-3.3.4
QSPOWER_DIR := $(LOCAL_PATH)/src/main/include/qspower
INCLUDE_DIR := $(LOCAL_PATH)/src/main/include


#------------------powersdk---------------------#
include $(CLEAR_VARS)
LOCAL_MODULE := power
LOCAL_SRC_FILES := $(LOCAL_PATH)/libs/$(TARGET_ARCH_ABI)/libqspower-1.1.0.so
#$(warning  " ---LOCAL_SRC_FILES: $(LOCAL_SRC_FILES) ")
include $(PREBUILT_SHARED_LIBRARY)



#------------------native lib---------------------#
include $(CLEAR_VARS)
LOCAL_MODULE := native-lib
LOCAL_SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
LOCAL_C_INCLUDES := $(EIGEN_DIR) $(QSPOWER_DIR) $(INCLUDE_DIR)
LOCAL_SHARED_LIBRARIES := power
LOCAL_LDFLAGS := -fopenmp
LOCAL_LDLIBS := -llog
LOCAL_ARM_MODE := arm
LOCAL_ARM_NEON := true
LOCAL_CPPFLAGS += -ffunction-sections -fdata-sections
LOCAL_CFLAGS += -ffunction-sections -fdata-sections
LOCAL_LDFLAGS += -Wl,--gc-sections
LOCAL_CPPFLAGS := -std=c++11 -O3  -fopenmp  -DUSE_POWERSDK  -DDEBUG_CPU_TUNE #-funroll-loops  -ffast-math

include $(BUILD_SHARED_LIBRARY)
