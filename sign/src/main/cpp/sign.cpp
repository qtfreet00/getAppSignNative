#include <jni.h>
#include <string>
#include "utils.h"

using namespace std;

static const char *JNI_CLASS = "com/msarlab/library/sign/SignHelper";

static bool checkException(JNIEnv *env) {
    if (env->ExceptionOccurred()) {
        env->ExceptionClear();
        return true;
    }
    return false;
}

extern "C" {


jstring getSignToCharString(JNIEnv *env, jclass) {
    char *path = utils::getAppPath();
    if (path) {
        string sign = utils::getSignToCharString(path);
        free(path);
        return env->NewStringUTF(sign.c_str());
    }
    return NULL;
}

jbyteArray getSignToByteArray(JNIEnv *env, jclass) {
    char *path = utils::getAppPath();
    if (path) {
        int size = 0;
        signed char *sign = utils::getSignToByteArray(path, &size);
        free(path);
        if (size == 0) {
            return NULL;
        }
        jbyteArray array = env->NewByteArray(size);
        env->SetByteArrayRegion(array, 0, size, reinterpret_cast<const jbyte *>(sign));
        return array;
    }
    return NULL;
}


jint getSignHashCode(JNIEnv *env, jclass) {
    char *path = utils::getAppPath();
    if (path) {
        int sign = utils::getSignHashCode(path);
        free(path);
        return sign;
    }
    return 0;
}


jstring getAppPath(JNIEnv *env, jclass) {
    char *path = utils::getAppPath();
    if (path) {
        string appPath(path);
        free(path);
        return env->NewStringUTF(appPath.c_str());
    }
    return NULL;
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
    if (checkException(env) || clazz == NULL) {
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