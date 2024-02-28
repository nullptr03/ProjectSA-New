LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := projectsa
LOCAL_LDLIBS := -llog

# psa
FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/game_sa/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/game_sa/Core/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/util/*.cpp)

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/game_sa/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/game_sa/Core/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/util/

LOCAL_CPPFLAGS := -w -s -pthread -Wall -fpack-struct=1 -O2 -std=c++14 -fexceptions
#LOCAL_CPPFLAGS := -w -s -fvisibility=hidden -pthread -Wall -fpack-struct=1 -O2 -std=c++14 -fexceptions
include $(BUILD_SHARED_LIBRARY)