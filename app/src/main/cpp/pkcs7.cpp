#include "pkcs7.h"

/**
 * 构造函数，必须提供签名证书文件或者apk文件
 */
pkcs7::pkcs7() {
    m_content = NULL;
    head = tail = NULL;
    p_cert = p_signer = NULL;
    m_pos = m_length = 0;
    apk_file = cert_file = NULL;

}

bool pkcs7::open_file(char *file_name) {
    bool ret = get_content(file_name);
    if (!ret) {
        printf("The file format is error!\n");
        return ret;
    }
    ret = parse_pkcs7();
    if (!ret) {
        printf("parse the pkcs7 format error!\n");
        return ret;
    }
    return true;
}

pkcs7::~pkcs7() {
    element *p = head;
    while (p != NULL) {
        head = p->next;
        free(p);
        p = head;
    }
    free(m_content);
    if (apk_file != NULL)
        free(apk_file);
    if (cert_file != NULL)
        free(cert_file);
}

/**
 * 该函数用于从apk中获取签名证书文件，  META-INF/.[RSA|DSA|EC]。
 * 若找到将该文件内容保存在m_content中，m_length为其长度
 *
 * 使用minizip库， 1）unzOpen64 打开apk文件；
 *                 2）unzGetGlobalInfo64 获取文件总数；
                   3）unzGoFirstFile 和 unzGoToNextFile 遍历文件；
				   4）unzGetCurrentFileInfo64 获取当前文件信息，对比找到签名证书文件；
				   5）unzOpenCurrentFilePass 打开当前文件；
				   6）unzReadCurrentFile 读取当前文件内容；
				   7）unzCloseCurrentFile 关闭当前文件；
				   8）unzClose 关闭apk文件。
 */
bool pkcs7::get_from_apk(char *file_name) {
    unzFile uf = NULL;
    unz_file_info64 file_info;
    char filename_inzip[256];
    int err;

    uf = unzOpen64(file_name);
    if (uf == NULL) {
        printf("open apk file error!\n");
        return false;
    }
    apk_file = (char *) malloc(sizeof(char) * (strlen(file_name) + 1));
    strcpy(apk_file, file_name);

    unz_global_info64 gi;
    err = unzGetGlobalInfo64(uf, &gi);
    if (err != UNZ_OK) {
        printf("error %d with zipfile in unzGetGlobalInfo \n", err);
        return false;
    }
    err = unzGoToFirstFile(uf);
    int i;
    for (i = 0; i < gi.number_entry; i++) {
        if (err != UNZ_OK) {
            printf("get file error!\n");
            return false;
        }
        if (unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0,
                                    NULL, 0)) {
            printf("get file infomation error!\n");
            return false;
        }
        size_t name_len = strlen(filename_inzip);
        if (name_len != file_info.size_filename) {
            printf("file name length is not right!\n");
            return false;
        }
        if (name_len > 13) {// "META-INF/*.RSA"
            if ((!strncmp(filename_inzip, "META-INF/", 9)) &&
                (!strcasecmp(filename_inzip + name_len - 4, ".RSA") ||
                 !strcasecmp(filename_inzip + name_len - 4, ".DSA") ||
                 !strcasecmp(filename_inzip + name_len - 3, ".EC"))) {
                cert_file = (char *) malloc(sizeof(char) * (name_len + 1));
                strcpy(cert_file, filename_inzip);
                break;
            }
        }
        err = unzGoToNextFile(uf);
    }
    if (i == gi.number_entry) {
        printf("cannot find the file!\n");
        return false;
    }

    err = unzOpenCurrentFilePassword(uf, NULL);
    if (err != UNZ_OK) {
        printf("open current error!\n");
        return false;
    }
    /*获取文件内容*/
    m_length = (int) file_info.uncompressed_size;
    if (m_length <= 0)
        return false;
    m_content = (unsigned char *) malloc((size_t) (m_length));
    err = unzReadCurrentFile(uf, m_content, (unsigned int) m_length);
    if (err != file_info.uncompressed_size) {
        printf("read content error!\n");
        return false;
    }
    unzCloseCurrentFile(uf);
    unzClose(uf);
    return true;
}

