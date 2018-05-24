#include <jni.h>
#include <string>
#include "utils.h"

using namespace std;
extern "C"

JNIEXPORT jstring JNICALL
Java_com_qtfreet00_getsignnative_MainActivity_getSign(JNIEnv *env, jobject instance) {

    char *path = utils::getAppPathMaps();
    if (path) {
        string sign = utils::getSign(path);
        free(path);
        return env->NewStringUTF(sign.c_str());
    } else {
        string pathS = utils::getAppPathOrigin(env);
        if (!pathS.empty()) {
            const char *path_ori = pathS.c_str();
            string sign = utils::getSign((char *) path_ori);
            return env->NewStringUTF(sign.c_str());
        }
    }
    return env->NewStringUTF("NULL");
}