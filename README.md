#### Android获取签名

不依赖任何java api直接获取app签名。

支持如下签名方式

```
    private String getSignCharString() {
        try {
            return getPackageManager().getPackageInfo(getPackageName(), PackageManager.GET_SIGNATURES).signatures[0].toCharsString();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return "";
    }


    private byte[] getSignByteArray() {
        try {
            return getPackageManager().getPackageInfo(getPackageName(), PackageManager.GET_SIGNATURES).signatures[0].toByteArray();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return null;
    }

    private int getSignHashCode() {
        try {
            return getPackageManager().getPackageInfo(getPackageName(), PackageManager.GET_SIGNATURES).signatures[0].hashCode();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return 0;
    }
```

对应Jni实现

```
Signature.toCharString()

Signature.toByteArray()

Signature.toHashCode()
```

代码写的比较粗糙，有更好的方法或者存在bug欢迎提出

##### 参考：

pkcs7文件解析 ：https://github.com/W-WTerDan/pkcs7