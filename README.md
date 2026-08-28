# depthai-qr-example

Android app for QR code detection using DepthAI and OAK cameras.

## Features

- Real-time RGB camera feed
- Stereo depth detection
- QR code detection via MobileNetDetectionNetwork
- Native C++ integration via JNI with DepthAI core library
- Compose UI

## Prerequisites

- Android SDK 33+
- Android NDK 28.2.13676358
- CMake 3.22.1+ (included with Android Studio)

## Setup

### 1. Clone Repository

```bash
git clone <repo-url>
cd depthai-qr-example
```

### 2. Build DepthAI Core (Already Included)

The prebuilt `depthai-core` binaries and headers are included in the repo at `depthai-core/`. These were built with custom tweaks for Android arm64-v8a.

If you need to rebuild (optional):
- Requires DepthAI core source
- Run build in `depthai-core/build-android` with appropriate toolchain
- Output goes to `depthai-core/` directory

### 3. Open in Android Studio

1. Open Android Studio
2. Select "Open an existing Android Studio project"
3. Navigate to this directory
4. Let Gradle sync complete

### 4. Build & Run

```bash
./gradlew build          # Build APK
./gradlew installDebug   # Install on connected device
```

Or use Android Studio's Build menu.

## Project Structure

```
depthai-qr-example/
├── app/
│   ├── src/
│   │   ├── main/
│   │   │   ├── java/com/example/depthai_qr/
│   │   │   │   ├── DepthaiBridge.kt                # JNI bridge to native code
│   │   │   │   ├── MainActivity.kt                 # Main activity
│   │   │   │   ├── model/
│   │   │   │   │   ├── QrDetection.kt
│   │   │   │   │   └── QrOverlay.kt
│   │   │   │   └── ui/
│   │   │   │       ├── CameraWithOverlay.kt
│   │   │   │       └── theme/
│   │   │   │           ├── Color.kt
│   │   │   │           ├── Theme.kt
│   │   │   │           └── Type.kt
│   │   │   ├── cpp/
│   │   │   │   └── native-lib.cpp                  # C++ JNI bindings
│   │   │   ├── include/                            # DepthAI headers & dependencies
│   │   │   ├── assets/
│   │   │   │   └── qr_code_detection_384x384.blob  # Detection model blob
│   │   │   ├── res/                                # Android resources
│   │   │   ├── jniLibs/
│   │   │   │   └── arm64-v8a/                      # Prebuilt native libraries
│   │   │   ├── AndroidManifest.xml
│   │   │   └── CMakeLists.txt                      # CMake build config
│   │   ├── androidTest/
│   │   └── test/
│   ├── build.gradle.kts                    # App build config
│   └── proguard-rules.pro
├── depthai-core/                           # Prebuilt DepthAI core
│   ├── include/                            # DepthAI headers
│   └── lib/cmake/                          # CMake configs
├── gradle/                                 # Gradle wrapper files
├── build.gradle.kts                        # Root build config
├── settings.gradle.kts
├── gradle.properties
├── gradlew
├── gradlew.bat
└── README.md
```

## Architecture

- **JNI Bridge** (`DepthaiBridge.kt`): Kotlin interface to native C++ code
- **Native Layer** (`native-lib.cpp`): DepthAI pipeline setup and frame access
- **UI** (`MainActivity.kt`, `CameraWithOverlay.kt`): Compose-based camera view with QR detection overlay

## Native Dependencies

Compiled for `arm64-v8a`:
- **libdepthai-core.so**: OAK camera pipeline & device control
- **libusb-1.0.so**: USB communication
- **libc++_shared.so**: C++ standard library

Headers and CMake files included for:
- depthai-core
- libusb-1.0
- OpenCV (optional, currently unused)
- XLink, nop, nlohmann/json

## Building for Different Architectures

Currently configured for `arm64-v8a` only. To add more ABIs:

1. Edit `app/build.gradle.kts` line 22:
   ```kotlin
   abiFilters += listOf("arm64-v8a", "x86_64")  // Add x86_64 or armeabi-v7a
   ```

2. Rebuild depthai-core for those architectures and place binaries in `depthai-core/lib/...`

## Troubleshooting

### Build fails: "libusb-1.0/libusb.h not found"
- Verify `depthai-core/` directory exists at project root
- Run `./gradlew clean build` to rebuild CMake cache

### Device connection issues
- Ensure USB debugging enabled on Android device
- Verify app has camera & USB permissions (AndroidManifest.xml)
- OAK camera must be connected via USB

## License

[Add your license here]
