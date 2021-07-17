//  MIT License
//  Copyright (c) 2021 Windysha
//  https://github.com/WindySha/bypass_dlfunctions
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//      of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//      to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//      copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//      AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include <jni.h>
#include <sys/system_properties.h>
#include <string.h>
#include <stdio.h>
#include "dlfcn_trampoline.h"
#include "bypass_dlfcn.h"
#include "efl_parser.h"

#define ANDROID_KITKAT                  19
#define ANDROID_KITKAT_WATCH            20
#define ANDROID_LOLLIPOP                21
#define ANDROID_LOLLIPOP_MR1            22
#define ANDROID_M                       23
#define ANDROID_N                       24
#define ANDROID_N_MR1                   25
#define ANDROID_O                       26
#define ANDROID_O_MR1                   27
#define ANDROID_P                       28
#define ANDROID_Q                       29
#define ANDROID_R                       30

#ifdef __LP64__
static const char *const kSystemLibDir = "/system/lib64/";
static const char *const kApexLibDir = "/apex/com.android.runtime/lib64/";
static const char *const kApexArtNsLibDir = "/apex/com.android.art/lib64/";
#else
static const char *const kSystemLibDir = "/system/lib/";
static const char *const kApexLibDir = "/apex/com.android.runtime/lib/";
static const char *const kApexArtNsLibDir = "/apex/com.android.art/lib/";
#endif

static int api_level = -1;
static inline int32_t get_android_api_level() {
    if (api_level < 0) {
        char api[PROP_VALUE_MAX];
        __system_property_get("ro.build.version.sdk", api);
        api_level = atoi(api);
    }
    return api_level;
}

static void *trampoline_address_from_system_lib = NULL;
static void *get_trampoline_address() {
    if (trampoline_address_from_system_lib == NULL) {
        // choose an address in the libnativebridge.so, beacasue libnativebridge.so is very small.
        const char *nativeBridge_name = "libnativebridge.so";
        const char *so_prefix;
        if (get_android_api_level() >= ANDROID_R) {
            so_prefix = kApexArtNsLibDir;
        } else if (get_android_api_level() == ANDROID_Q) {
            so_prefix = kApexLibDir;
        } else {
            so_prefix = kSystemLibDir;
        }
        char target_so_path[512];
        memset(target_so_path, 0, sizeof(target_so_path));
        sprintf(target_so_path, "%s%s", so_prefix, nativeBridge_name);

#ifdef __LP64__
        const ASM_Instruction instruction[2] = {
            0xA8C17BFD,    //  ldp	x29, x30, [sp],#16
            0xD65F03C0     //  ret
        };
        trampoline_address_from_system_lib =
            getInstructionAddressFromElf(target_so_path, &instruction, 2);
#else
        const ASM_Instruction instruction = 0xBD10;   // pop {r4, pc}
        trampoline_address_from_system_lib =
            getInstructionAddressFromElf(target_so_path, &instruction, 1) + 1; // This is thumb instruction, need +1 here !!
#endif
        LOGI("get trampoline_address_from_system_lib result : %p ", trampoline_address_from_system_lib);
    }
    return trampoline_address_from_system_lib;
}

void *JNIEXPORT bp_dlopen(const char *filename, int flag) {
    if (get_android_api_level() < ANDROID_N) {
        return dlopen(filename, flag);
    }
    return dlfcn_trampoline(filename, (void *) flag, get_trampoline_address(), dlopen);
}

int JNIEXPORT bp_dlclose(void *handle) {
    return dlclose(handle);
}

const char *JNIEXPORT bp_dlerror() {
    return dlerror();
}

void *JNIEXPORT bp_dlsym(void *handle, const char *symbol) {
    return dlsym(handle, symbol);
//    if (get_android_api_level() < ANDROID_N) {
//        return dlsym(handle, symbol);
//    }
//    return dlfcn_trampoline(handle, symbol, get_trampoline_address(), dlsym);
}

int JNIEXPORT bp_dladdr(const void *ddr, Dl_info *info) {
    return dladdr(ddr, info);
}