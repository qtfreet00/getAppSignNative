#include <jni.h>
#include <string>
#include "signHelper.h"

using namespace std;

static const char *JNI_CLASS = "com/msarlab/library/security/Signature";

namespace JavaExp {

    bool checkException(JNIEnv *env) {
        if (env->ExceptionOccurred()) {
            env->ExceptionClear();
            return true;
        }
        return false;
    }
}

extern "C" {


jstring getSignToCharString(JNIEnv *env, jclass) {
    char *path = signHelper::getAppPath();
    if (!path) {
        return NULL;
    }
    char *sign = signHelper::getSignToCharString(path);
    free(path);
    if (!sign) {
        return NULL;
    }
    string toString(sign);
    free(sign);
    return env->NewStringUTF(toString.c_str());
}

jbyteArray getSignToByteArray(JNIEnv *env, jclass) {
    char *path = signHelper::getAppPath();
    if (!path) {
        return NULL;
    }
    int size = 0;
    signed char *sign = signHelper::getSignToByteArray(path, &size);
    free(path);
    if (size == 0) {
        return NULL;
    }
    jbyteArray array = env->NewByteArray(size);
    env->SetByteArrayRegion(array, 0, size, sign);
    return array;
}


jint getSignHashCode(JNIEnv *env, jclass) {
    char *path = signHelper::getAppPath();
    if (!path) {
        return 0;
    }
    int sign = signHelper::getSignHashCode(path);
    free(path);
    return sign;
}


jstring getAppPath(JNIEnv *env, jclass) {
    char *path = signHelper::getAppPath();
    if (!path) {
        return NULL;
    }
    string appPath(path);
    free(path);
    return env->NewStringUTF(appPath.c_str());
}

static JNINativeMethod gMethods[] = {
        {"k0", "()Ljava/lang/String;", (void *) getSignToCharString},
        {"k1", "()[B",                 (void *) getSignToByteArray},
        {"k2", "()I",                  (void *) getSignHashCode},
        {"k3", "()Ljava/lang/String;", (void *) getAppPath},
};

static int registerNativeMethods(JNIEnv *env, const char *className,
                                 JNINativeMethod *gMethods, int numMethods) {
    jclass clazz;
    clazz = env->FindClass(className);
    if (JavaExp::checkException(env) || clazz == NULL) {
        return JNI_FALSE;
    }

    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}


JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return JNI_ERR;
    }
    if (registerNativeMethods(env, JNI_CLASS,
                              gMethods,
                              sizeof(gMethods) / sizeof(gMethods[0])) == JNI_FALSE) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_4;
}
}