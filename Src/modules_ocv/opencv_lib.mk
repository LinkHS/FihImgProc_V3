LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/include
#LOCAL_C_INCLUDES += $(TOP)/bionic

#For using OpenCV2/3
#LOCAL_SHARED_LIBRARIES += libopencv_calib3d
LOCAL_SHARED_LIBRARIES += libopencv_core      
#LOCAL_SHARED_LIBRARIES += libopencv_features2d
#LOCAL_SHARED_LIBRARIES += libopencv_flann     
#LOCAL_SHARED_LIBRARIES += libopencv_highgui   
LOCAL_SHARED_LIBRARIES += libopencv_imgproc   
#LOCAL_SHARED_LIBRARIES += libopencv_ml        
#LOCAL_SHARED_LIBRARIES += libopencv_objdetect 
#LOCAL_SHARED_LIBRARIES += libopencv_photo     
#LOCAL_SHARED_LIBRARIES += libopencv_stitching 
#LOCAL_SHARED_LIBRARIES += libopencv_superres  
#LOCAL_SHARED_LIBRARIES += libopencv_video     
#LOCAL_SHARED_LIBRARIES += libopencv_videostab 

#For using OpenCV2
#LOCAL_SHARED_LIBRARIES += libopencv_contrib  
#LOCAL_SHARED_LIBRARIES += libopencv_gpu  
#LOCAL_SHARED_LIBRARIES += libopencv_legacy    
#LOCAL_SHARED_LIBRARIES += libopencv_nonfree   
#LOCAL_SHARED_LIBRARIES += libopencv_ocl 

#For using OpenCV3
LOCAL_SHARED_LIBRARIES += libopencv_imgcodecs
#LOCAL_SHARED_LIBRARIES += libopencv_shape
#LOCAL_SHARED_LIBRARIES += libopencv_videoio

#LOCAL_SHARED_LIBRARIES += libutils libcutils
#LOCAL_SHARED_LIBRARIES += libstdc++ libc++ 