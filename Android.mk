LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL
SDL_TTF_PATH := ../SDL_ttf

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include $(LOCAL_PATH)/$(SDL_TTF_PATH)/include

LOCAL_SRC_FILES := srcs/main.c srcs/getarg.c srcs/card.c srcs/clock.c srcs/flipclock.c

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -lm

include $(BUILD_SHARED_LIBRARY)
