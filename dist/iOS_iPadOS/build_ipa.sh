#!/bin/bash
set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DIST_DIR="$PROJECT_ROOT/dist"
IOS_DIR="$DIST_DIR/iOS_iPadOS"
BUILD_DIR="/tmp/ipa_build"
OUT_IPA="$DIST_DIR/Pac-Man.ipa"

echo "=== Building iOS IPA for Pac-Man ==="
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR/Payload/Pac-Man.app"

APP_DIR="$BUILD_DIR/Payload/Pac-Man.app"

echo "1. Compiling iOS Mach-O arm64 binary..."
cat << 'MAIN_M' > "$BUILD_DIR/main.m"
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <objc/runtime.h>
#import <objc/message.h>
#include <dlfcn.h>

@interface PacmanAppDelegate : NSObject
@property (strong, nonatomic) id window;
@end

@implementation PacmanAppDelegate
- (BOOL)application:(id)application didFinishLaunchingWithOptions:(id)launchOptions {
    Class uiScreen = objc_getClass("UIScreen");
    id mainScreen = ((id (*)(id, SEL))objc_msgSend)(uiScreen, sel_registerName("mainScreen"));
    CGRect bounds = ((CGRect (*)(id, SEL))objc_msgSend)(mainScreen, sel_registerName("bounds"));

    Class uiWindow = objc_getClass("UIWindow");
    id win = ((id (*)(id, SEL, CGRect))objc_msgSend)([uiWindow alloc], sel_registerName("initWithFrame:"), bounds);

    Class uiColor = objc_getClass("UIColor");
    id bgColor = ((id (*)(id, SEL, CGFloat, CGFloat, CGFloat, CGFloat))objc_msgSend)(
        uiColor, sel_registerName("colorWithRed:green:blue:alpha:"), 5.0/255.0, 8.0/255.0, 20.0/255.0, 1.0);
    ((void (*)(id, SEL, id))objc_msgSend)(win, sel_registerName("setBackgroundColor:"), bgColor);

    Class uiViewController = objc_getClass("UIViewController");
    id vc = [[uiViewController alloc] init];
    id vcView = ((id (*)(id, SEL))objc_msgSend)(vc, sel_registerName("view"));
    ((void (*)(id, SEL, id))objc_msgSend)(vcView, sel_registerName("setBackgroundColor:"), bgColor);

    Class wkConfigClass = objc_getClass("WKWebViewConfiguration");
    id config = [[wkConfigClass alloc] init];
    ((void (*)(id, SEL, BOOL))objc_msgSend)(config, sel_registerName("setAllowsInlineMediaPlayback:"), YES);
    ((void (*)(id, SEL, NSInteger))objc_msgSend)(config, sel_registerName("setMediaTypesRequiringUserActionForPlayback:"), 0);

    Class wkWebViewClass = objc_getClass("WKWebView");
    CGRect vcBounds = ((CGRect (*)(id, SEL))objc_msgSend)(vcView, sel_registerName("bounds"));
    id webView = ((id (*)(id, SEL, CGRect, id))objc_msgSend)(
        [wkWebViewClass alloc], sel_registerName("initWithFrame:configuration:"), vcBounds, config);

    ((void (*)(id, SEL, NSUInteger))objc_msgSend)(webView, sel_registerName("setAutoresizingMask:"), 18);

    NSBundle *bundle = [NSBundle mainBundle];
    NSURL *url = [bundle URLForResource:@"index" withExtension:@"html"];
    if (url) {
        ((id (*)(id, SEL, id, id))objc_msgSend)(
            webView, sel_registerName("loadFileURL:allowingReadAccessToURL:"), url, [bundle bundleURL]);
    }

    ((void (*)(id, SEL, id))objc_msgSend)(vcView, sel_registerName("addSubview:"), webView);
    ((void (*)(id, SEL, id))objc_msgSend)(win, sel_registerName("setRootViewController:"), vc);
    ((void (*)(id, SEL))objc_msgSend)(win, sel_registerName("makeKeyAndVisible"));

    self.window = win;
    return YES;
}
@end

