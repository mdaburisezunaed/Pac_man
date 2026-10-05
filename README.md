# Pac-Man Arcade

**Developer:** Md. Abu Rise Zunaed

---

## Download & Play Instantly (No Building Required)

Pre-packaged versions are ready for all platforms so anyone can download and play immediately without installing compilers or build tools.

### 🍏 macOS (One-Click App)
- Double-click **`Pac-Man.app`** located in the project folder (or in `dist/Pac-Man-macOS.zip`).
- **No Homebrew or SDL2 installation required:** All dynamic libraries and high-resolution icons are bundled directly inside the application bundle.

### 📱 iPhone & iPad (iOS / iPadOS)
- Open `web/index.html` in **Safari**.
- Tap the **Share** button (box with upward arrow) ➔ tap **"Add to Home Screen"**.
- A native **Pac-Man** app icon will appear on your home screen. Tap it to play in fullscreen with touch controls (swipe & virtual D-pad) and synthesized arcade audio.

### 🤖 Android (Smartphones & Tablets)
- Open `web/index.html` in **Google Chrome**.
- Tap the **Menu (⋮)** in the top right ➔ tap **"Install app"** (or "Add to Home screen").
- Pac-Man installs directly to your Android app drawer with the official logo. Opens in full screen with touch and swipe controls.

### 🪟 Windows (PC)
- **Instant Play:** Open `web/index.html` in **Microsoft Edge** or **Google Chrome**, then click the **"Install"** button in the address bar to install Pac-Man as a native Windows desktop app with desktop shortcut and icon.
- **Native .EXE Launcher:** Double-click `dist/Windows/Play_Pacman.bat` to launch or automatically build `pacman.exe`.

---

## Game Controls

### Desktop (Mac / Windows / Linux)
- **Move:** Arrow Keys or `W`, `A`, `S`, `D`
- **Start / Restart:** `Space` or `Enter`
- **Pause / Unpause:** `P`
- **Mute / Unmute Audio:** `M`
- **Quit:** `Esc`

### Mobile & Tablet (iPhone / iPad / Android)
- **Virtual D-Pad:** Directional buttons (▲, ▼, ◀, ▶)
- **Swipe Gestures:** Swipe anywhere on the maze in any direction
- **Action Buttons:** Tap **START / ENTER**, **PAUSE**, or **SOUND**

---

## Developer
- **Lead Developer:** Md. Abu Rise Zunaed
- **Copyright:** © 2026 Md. Abu Rise Zunaed. All Rights Reserved.

---

## Building from Source (Optional)

If you prefer compiling the C++ source code manually:

### macOS Manual Build
```bash
# 1. Install SDL2
brew install sdl2

# 2. Build
make

# 3. Run
./pacman
```

### Windows (MSYS2) Manual Build
```bash
# In MSYS2 UCRT64 terminal:
pacman -S --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-sdl2
cd /path/to/Pac_man
mingw32-make
./pacman.exe
```

### Linux (Ubuntu / Debian) Manual Build
```bash
sudo apt update && sudo apt install -y build-essential clang libsdl2-dev
make
./pacman
```
