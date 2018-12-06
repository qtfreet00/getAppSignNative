//
// Created by qtfreet00 on 2018/5/24.
//
#include <libgen.h>
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

bool utils::isOreo() {
    int osVer = native_get_int("ro.build.version.sdk");
    if (osVer != 0) {
        if (osVer >= 25) {
            if (osVer >= 26) {
                return true;
            } else if (osVer == 25) {
                string version = native_get("ro.build.version.release");
                if (version.compare("O") == 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

char *utils::getAppPath() {
    char *name = getProcessName();
    if (!name) {
        return NULL;
    }
    size_t length = strlen(name);
    regex_t re;
    int status;
    if (isOreo()) {
        status = regcomp(&re,
                         "^/data/app/[a-zA-Z0-9]+(.[a-zA-Z0-9]+)+-([a-zA-Z0-9\\-_=]+)+(/base)?.apk$",
                         REG_EBRACE);
    } else {
        status = regcomp(&re, "^/data/app/[a-zA-Z0-9]+(.[a-zA-Z0-9]+)+-[0-9]+(/base)?.apk$",
                         REG_EBRACE);
    }
    if (status) {
        free(name);
        return NULL;
    }
    FILE *fp = fopen("/proc/self/maps", "r");
    if (!fp) {
        free(name);
        return NULL;
    }
    char buffer[256] = {0};
    char path[180] = {0};
    regmatch_t pmatch;
    bool find = false;
    while (fgets(buffer, 256, fp)) {
        if (sscanf(buffer, "%*llx-%*llx %*s %*s %*s %*s %s", path) != 1) { continue; }
        if (regexec(&re, path, 1, &pmatch, 0) != 0) { continue; }
        char *package = strstr(path, name);
        if (package == NULL) { continue; }
        char next = *(package + length);
        if (next == '-') {
            find = true;
            break;
        }
    }
    fclose(fp);
    regfree(&re);
    free(name);
    if (find) {
        char *appPath = (char *) malloc(128);
        memset(appPath, 0, 128);
        memcpy(appPath, path, strlen(path));
        return appPath;
    }
    return NULL;
}

string utils::getSignToCharString(char *appPath) {
    pkcs7 signature;
    if (signature.open_file(appPath)) {
        char *buff = signature.toCharString();
        if (buff) {
            string sign(buff);
            free(buff);
            return sign;
        }
    }
    return string();
}

signed char *utils::getSignToByteArray(char *appPath, int *size) {
    pkcs7 signature;
    if (signature.open_file(appPath)) {
        signed char *array = signature.toByteArray(size);
        return array;
    }
    return NULL;
}


int utils::getSignHashCode(char *appPath) {
    pkcs7 signature;
    if (signature.open_file(appPath)) {
        int hashcode = signature.hashCode();
        return hashcode;
    }
    return 0;
}
