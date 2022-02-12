//
// Created by WindySha on 2021/7/7.
//

#pragma

#include <jni.h>
#include "common.h"

#define CHECK_NOT_NULL_RETURN(__x)  \
    if (__x) {           \
        return (__x);               \
    }

struct FakeJNIEnv {
    void *vtb_function;
    void *const self_;  // Link to Thread::Current().    Thread* const self_;
    void *const vm_;  // The invocation interface JavaVM.    JavaVMExt* const vm_;
};

template<typename ...Args>
static bool JNIExceptionClearAndDescribe(JNIEnv *env, const char *format, Args... args) {
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        LOGE(format, args...);
        return false;
    }
    return true;
}

namespace bypass {
class StringPiece {
public:
    StringPiece(const char *str)
        : ptr_(str), length_((str == nullptr) ? 0 : strlen(str)) {}
private:
    const char *ptr_;
    // Length of data.
    size_t length_;
};
}