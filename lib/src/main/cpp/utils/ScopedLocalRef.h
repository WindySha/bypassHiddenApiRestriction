
#pragma once

#include "jni.h"
#include <stddef.h>

template<typename T>
class ScopedLocalRef {
public:
    ScopedLocalRef(JNIEnv *env, T localRef) : mEnv(env), mLocalRef(localRef) {
    }

    ~ScopedLocalRef() {
        reset();
    }

    void reset(T ptr = NULL) {
        if (ptr != mLocalRef) {
            if (mLocalRef != NULL) {
                mEnv->DeleteLocalRef(mLocalRef);
            }
            mLocalRef = ptr;
        }
    }

    T release() __attribute__((warn_unused_result)) {
        T localRef = mLocalRef;
        mLocalRef = NULL;
        return localRef;
    }

    T get() const {
        return mLocalRef;
    }

private:
    JNIEnv *mEnv;
    T mLocalRef;

    // Disallow copy and assignment.
    ScopedLocalRef(const ScopedLocalRef &);
    void operator=(const ScopedLocalRef &);
};
