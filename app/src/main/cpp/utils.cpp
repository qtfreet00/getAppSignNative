//
// Created by qtfreet00 on 2018/5/24.
//


#include "utils.h"


char *utils::getProcessName() {
    char *buffer = (char *) malloc(128);
    memset(buffer, 0, 128);
    int fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd > 0) {
        ssize_t r = read(fd, buffer, 128);
        close(fd);
        if (r > 0) {
            return buffer;
        }
    }
    free(buffer);
    return NULL;
}

int utils::native_get_int(const char *name) {
    char value[PROP_VALUE_MAX];
    int len = 0;
    char *end;
    if (name != NULL) {
        len = __system_property_get(name, value);
    }
    if (len <= 0) {
        return 0;
    }
    int result = (int) strtol(value, &end, 0);
    if (end == value) {
        result = 0;
    }
    return result;
}

string utils::native_get(const char *name) {
    char value[PROP_VALUE_MAX];
    if (name != NULL) {
        __system_property_get(name, value);
    }
    return string(value);
}

int utils::isOreo() {
    int osVer = native_get_int("ro.build.version.sdk");
    if (osVer != 0) {
        if (osVer >= 25) {
            if (osVer >= 26) {
                return 1;
            } else if (osVer == 25) {
                string version = native_get("ro.build.version.release");
                if (version.compare("O") == 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

char *utils::getAppPathMaps() {
    char *processName = getProcessName();
    if (!processName) {
        return NULL;
    }
    regex_t re;
    int status;
    if (isOreo()) {
        status = regcomp(&re,
                         "^/data/app/[a-zA-Z0-9]+(.[a-zA-Z0-9]+)+-(.[a-zA-Z0-9-_=]+)+(/base)?.apk$",
                         REG_EBRACE);
    } else {
        status = regcomp(&re, "^/data/app/[a-zA-Z0-9]+(.[a-zA-Z0-9]+)+-[0-9]+(/base)?.apk$",
                         REG_EBRACE);
    }
    if (status) {
        free(processName);
        return NULL;
    }
    FILE *fp = fopen("/proc/self/maps", "r");
    if (!fp) {
        free(processName);
        return NULL;
    }
    char buffer[256] = {0};
    char path[180] = {0};
    regmatch_t pmatch;
    bool find = false;
    while (fgets(buffer, 256, fp)) {
        if (sscanf(buffer, "%*llx-%*llx %*s %*s %*s %*s %s", path) == 1) {
            if (regexec(&re, path, 1, &pmatch, 0) == 0 && strstr(path, processName)) {
                find = true;
                break;
            }
        }
    }
    fclose(fp);
    regfree(&re);
    free(processName);
    if (find) {
        char *appPath = (char *) malloc(128);
        memset(appPath, 0, 128);
        memcpy(appPath, path, strlen(path));
        return appPath;
    }
    return NULL;
}

jobject utils::getGlobalContext(JNIEnv *env) {
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread,
                                                             "currentActivityThread",
                                                             "()Landroid/app/ActivityThread;");
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(activityThread);
        return NULL;
    }
    jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(activityThread);
        return NULL;
    }
    jmethodID getApplication = env->GetMethodID(activityThread, "getApplication",
                                                "()Landroid/app/Application;");
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(activityThread);
        env->DeleteLocalRef(at);
        return NULL;
    }
    jobject context = env->CallObjectMethod(at, getApplication);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(activityThread);
        env->DeleteLocalRef(at);
        return NULL;
    }
    env->DeleteLocalRef(activityThread);
    env->DeleteLocalRef(at);
    return context;
}

string utils::getPackageResourcePath(JNIEnv *env, jobject context) {
    jclass clazz = env->GetObjectClass(context);
    jmethodID resPathMid = env->GetMethodID(clazz, "getPackageResourcePath",
                                            "()Ljava/lang/String;");
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        return "";
    }
    jstring resPath = (jstring) env->CallObjectMethod(context, resPathMid);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        return "";
    }
    const char *resPathStr = env->GetStringUTFChars(resPath, false);
    string appPath(resPathStr);
    env->DeleteLocalRef(clazz);
    env->ReleaseStringUTFChars(resPath, resPathStr);
    return appPath;
}

string utils::getAppPathOrigin(JNIEnv *env) {
    jobject context = getGlobalContext(env);
    if (!context) {
        return "";
    }
    string path_origin = getPackageResourcePath(env, context);
    env->DeleteLocalRef(context);
    return path_origin;
}

string utils::getSign(char *appPath) {
    pkcs7 pk;
    string sign;
    if (pk.open_file(appPath)) {
        char *buff = pk.getSign();
        if (buff) {
            sign.append(buff);
            free(buff);
            return sign;
        }
    }
    return "";
}

