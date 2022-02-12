//
// Created by Windysha on 2022/2/12.
//

#include <link.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/system_properties.h>
#include <android/api-level.h>
#include <stdio.h>
#include "bypass_dlopen.h"
#include "../utils/common.h"

typedef void *(*loader_dlopen_func)(const char *filename, int flag, void *address);

extern __attribute__((weak))
int dl_iterate_phdr(int (*__callback)(struct dl_phdr_info *, size_t, void *), void *__data);

typedef struct {
    const char *elf_path;
    void *address;
} dl_phdr_data;

static inline int32_t get_android_api_level() {
    static int api_level = -1;
    if (api_level < 0) {
        api_level = android_get_device_api_level();
    }
    return api_level;
}

static int dl_iterate_callback(struct dl_phdr_info *info, size_t size, void *data) {
    dl_phdr_data *target = (dl_phdr_data *) (data);
    if (info->dlpi_addr != 0 && strstr(info->dlpi_name, target->elf_path)) {
        target->address = (void *) info->dlpi_addr;
        return 1;
    }
    return 0;
}

static void *get_lib_art_base_address() {
    static void *art_base_address = NULL;
    if (art_base_address != NULL) {
        return art_base_address;
    }
    if (dl_iterate_phdr == NULL) {
        return NULL;
    }
    const char *art_name = "libart.so";
    const char *so_prefix;
    int level = get_android_api_level();
    if (level >= ANDROID_R) {
        so_prefix = kApexArtNsLibDir;
    } else if (level == ANDROID_Q) {
        so_prefix = kApexLibDir;
    } else {
        so_prefix = kSystemLibDir;
    }
    char target_so_path[256];
    memset(target_so_path, 0, sizeof(target_so_path));
    sprintf(target_so_path, "%s%s", so_prefix, art_name);

    dl_phdr_data data;
    data.elf_path = target_so_path;
    data.address = NULL;
    dl_iterate_phdr(dl_iterate_callback, &data);
    art_base_address = data.address;

    return art_base_address;
}

__attribute__ ((visibility ("default")))
void *bypass_dlopen(const char *filename, int flag) {
    if (get_android_api_level() < ANDROID_N) {
        return dlopen(filename, flag);
    }
    void *handle = dlopen("libdl.so", RTLD_NOW);
    loader_dlopen_func loader_dlopen = (loader_dlopen_func) (dlsym(handle, "__loader_dlopen"));

    void *result_handle = loader_dlopen(filename, flag, (void *) dlerror);
    if (result_handle != NULL) return result_handle;

    void *called_adddres = get_lib_art_base_address();
    LOGI("dlopen called_adddres ->  [%p]", called_adddres);
    if (called_adddres == NULL) {
        return NULL;
    }

    return loader_dlopen(filename, flag, called_adddres);
}