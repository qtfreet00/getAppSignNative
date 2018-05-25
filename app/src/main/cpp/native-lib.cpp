#include <jni.h>
#include <string>
#include "utils.h"

using namespace std;
extern "C"

JNIEXPORT jstring JNICALL
Java_com_qtfreet00_getsignnative_MainActivity_getSign(JNIEnv *env, jobject instance) {

    char *path = utils::getAppPath();
    if (path) {
        string sign = utils::getSign(path);
        free(path);
        return env->NewStringUTF(sign.c_str());
    }
    return env->NewStringUTF("NULL");
}