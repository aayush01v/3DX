#!/bin/bash
# CarXSimulator APK Build Script with Auto-Install

set -e

echo "🏎️ CarXSimulator APK Builder"
echo "============================"
echo ""

# Detect OS
OS="$(uname -s)"
ARCH="$(uname -m)"

# ============================================
# JDK Installation
# ============================================
install_jdk() {
    echo "📥 Installing JDK 17..."
    
    if [ "$OS" = "Linux" ]; then
        if command -v apt-get &> /dev/null; then
            sudo apt-get update
            sudo apt-get install -y openjdk-17-jdk
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y java-17-openjdk-devel
        elif command -v pacman &> /dev/null; then
            sudo pacman -S --noconfirm jdk17-openjdk
        elif command -v yum &> /dev/null; then
            sudo yum install -y java-17-openjdk-devel
        else
            echo "❌ Unknown package manager. Install JDK 17 manually."
            exit 1
        fi
    elif [ "$OS" = "Darwin" ]; then
        if command -v brew &> /dev/null; then
            brew install openjdk@17
            sudo ln -sfn $(brew --prefix)/opt/openjdk@17/libexec/openjdk.jdk /Library/Java/JavaVirtualMachines/openjdk-17.jdk
        else
            echo "❌ Install Homebrew first: /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
            exit 1
        fi
    else
        echo "❌ Unsupported OS: $OS"
        exit 1
    fi
    
    echo "✅ JDK 17 installed"
}

# ============================================
# Android SDK Installation
# ============================================
install_android_sdk() {
    echo "📥 Installing Android SDK..."
    
    SDK_DIR="$HOME/Android/Sdk"
    CMDLINE_TOOLS_URL=""
    
    if [ "$OS" = "Linux" ]; then
        CMDLINE_TOOLS_URL="https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip"
    elif [ "$OS" = "Darwin" ]; then
        CMDLINE_TOOLS_URL="https://dl.google.com/android/repository/commandlinetools-mac-11076708_latest.zip"
    fi
    
    mkdir -p "$SDK_DIR/cmdline-tools"
    cd "$SDK_DIR/cmdline-tools"
    
    echo "   Downloading command-line tools..."
    curl -L -o cmdline-tools.zip "$CMDLINE_TOOLS_URL"
    unzip -q -o cmdline-tools.zip
    rm cmdline-tools.zip
    mv cmdline-tools latest 2>/dev/null || true
    
    cd - > /dev/null
    
    export ANDROID_HOME="$SDK_DIR"
    export PATH="$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools"
    
    echo "   Installing SDK components..."
    yes | sdkmanager --licenses > /dev/null 2>&1 || true
    sdkmanager "platform-tools" "platforms;android-34" "build-tools;34.0.0"
    
    echo "✅ Android SDK installed at $SDK_DIR"
}

# ============================================
# Android NDK Installation
# ============================================
install_ndk() {
    echo "📥 Installing Android NDK..."
    
    if [ -z "$ANDROID_HOME" ]; then
        export ANDROID_HOME="$HOME/Android/Sdk"
    fi
    
    export PATH="$PATH:$ANDROID_HOME/cmdline-tools/latest/bin"
    
    sdkmanager "ndk;26.1.10909125"
    
    export ANDROID_NDK_HOME="$ANDROID_HOME/ndk/26.1.10909125"
    
    echo "✅ NDK installed at $ANDROID_NDK_HOME"
}

# ============================================
# Check and Install Dependencies
# ============================================
echo "🔍 Checking dependencies..."
echo ""

# Check JDK
if ! command -v java &> /dev/null; then
    echo "⚠️  JDK not found"
    read -p "   Install JDK 17? [Y/n] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
        install_jdk
    else
        echo "❌ JDK required. Exiting."
        exit 1
    fi
else
    JAVA_VER=$(java -version 2>&1 | head -n 1 | cut -d'"' -f2 | cut -d'.' -f1)
    echo "✅ JDK found (version $JAVA_VER)"
fi

# Check Android SDK
if [ -z "$ANDROID_HOME" ]; then
    if [ -d "$HOME/Android/Sdk" ]; then
        export ANDROID_HOME="$HOME/Android/Sdk"
    elif [ -d "/usr/local/android-sdk" ]; then
        export ANDROID_HOME="/usr/local/android-sdk"
    fi
fi

if [ -z "$ANDROID_HOME" ] || [ ! -d "$ANDROID_HOME" ]; then
    echo "⚠️  Android SDK not found"
    read -p "   Install Android SDK? [Y/n] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
        install_android_sdk
    else
        echo "❌ Android SDK required. Exiting."
        exit 1
    fi
else
    echo "✅ Android SDK found at $ANDROID_HOME"
fi

# Check NDK
if [ -z "$ANDROID_NDK_HOME" ]; then
    if [ -d "$ANDROID_HOME/ndk" ]; then
        NDK_VER=$(ls "$ANDROID_HOME/ndk" 2>/dev/null | sort -V | tail -1)
        if [ -n "$NDK_VER" ]; then
            export ANDROID_NDK_HOME="$ANDROID_HOME/ndk/$NDK_VER"
        fi
    fi
fi

if [ -z "$ANDROID_NDK_HOME" ] || [ ! -d "$ANDROID_NDK_HOME" ]; then
    echo "⚠️  Android NDK not found"
    read -p "   Install Android NDK? [Y/n] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
        install_ndk
    else
        echo "⚠️  NDK not installed - native C++ code won't compile"
    fi
else
    echo "✅ Android NDK found at $ANDROID_NDK_HOME"
fi

# Export paths
export PATH="$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools"

echo ""
echo "📋 Environment:"
echo "   ANDROID_HOME=$ANDROID_HOME"
echo "   ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
echo ""

# ============================================
# Build APK
# ============================================

# Create assets directory and copy web files
echo "📦 Copying web assets..."
mkdir -p app/src/main/assets/www
cp -r ../preview/* app/src/main/assets/www/ 2>/dev/null || echo "   (no preview files to copy)"

# Make gradlew executable
chmod +x gradlew 2>/dev/null || true

# Build APK
echo "🔨 Building APK..."
echo ""

if [ -f "./gradlew" ]; then
    ./gradlew assembleDebug --no-daemon
elif command -v gradle &> /dev/null; then
    gradle assembleDebug
else
    echo "❌ Gradle not found"
    exit 1
fi

# Check output
APK_PATH="app/build/outputs/apk/debug/app-debug.apk"
if [ -f "$APK_PATH" ]; then
    echo ""
    echo "════════════════════════════════════════"
    echo "✅ APK built successfully!"
    echo "════════════════════════════════════════"
    echo "📍 Location: $APK_PATH"
    echo "📊 Size: $(du -h "$APK_PATH" | cut -f1)"
    echo ""
    echo "📱 Install commands:"
    echo "   adb install -r $APK_PATH"
    echo ""
    echo "   Or copy to phone and install manually"
else
    echo ""
    echo "❌ Build failed. Check errors above."
    exit 1
fi
