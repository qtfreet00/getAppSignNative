#### Android获取签名

pkcs7 解析参考 ：https://github.com/W-WTerDan/pkcs7

原项目对pkcs7文件格式进行了解析

在解析字段里可以找到Apk所必须的签名字段，对字段进行拼接可以得到等价于java里signature.toCharString()的值

Android 4.x-P 不依赖java api直接获取app签名。

代码写的比较粗糙，有更好的方法或者存在bug欢迎提出