/**
 * 获取签名文件内容，支持：1）直接提供的是签名文件；2）apk压缩文件。
 */

bool pkcs7::get_content(char *file_name) {
    size_t name_len = strlen(file_name);
    if (name_len < 4)
        return false;
    if (!strcasecmp(file_name + name_len - 4, ".RSA") ||
        !strcasecmp(file_name + name_len - 4, ".DSA") ||
        !strcasecmp(file_name + name_len - 3, ".EC")) {
        FILE *f = fopen(file_name, "rb");
        if (f == NULL)
            return false;
        fseek(f, 0, SEEK_END);
        m_length = (int) ftell(f);
        if (m_length == -1) {
            fclose(f);
            return false;
        }
        fseek(f, 0, SEEK_SET);
        m_content = (unsigned char *) malloc(sizeof(unsigned char) * m_length);
        bool match = fread(m_content, 1, (size_t) m_length, f) == m_length;
        fclose(f);
        return match;
    }
    return get_from_apk(file_name);
}

/**
 * 根据lenbyte计算出 length所占的字节个数， 1）字节最高位为1，则低7位长度字节数；2）最高位为0，则lenbyte表示长度
 */
int pkcs7::len_num(unsigned char lenbyte) {
    int num = 1;
    if (lenbyte & 0x80) {
        num += lenbyte & 0x7f;
    }
    return num;
}

/**
 * 将长度信息转化成ASN.1长度格式
 * len <= 0x7f       1
 * len >= 0x80       1 + 非零字节数
 */
int pkcs7::num_from_len(int len) {
    int num = 0;
    int tmp = len;
    while (tmp) {
        num++;
        tmp >>= 8;
    }
    if ((num == 1 && len >= 0x80) || (num > 1))
        num += 1;
    return num;
}

/**
 *每个element元素都是{tag, length, data}三元组，tag和length分别由tag和len保存，data是由[begin, begin+len)保存。
 *
 *该函数是从data位置计算出到tag位置的偏移值
 */
int pkcs7::tag_offset(element *p) {
    if (p == NULL)
        return 0;
    int offset = num_from_len(p->len);
    if (m_content[p->begin - offset - 1] == p->tag)
        return offset + 1;
    else
        return 0;
}

/**
 * 根据lenbyte计算长度信息，算法是 lenbyte最高位为1， 则lenbyte & 0x7F表示length的字节长度，后续字节使用大端方式存放
 * 最高位为0， lenbyte直接表示长度
 *
 * 1)若 0x82 0x34 0x45 0x22 ....  0x82是lenbyte， 高位为1，0x82 & 0x7F == 2，则后续两个字节是高端存放的长度信息
    则长度信息为 0x3445
   2)若 lenbyte == 0x34， 最高位为0， 则长度信息是0x34
*/
int pkcs7::get_length(unsigned char lenbyte, int offset) {
    int len = 0, num;
    unsigned char tmp;
    if (lenbyte & 0x80) {
        num = lenbyte & 0x7f;
        if (num < 0 || num > 4) {
            printf("its too long !\n");
            return 0;
        }
        while (num) {
            len <<= 8;
            tmp = m_content[offset++];
            len += (tmp & 0xff);
            num--;
        }
    } else {
        len = lenbyte & 0xff;
    }
    return len;
}

/**
 *根据名字找到pkcs7中的元素, 若没有找到返回NULL.
 *name: 名字，可以只提供元素名字前面的字符
 *begin: 查找的开始位置
 */
element *pkcs7::get_element(const char *name, element *begin) {
    if (begin == NULL)
        begin = head;
    element *p = begin;
    while (p != NULL) {
        if (strncmp(p->name, name, strlen(name)) == 0)
            return p;
        p = p->next;
    }
    printf("not found the \"%s\"\n", name);
    return p;
}

