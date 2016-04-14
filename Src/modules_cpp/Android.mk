LOCAL_PATH := $(call my-dir)
FIHIMGPROC_PATH := $(TOP)/$(MTK_PATH_SOURCE)/external/fihimgproc_cpp

#-----------------------------------------------------------
include $(CLEAR_VARS)

include $(FIHIMGPROC_PATH)/config.mk
include $(LOCAL_PATH)/Modules_cpp.mk

ifeq (yes, $(strip $(FIH_IMGPROC_OPENMP)))
  $(info use openmp)
  LOCAL_CFLAGS += -fopenmp -pthread
  LOCAL_STATIC_LIBRARIES += libopenmp
endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += ${LOCAL_PATH}/
LOCAL_C_INCLUDES += ${FIHIMGPROC_PATH}/
LOCAL_C_INCLUDES += ${FIHIMGPROC_PATH}/includes

#-----------------------------------------------------------
LOCAL_SRC_FILES += others.cpp
LOCAL_SRC_FILES += CLAHE.cpp 
LOCAL_SRC_FILES += LMVFilter.cpp 

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog

#-----------------------------------------------------------
LOCAL_MODULE := libfihimgproc.modules_cpp
LOCAL_MULTILIB := both
ifeq (shared, $(strip $(MODULES_CPP_LIB)))
  include $(BUILD_SHARED_LIBRARY)
else
  include $(BUILD_STATIC_LIBRARY)
endif


ifeq (yes, no)
  include $(CLEAR_VARS)
  include $(call all-makefiles-under,$(LOCAL_PATH))
endif

