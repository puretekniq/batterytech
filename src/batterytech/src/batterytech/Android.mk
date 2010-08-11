# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -DANDROID_NDK \
                -DDISABLE_IMPORTGL

LOCAL_MODULE    := batterytech
LOCAL_SRC_FILES :=\
	batterytech.cpp \
	logger.cpp \
	decoders/stb_image.c \
	decoders/stb_vorbis.c \
	sound/PCMSound.cpp \
	sound/PCMStream.cpp \
	sound/SoundManager.cpp \
	platform/android/androidgeneral.cpp \
	platform/android/boot.cpp \
	platform/android/importgl.c

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_SHARED_LIBRARY)
