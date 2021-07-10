
//
// Created by WindySha(https://github.com/WindySha) on 2021/7/10.
//

#include <jni.h>
#include <string>
#include <android/api-level.h>
#include <mutex>
#include <dlfcn.h>
#include <bypass_dlfcn.h>
#include "utils/utils.h"
#include "utils/ScopedLocalRef.h"

static void *(*DecodeJObject)(void *, void *) = nullptr;
static void *(*FindClassMethod)(void *, std::string_view, std::string_view, size_t) = nullptr;
static void *(*FindClassMethod_P)(void *,
                                  const bypass::StringPiece &,
                                  const bypass::StringPiece &,
                                  size_t) = nullptr;  //Only For Android P
static void *(*FindDeclaredDirectMethodByName)(void *, std::string_view, size_t) = nullptr;
static void *(*FindDeclaredVirtualMethodByName)(void *, std::string_view, size_t) = nullptr;

// Mutex protecting static variables
static std::mutex g_has_inited_mutex;
static bool g_has_inited = false;

static const auto android_api_level = android_get_device_api_level();  /* NOLINT */

static void EnsureInitialized() {
    if (g_has_inited) {
        return;
    }
    std::lock_guard<std::mutex> guard(g_has_inited_mutex);
    if (g_has_inited) {
        return;
    }

    void *art_so_handle = bp_dlopen("libart.so", RTLD_NOW);

    const char *findClassMethod_name;
    if (android_api_level == ANDROID_P) {
        findClassMethod_name =
            "_ZN3art6mirror5Class15FindClassMethodERKNS_11StringPieceES4_NS_11PointerSizeE";
        void *address = bp_dlsym(art_so_handle, findClassMethod_name);
        FindClassMethod_P = reinterpret_cast<decltype(FindClassMethod_P)>(address);
    } else {
        findClassMethod_name =
            "_ZN3art6mirror5Class15FindClassMethodENSt3__117basic_string_viewIcNS2_11char_traitsIcEEEES6_NS_11PointerSizeE";
        void *address = bp_dlsym(art_so_handle, findClassMethod_name);
        FindClassMethod = reinterpret_cast<decltype(FindClassMethod)>(address);
    }

    const char *decodeJObject_name = "_ZNK3art6Thread13DecodeJObjectEP8_jobject";
    DecodeJObject = reinterpret_cast<decltype(DecodeJObject)>(bp_dlsym(
        art_so_handle,
        decodeJObject_name));

    g_has_inited = true;
}

static void *FindClassMethodCompat(void *mirrorClass,
                                   std::string name,
                                   std::string signature,
                                   size_t pointer_size) {
    void *art_method = nullptr;
    if (android_api_level == ANDROID_P) {
        if (FindClassMethod_P != nullptr) {
            art_method = FindClassMethod_P(mirrorClass,
                                           bypass::StringPiece(name.c_str()),
                                           bypass::StringPiece(signature.c_str()),
                                           pointer_size);
        }
    } else {
        if (FindClassMethod != nullptr) {
            art_method = FindClassMethod(mirrorClass,
                                         name,
                                         signature,
                                         pointer_size);
        }
    }
    CHECK_NOT_NULL_RETURN(art_method)

    // If FindClassMethod is blocked by Google in the future, try to use FindDeclaredDirectMethodByName or FindDeclaredVirtualMethodByName
    // So, you know, the following code will not be executed at current time.
    void *art_so_handle = bp_dlopen("libart.so", RTLD_NOW);
    if (FindDeclaredDirectMethodByName == nullptr) {
        const char *FindDeclaredDirectMethodByName_sig =
            "_ZN3art6mirror5Class30FindDeclaredDirectMethodByNameENSt3__117basic_string_viewIcNS2_11char_traitsIcEEEENS_11PointerSizeE";
        FindDeclaredDirectMethodByName =
            reinterpret_cast<decltype(FindDeclaredDirectMethodByName)>(bp_dlsym(art_so_handle,
                                                                                FindDeclaredDirectMethodByName_sig));
    }
    if (FindDeclaredDirectMethodByName != nullptr) {
        art_method = FindDeclaredDirectMethodByName(mirrorClass, name, pointer_size);
        CHECK_NOT_NULL_RETURN(art_method)
    }

    if (FindDeclaredVirtualMethodByName == nullptr) {
        const char *FindDeclaredVirtualMethodByName_sig =
            "_ZN3art6mirror5Class31FindDeclaredVirtualMethodByNameENSt3__117basic_string_viewIcNS2_11char_traitsIcEEEENS_11PointerSizeE";
        FindDeclaredVirtualMethodByName =
            reinterpret_cast<decltype(FindDeclaredVirtualMethodByName)>(bp_dlsym(art_so_handle,
                                                                                 FindDeclaredVirtualMethodByName_sig));
    }
    if (FindDeclaredVirtualMethodByName != nullptr) {
        art_method = FindDeclaredVirtualMethodByName(mirrorClass, name, pointer_size);
        CHECK_NOT_NULL_RETURN(art_method)
    }
    return art_method;
}