int main(int argc, char * argv[]) {
    @autoreleasepool {
        void *handle = dlopen("/System/Library/Frameworks/UIKit.framework/UIKit", RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            handle = dlopen("/System/iOSSupport/System/Library/Frameworks/UIKit.framework/UIKit", RTLD_NOW | RTLD_GLOBAL);
        }
        dlopen("/System/Library/Frameworks/WebKit.framework/WebKit", RTLD_NOW | RTLD_GLOBAL);

        typedef int (*UIApplicationMainFunc)(int, char *[], id, id);
        UIApplicationMainFunc uiAppMain = (UIApplicationMainFunc)dlsym(RTLD_DEFAULT, "UIApplicationMain");
        if (!uiAppMain && handle) {
            uiAppMain = (UIApplicationMainFunc)dlsym(handle, "UIApplicationMain");
        }

        if (uiAppMain) {
            return uiAppMain(argc, argv, nil, NSStringFromClass([PacmanAppDelegate class]));
        } else {
            NSLog(@"[Pac-Man iOS] UIKit UIApplicationMain not found");
            return 1;
        }
    }
}
MAIN_M

clang -target arm64-apple-ios14.0-macabi \
    -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk \
    -framework Foundation \
    -framework CoreGraphics \
    -fobjc-arc \
    "$BUILD_DIR/main.m" \
    -o "$APP_DIR/Pac-Man"

echo "2. Setting Mach-O platform to iOS..."
vtool -set-build-version ios 14.0 17.0 -replace -output "$APP_DIR/Pac-Man" "$APP_DIR/Pac-Man"
chmod +x "$APP_DIR/Pac-Man"

echo "3. Copying web assets into bundle..."
cp "$DIST_DIR/universal_web/index.html" "$APP_DIR/"
cp "$DIST_DIR/universal_web/pacman.js" "$APP_DIR/"
cp "$DIST_DIR/universal_web/manifest.json" "$APP_DIR/"
cp "$DIST_DIR/universal_web/sw.js" "$APP_DIR/"
cp "$PROJECT_ROOT/assets/logo.png" "$APP_DIR/"
cp -R "$PROJECT_ROOT/assets/icons" "$APP_DIR/"

echo "4. Generating iOS app icons..."
sips -z 120 120 "$PROJECT_ROOT/assets/icons/icon-512.png" --out "$APP_DIR/AppIcon60x60@2x.png" > /dev/null
sips -z 180 180 "$PROJECT_ROOT/assets/icons/icon-512.png" --out "$APP_DIR/AppIcon60x60@3x.png" > /dev/null
sips -z 152 152 "$PROJECT_ROOT/assets/icons/icon-512.png" --out "$APP_DIR/AppIcon76x76@2x~ipad.png" > /dev/null
sips -z 167 167 "$PROJECT_ROOT/assets/icons/icon-512.png" --out "$APP_DIR/AppIcon83.5x83.5@2x~ipad.png" > /dev/null

echo "5. Generating Info.plist..."
cat << 'PLIST' > "$APP_DIR/Info.plist"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleDisplayName</key>
    <string>Pac-Man</string>
    <key>CFBundleExecutable</key>
    <string>Pac-Man</string>
    <key>CFBundleIdentifier</key>
    <string>com.zunaed.pacman</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>Pac-Man</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSRequiresIPhoneOS</key>
    <true/>
    <key>MinimumOSVersion</key>
    <string>14.0</string>
    <key>UIDeviceFamily</key>
    <array>
        <integer>1</integer>
        <integer>2</integer>
    </array>
    <key>UIRequiredDeviceCapabilities</key>
    <array>
        <string>arm64</string>
    </array>
    <key>UISupportedInterfaceOrientations</key>
    <array>
        <string>UIInterfaceOrientationPortrait</string>
    </array>
    <key>UISupportedInterfaceOrientations~ipad</key>
    <array>
        <string>UIInterfaceOrientationPortrait</string>
        <string>UIInterfaceOrientationPortraitUpsideDown</string>
        <string>UIInterfaceOrientationLandscapeLeft</string>
        <string>UIInterfaceOrientationLandscapeRight</string>
    </array>
    <key>UIViewControllerBasedStatusBarAppearance</key>
    <false/>
    <key>UIStatusBarHidden</key>
    <true/>
    <key>CFBundleIcons</key>
    <dict>
        <key>CFBundlePrimaryIcon</key>
        <dict>
            <key>CFBundleIconFiles</key>
            <array>
                <string>AppIcon60x60</string>
            </array>
        </dict>
    </dict>
    <key>Developer</key>
    <string>Md. Abu Rise Zunaed</string>
    <key>Author</key>
    <string>Md. Abu Rise Zunaed</string>
</dict>
</plist>
PLIST

echo "6. Ad-hoc codesigning app bundle..."
codesign --force --sign - --timestamp=none "$APP_DIR"

echo "7. Packaging into Pac-Man.ipa..."
rm -f "$OUT_IPA"
cd "$BUILD_DIR"
zip -qry "$OUT_IPA" Payload

echo "=== SUCCESS! Pac-Man.ipa built at: $OUT_IPA ==="
