//
// Created by qtfreet00 on 2018/5/24.
//

#ifndef GETSIGNNATIVE_UTILS_H
#define GETSIGNNATIVE_UTILS_H

#include <jni.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <sys/system_properties.h>
#include <string>
#include "pkcs7.h"

using namespace std;

class utils {
public:

    static char *getAppPath();

    static string getSign(char *appPath);

private:
    static char *getProcessName();

    static int isOreo();

    static int native_get_int(const char *name);

    static string native_get(const char *name);

};


#endif //GETSIGNNATIVE_UTILS_H