/**
 * 创建element.pkcs7中的每个元素都有对应element.
 */
int pkcs7::create_element(unsigned char tag, char *name, int level) {
    unsigned char get_tag = m_content[m_pos++];
    if (get_tag != tag) {
        m_pos--;
        return -1;
    }
    unsigned char lenbyte = m_content[m_pos];
    int len = get_length(lenbyte, m_pos + 1);
    m_pos += len_num(lenbyte);

    element *node = (element *) malloc(sizeof(element));
    node->tag = get_tag;
    strcpy(node->name, name);
    node->begin = m_pos;
    node->len = len;
    node->level = level;
    node->next = NULL;

    if (head == NULL) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
    return len;
}

/**
 * 解析证书信息
 */
bool pkcs7::parse_certificate(int level) {
    char *names[] = {
            "tbsCertificate",
            "version",
            "serialNumber",
            "signature",
            "issuer",
            "validity",
            "subject",
            "subjectPublicKeyInfo",
            "issuerUniqueID-[optional]",
            "subjectUniqueID-[optional]",
            "extensions-[optional]",
            "signatureAlgorithm",
            "signatureValue"};
    int len = 0;
    unsigned char tag;
    bool have_version = false;
    len = create_element(TAG_SEQUENCE, names[0], level);
    if (len == -1 || m_pos + len > m_length) {
        return false;
    }
    //version
    tag = m_content[m_pos];
    if (((tag & 0xc0) == 0x80) && ((tag & 0x1f) == 0)) {
        m_pos += 1;
        m_pos += len_num(m_content[m_pos]);
        len = create_element(TAG_INTEGER, names[1], level + 1);
        if (len == -1 || m_pos + len > m_length) {
            return false;
        }
        m_pos += len;
        have_version = true;
    }

    for (int i = 2; i < 11; i++) {
        switch (i) {
            case 2:
                tag = TAG_INTEGER;
                break;
            case 8:
                tag = 0xA1;
                break;
            case 9:
                tag = 0xA2;
                break;
            case 10:
                tag = 0xA3;
                break;
            default:
                tag = TAG_SEQUENCE;
        }
        len = create_element(tag, names[i], level + 1);
        if (i < 8 && len == -1) {
            return false;
        }
        if (len != -1)
            m_pos += len;
    }
    //signatureAlgorithm
    len = create_element(TAG_SEQUENCE, names[11], level);
    if (len == -1 || m_pos + len > m_length) {
        return false;
    }
    m_pos += len;
    //signatureValue
    len = create_element(TAG_BITSTRING, names[12], level);
    if (len == -1 || m_pos + len > m_length) {
        return false;
    }
    m_pos += len;
    return true;
}

/**
 * 解析签名者信息
 */
bool pkcs7::parse_signerInfo(int level) {
    char *names[] = {
            "version",
            "issuerAndSerialNumber",
            "digestAlgorithmId",
            "authenticatedAttributes-[optional]",
            "digestEncryptionAlgorithmId",
            "encryptedDigest",
            "unauthenticatedAttributes-[optional]"};
    int len;
    unsigned char tag;
    for (int i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        switch (i) {
            case 0:
                tag = TAG_INTEGER;
                break;
            case 3:
                tag = 0xA0;
                break;
            case 5:
                tag = TAG_OCTETSTRING;
                break;
            case 6:
                tag = 0xA1;
                break;
            default:
                tag = TAG_SEQUENCE;

        }
        len = create_element(tag, names[i], level);
        if (len == -1 || m_pos + len > m_length) {
            if (i == 3 || i == 6)
                continue;
            return false;
        }
        m_pos += len;
    }
    int ret = (m_pos == m_length ? 1 : 0);
    return true;
}

/**
 * 解析 contentType == signedData 的content部分
 */