static void *GetCurrentThread(JNIEnv *env) {
    ScopedLocalRef<jclass> thread_class(env, env->FindClass("java/lang/Thread"));
    static jmethodID currentThread_id =
        env->GetStaticMethodID(thread_class.get(), "currentThread", "()Ljava/lang/Thread;");
    ScopedLocalRef<jobject>
        current_thread(env, env->CallStaticObjectMethod(thread_class.get(), currentThread_id));
    static jfieldID nativePeer_fid = env->GetFieldID(thread_class.get(), "nativePeer", "J");

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    } else {
        jlong thread_id = env->GetLongField(current_thread.get(), nativePeer_fid);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        } else {
            return reinterpret_cast<void *>(thread_id);
        }
    }

    //  Thread.nativePeer is in the graylist, if it cannot be accessed in the future, use the following method:
    auto *mirrorEnv = reinterpret_cast<FakeJNIEnv *>(env);
    return mirrorEnv->self_;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wind_hiddenapi_bypass_HiddenApiBypass_setHiddenApiExemptions(JNIEnv *env,
                                                                      jclass clazz,
                                                                      jobjectArray signature_prefixes) {
    if (android_api_level < ANDROID_P) {
        LOGI("Device do not need to bypass hidden api restriction.");
        return;
    }
    EnsureInitialized();

    const char *VMRuntime_class_name = "dalvik/system/VMRuntime";
    ScopedLocalRef<jclass> vmRumtime_class(env, env->FindClass(VMRuntime_class_name));
    if (!JNIExceptionClearAndDescribe(env, "Find VMRuntime class failed !!!", "")) {
        return;
    }

    auto current_thread = GetCurrentThread(env);
    void *VMRuntime_mirror_class_obj = DecodeJObject(current_thread, vmRumtime_class.get());
    if (VMRuntime_mirror_class_obj == nullptr) {
        LOGE("Decode VMRuntime jclass to mirror::class failed !!!");
        return;
    }

    size_t pointer_size = sizeof(void *);
    // Find the jmethodId of VMRuntime.getRuntime()
    void *getRuntime_art_method = FindClassMethodCompat(VMRuntime_mirror_class_obj,
                                                        "getRuntime",
                                                        "()Ldalvik/system/VMRuntime;",
                                                        pointer_size);
    if (getRuntime_art_method == nullptr) {
        LOGE("VMRuntime.getRuntime() method cannot be founded !!!");
        return;
    }
    ScopedLocalRef<jobject> VMRuntime_instance(env,
                                               env->CallStaticObjectMethod(vmRumtime_class.get(),
                                                                           reinterpret_cast<jmethodID>(getRuntime_art_method)));
    if (!JNIExceptionClearAndDescribe(env, "Call VMRuntime.getRuntime() method failed !!!", "")) {
        return;
    }

    // Find the jmethodId of VMRuntime.setHiddenApiExemptions()
    void *setHiddenApiExemptions_art_method = FindClassMethodCompat(VMRuntime_mirror_class_obj,
                                                                    "setHiddenApiExemptions",
                                                                    "([Ljava/lang/String;)V",
                                                                    pointer_size);
    if (setHiddenApiExemptions_art_method == nullptr) {
        LOGE("VMRuntime.setHiddenApiExemptions() method cannot be founded !!!");
        return;
    }

    env->CallVoidMethod(VMRuntime_instance.get(),
                        reinterpret_cast<jmethodID>(setHiddenApiExemptions_art_method),
                        signature_prefixes);
    if (!JNIExceptionClearAndDescribe(env,
                                      "Call VMRuntime.setHiddenApiExemptions() method failed !!!",
                                      "")) {
        return;
    }
    LOGI("Bypass Hidden Api Restriction Succeed.");
}