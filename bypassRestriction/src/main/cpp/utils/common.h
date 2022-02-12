//
// Created by Windysha on 2022/2/12.
//
#pragma
#include <android/log.h>

#define ANDROID_N                       24
#define ANDROID_P                       28
#define ANDROID_Q                       29
#define ANDROID_R                       30

#define LOG_TAG  "Bypass_Restriction"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,  LOG_TAG, __VA_ARGS__)

#ifdef __LP64__
const char *const kSystemLibDir = "/system/lib64/";
const char *const kApexLibDir = "/apex/com.android.runtime/lib64/";
const char *const kApexArtNsLibDir = "/apex/com.android.art/lib64/";
#else
const char *const kSystemLibDir = "/system/lib/";
const char *const kApexLibDir = "/apex/com.android.runtime/lib/";
const char *const kApexArtNsLibDir = "/apex/com.android.art/lib/";
#endif