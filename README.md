## BypassHiddenApiRestriction
This is a library that can bypass the restrictions on non-SDK interfaces on Android 9-12.
## How it works
[另一种绕过Android 9以上非SDK接口调用限制的方法](https://windysha.github.io/2021/07/20/%E5%8F%A6%E4%B8%80%E7%A7%8D%E7%BB%95%E8%BF%87%20Android%209%E4%BB%A5%E4%B8%8A%E9%9D%9ESDK%E6%8E%A5%E5%8F%A3%E8%B0%83%E7%94%A8%E9%99%90%E5%88%B6%E7%9A%84%E6%96%B9%E6%B3%95/)

## Usage
### 1. Add dependency in build.gradle
```
allprojects {
    repositories {
        mavenCentral()
    }
}
```
```
dependencies {
    implementation 'io.github.windysha:bypassHiddenApiRestriction:1.1.0'
}
```
### 2. Add this code to your Application.attachBaseContext
```
import com.wind.hiddenapi.bypass.HiddenApiBypass

if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
    HiddenApiBypass.startBypass();
}
```
## License
```
Copyright 2021 WindySha
Apache License, Version 2.0
http://www.apache.org/licenses/LICENSE-2.0
```