bool pkcs7::parse_content(int level) {

    char *names[] = {"version",
                     "DigestAlgorithms",
                     "contentInfo",
                     "certificates-[optional]",
                     "crls-[optional]",
                     "signerInfos",
                     "signerInfo"};

    unsigned char tag;
    int len = 0;
    element *p = NULL;
    //version
    len = create_element(TAG_INTEGER, names[0], level);
    if (len == -1 || m_pos + len > m_length) {
        return false;
    }
    m_pos += len;
    //DigestAlgorithms
    len = create_element(TAG_SET, names[1], level);
    if (len == -1 || m_pos + len > m_length) {
        return false;
    }
    m_pos += len;
    //contentInfo
    len = create_element(TAG_SEQUENCE, names[2], level);
    if (len == -1 || m_pos + len > m_length) {
        return false;
    }
    m_pos += len;
    //certificates-[optional]
    tag = m_content[m_pos];
    if (tag == TAG_OPTIONAL) {
        m_pos++;
        m_pos += len_num(m_content[m_pos]);
        len = create_element(TAG_SEQUENCE, names[3], level);
        if (len == -1 || m_pos + len > m_length) {
            return false;
        }
        p_cert = tail;
        bool ret = parse_certificate(level + 1);
        if (!ret) {
            return ret;
        }
    }
    //crls-[optional]
    tag = m_content[m_pos];
    if (tag == 0xA1) {
        m_pos++;
        m_pos += len_num(m_content[m_pos]);
        len = create_element(TAG_SEQUENCE, names[4], level);
        if (len == -1 || m_pos + len > m_length) {
            return false;
        }
        m_pos += len;
    }
    //signerInfos
    tag = m_content[m_pos];
    if (tag != TAG_SET) {
        return false;
    }
    len = create_element(TAG_SET, names[5], level);
    if (len == -1 || m_pos + len > m_length) {
        return false;
    }
    //signerInfo
    len = create_element(TAG_SEQUENCE, names[6], level + 1);
    if (len == -1 || m_pos + len > m_length) {
        return false;
    }
    p_signer = tail;
    return parse_signerInfo(level + 2);
}

/**
 * 解析文件开始函数
 */
bool pkcs7::parse_pkcs7() {
    unsigned char tag, lenbyte;
    int len = 0;
    int level = 0;
    tag = m_content[m_pos++];
    if (tag != TAG_SEQUENCE) {
        printf("not found the Tag indicating an ASN.1!\n");
        return false;
    }
    lenbyte = m_content[m_pos];
    len = get_length(lenbyte, m_pos + 1);
    m_pos += len_num(lenbyte);
    if (m_pos + len > m_length)
        return false;
    //contentType
    len = create_element(TAG_OBJECTID, "contentType", level);
    if (len == -1) {
        printf("not found the ContentType!\n");
        return false;
    }
    m_pos += len;
    //optional
    tag = m_content[m_pos++];
    lenbyte = m_content[m_pos];
    m_pos += len_num(lenbyte);
    //content-[optional]
    len = create_element(TAG_SEQUENCE, "content-[optional]", level);
    if (len == -1) {
        printf("not found the content!\n");
        return false;
    }
    return parse_content(level + 1);
}


char *pkcs7::getSign() {
    element *p = head;
    element *e = get_element("certificates-[optional]", p);
    if (!e) {
        return NULL;
    }
    int len = 0;
    int begin = 0;
    int lenByte = num_from_len(e->len); //考虑证书长度，如果小于0x7f，则只有1位，否则会有一个0x8x的标志位，用来标识长度
    len = e->len + 1 + lenByte;
    begin = e->begin - 1 - lenByte;
    if (len <= 0 || begin <= 0 || len + begin > m_length) { return NULL; }
    char *sign = (char *) malloc((2 * len + 1) * sizeof(char));
    for (int i = 0; i < len; i++) {
        sprintf(sign + 2 * i, "%02x", m_content[begin + i]);
    }
    sign[2 * len] = '\0';
    return sign;
}
