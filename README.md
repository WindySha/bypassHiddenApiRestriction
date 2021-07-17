## BypassHiddenApiRestriction
This is a library that can bypass the restrictions on non-SDK interfaces on Android 9-12.

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
    implementation 'io.github.windysha:bypassHiddenApiRestriction:1.0.2'
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
