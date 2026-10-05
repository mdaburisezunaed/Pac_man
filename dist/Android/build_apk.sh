#!/bin/bash
set -e

SDK_ROOT="${ANDROID_SDK_ROOT:-/opt/homebrew/share/android-commandlinetools}"
BUILD_TOOLS="$SDK_ROOT/build-tools/34.0.0"
PLATFORM="$SDK_ROOT/platforms/android-34/android.jar"
JAVA_HOME="${JAVA_HOME:-/opt/homebrew/opt/openjdk}"
PATH="$JAVA_HOME/bin:$BUILD_TOOLS:$PATH"

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$PROJECT_ROOT/app/src/main"
WORK_DIR="/tmp/apk_build"
OUT_APK="$(cd "$PROJECT_ROOT/.." && pwd)/Pac-Man.apk"
KEYSTORE="$PROJECT_ROOT/pacman.keystore"

echo "=== Building Android APK for Pac-Man ==="
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR/gen" "$WORK_DIR/classes" "$WORK_DIR/dex"

echo "1. Compiling resources..."
aapt2 compile --dir "$SRC_DIR/res" -o "$WORK_DIR/compiled_res.zip"

echo "2. Linking resources and generating R.java..."
aapt2 link -I "$PLATFORM" \
    --min-sdk-version 21 \
    --target-sdk-version 34 \
    --manifest "$SRC_DIR/AndroidManifest.xml" \
    --java "$WORK_DIR/gen" \
    -o "$WORK_DIR/resources.apk" \
    "$WORK_DIR/compiled_res.zip"

echo "3. Compiling Java sources..."
javac -source 1.8 -target 1.8 \
    -cp "$PLATFORM" \
    -d "$WORK_DIR/classes" \
    $(find "$SRC_DIR/java" -name "*.java") \
    $(find "$WORK_DIR/gen" -name "*.java")

echo "4. Converting bytecode to classes.dex (d8)..."
d8 --output "$WORK_DIR/dex" \
   --lib "$PLATFORM" \
   $(find "$WORK_DIR/classes" -name "*.class")

echo "5. Packaging assets and resources into APK..."
aapt2 link -I "$PLATFORM" \
    --min-sdk-version 21 \
    --target-sdk-version 34 \
    --manifest "$SRC_DIR/AndroidManifest.xml" \
    -A "$SRC_DIR/assets" \
    -o "$WORK_DIR/unaligned.apk" \
    "$WORK_DIR/compiled_res.zip"

cd "$WORK_DIR/dex"
zip -u "$WORK_DIR/unaligned.apk" classes.dex

echo "6. Aligning package (zipalign)..."
zipalign -v -p 4 "$WORK_DIR/unaligned.apk" "$WORK_DIR/aligned.apk"

echo "7. Signing APK (apksigner)..."
if [ ! -f "$KEYSTORE" ]; then
    keytool -genkeypair -v \
        -keystore "$KEYSTORE" \
        -alias pacman \
        -keyalg RSA \
        -keysize 2048 \
        -validity 10000 \
        -storepass "pacman123" \
        -keypass "pacman123" \
        -dname "CN=Md. Abu Rise Zunaed, OU=Arcade, O=Zunaed, L=Dhaka, ST=Dhaka, C=BD"
fi

apksigner sign \
    --ks "$KEYSTORE" \
    --ks-pass "pass:pacman123" \
    --key-pass "pass:pacman123" \
    --out "$OUT_APK" \
    "$WORK_DIR/aligned.apk"

echo "8. Verifying signature..."
apksigner verify -v "$OUT_APK"

echo "=== SUCCESS! Standalone APK created at: $OUT_APK ==="
