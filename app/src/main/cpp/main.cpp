//
// Created by WindySha on 2021/7/16.
//

#include "main.h"
#include "bypass_dlfcn.h"
#include <jni.h>

extern "C"
JNIEXPORT void JNICALL
Java_com_wind_example_bypass_MainActivity_testBypassDlfcn(JNIEnv *env, jclass clazz) {
    void *handle = bp_dlopen("libart.so", RTLD_NOW);
    void *func_addres = bp_dlsym(handle,
                                 "_ZN3art6mirror5Class15FindClassMethodENSt3__117basic_string_viewIcNS2_11char_traitsIcEEEES6_NS_11PointerSizeE");
    LOGD(" art handle -> %p   func_addres -> %p", handle, func_addres);
}