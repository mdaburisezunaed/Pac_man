# Pac-Man

**Developer:** Md. Abu Rise Zunaed

---

## Controls

- **Move:** Arrow Keys / `W`, `A`, `S`, `D`
- **Start / Restart:** `Space` / `Enter`
- **Pause:** `P`
- **Mute:** `M`
- **Quit:** `Esc`

---

# Play Instantly (No Building Required)

Pre-packaged versions are ready for all platforms so anyone can download and play immediately without installing compilers or build tools.

### 🍏 macOS (One-Click App)
- Double-click **`Pac-Man.app`** in the project folder (or download from [GitHub Releases](https://github.com/mdaburisezunaed/Pac_man/releases)).
- **No Homebrew or SDL required:** Dynamic libraries (`SDL2` and `SDL3`) are pre-bundled inside the application bundle.

### 📱 iPhone & iPad (iOS & iPadOS)
- **Direct Sideload Package (IPA):** Download **[`Pac-Man.ipa`](https://github.com/mdaburisezunaed/Pac_man/releases/download/v1.0.0/Pac-Man.ipa)** (or from the `dist/` folder) and install with **AltStore**, **Sideloadly**, **TrollStore**, **Scarlet**, or Xcode/Apple Configurator. Full native ARM64 Mach-O app with embedded arcade audio and touch controls.
- **1-Tap Profile Install:** Download **[`Pac-Man.mobileconfig`](https://github.com/mdaburisezunaed/Pac_man/releases/download/v1.0.0/Pac-Man.mobileconfig)** on iOS, tap *Allow*, then go to *Settings ➔ Profile Downloaded ➔ Install* to place the full-screen arcade app directly on your Home Screen.
- **Safari Instant Play:** Open `web/index.html` in **Safari**, tap **Share** ➔ **Add to Home Screen**.

### 🤖 Android (APK & Native Mobile)
- **Direct APK Install (Download & Play):** Download **[`Pac-Man.apk`](https://github.com/mdaburisezunaed/Pac_man/releases/download/v1.0.0/Pac-Man.apk)** (or from the `dist/` folder) directly onto your Android device and tap to install! Fully offline, hardware accelerated, immersive sticky fullscreen with enlarged right-side arcade touch D-pad & swipe controls. Signed by **Md. Abu Rise Zunaed**.

> ⚠️ **Android Install Warning / Play Protect**  
> When installing the APK directly on Android, Google Play Protect or your system package installer may display an *"Unrecognized app"* or *"File might be harmful"* message because it was downloaded outside the Google Play Store.  
>   
> **To install the game:**  
> 1. When the prompt appears, tap **More details** (or **More info**).  
> 2. Tap **Install anyway**.  
> 3. If prompted by your browser or file manager, enable **"Allow from this source"**.  
> 4. The game will install and launch smoothly!

- **Instant 1-Tap WebAPK:** Open `web/index.html` in **Google Chrome**, tap the **Menu (⋮)** in the top right ➔ tap **"Install app"** (or "Add to Home screen").
- **Android Studio Project Package:** Download **`dist/Pac-Man-Android.zip`** containing the full source project and `./build_apk.sh`.

### 🪟 Windows (PC)
- **One-Click Native Game:** Download **`dist/Pac-Man-Windows.zip`** (or download from [GitHub Releases](https://github.com/mdaburisezunaed/Pac_man/releases)), **extract the zip file first**, and double-click **`pacman.exe`** to play immediately! All runtime libraries (`SDL2.dll`, `libwinpthread`) and the custom arcade icon are pre-bundled—zero building or installation required.

> ⚠️ **Windows SmartScreen Warning**  
> When running the game for the first time, Windows may display a **“Windows protected your PC”** message because the game is currently unsigned and not yet recognized by Microsoft.  
> This does not necessarily mean there is a problem with the game.  
>   
> **To run the game:**  
> 1. Extract the downloaded zip file first.  
> 2. Open the game's `.exe` file (`pacman.exe`).  
> 3. If the “Windows protected your PC” window appears, click **More info**.  
> 4. Click **Run anyway**.  
> 5. The game should launch normally.  
>   
> *Note: Only choose “Run anyway” if you downloaded the game from the official source/repository and you trust the file. As the game becomes officially published and signed, this warning may no longer appear.*

- **Web Desktop App:** Open `web/index.html` in **Microsoft Edge** or **Google Chrome**, then click the **"Install"** button in the address bar to install Pac-Man as a native Windows desktop app with desktop shortcut.

---

# Installation & Running (From Source)

## macOS

### Step 1: Install Command Line Tools

Open **Terminal** and run:

```bash
xcode-select --install
```

Follow the installation prompts.

### Step 2: Install Homebrew

If Homebrew is not already installed, run:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Add Homebrew to your PATH:

```bash
(echo; echo 'eval "$(/opt/homebrew/bin/brew shellenv)"') >> ~/.zprofile
eval "$(/opt/homebrew/bin/brew shellenv)"
```

Verify the installation:

```bash
brew --version
```

### Step 3: Install SDL2

```bash
brew install sdl2
```

### Step 4: Navigate to the Project Directory

Open Terminal and use `cd` to enter the Pac-Man project folder.

Example:

```bash
cd ~/Desktop/Codes/Pac_man
```

> **Tip:** You can type `cd ` and drag the project folder into Terminal to automatically enter its path.

### Step 5: Build the Game

```bash
make
```

### Step 6: Run the Game

```bash
./pacman
```
Enjoy the game!

---

## Windows — MSYS2

### Step 1: Install MSYS2

Download and install MSYS2 from:
https://www.msys2.org/

### Step 2: Open MSYS2 UCRT64

After installation, open **MSYS2 UCRT64** from the Start Menu.

> **Important:** Use the **MSYS2 UCRT64** terminal for the following steps.

### Step 3: Update MSYS2

```bash
pacman -Syu --noconfirm
```

If MSYS2 asks you to close and reopen the terminal, do so and run the command again.

### Step 4: Install GCC, Make, and SDL2

```bash
pacman -S --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-sdl2
```

### Step 5: Navigate to the Project Directory

For example, if the project is located at `C:\Users\YourName\Desktop\Pac_man`, run:

```bash
cd /c/Users/YourName/Desktop/Pac_man
```

### Step 6: Build the Game

```bash
mingw32-make
```

Alternatively, you can compile directly with:

```bash
g++ -std=c++17 -O2 main.cpp $(sdl2-config --cflags --libs) -o pacman.exe
```

### Step 7: Run the Game

```bash
./pacman.exe
```

---

## Windows — WSL2

### Step 1: Install WSL2

Open **Windows PowerShell as Administrator** and run:

```powershell
wsl --install
```

Restart your computer if prompted.

### Step 2: Open Ubuntu

After restarting, open **Ubuntu** from the Start Menu.

### Step 3: Install Build Tools and SDL2

```bash
sudo apt update
sudo apt install -y build-essential clang libsdl2-dev
```

### Step 4: Navigate to the Project Directory

If the project is stored on the Windows `C:` drive, access it through `/mnt/c`:

```bash
cd /mnt/c/Users/YourName/Desktop/Pac_man
```

### Step 5: Build the Game

```bash
make
```

### Step 6: Run the Game

```bash
./pacman
```

> **Note:** The Pac-Man window requires Linux GUI support. Windows 11 with WSLg provides this automatically.

---

## Linux — Ubuntu / Debian

### Step 1: Open Terminal

Open your terminal application.

### Step 2: Install Build Tools and SDL2

```bash
sudo apt update
sudo apt install -y build-essential clang libsdl2-dev
```

### Step 3: Navigate to the Project Directory

```bash
cd ~/Pac_man
```

### Step 4: Build the Game

```bash
make
```

### Step 5: Run the Game

```bash
./pacman
```

---

# Troubleshooting

### `macOS: "Pac-Man is damaged and can’t be opened"`

This is standard macOS Gatekeeper quarantine for apps downloaded from the web. Open Terminal and run:

```bash
xattr -cr /path/to/Pac-Man.app
```

Then double-click `Pac-Man.app` to open.

### `make: command not found`

Make sure you installed the required build tools for your operating system (`xcode-select --install` on Mac, or `build-essential` on Linux).

### `SDL2/SDL.h: No such file or directory`

Make sure SDL2 is installed correctly and that you completed the SDL2 installation step for your operating system.

### `./pacman: Permission denied`

On macOS or Linux, run:

```bash
chmod +x pacman
```

Then run `./pacman`.

---

# Developer

- **Lead Developer:** Md. Abu Rise Zunaed
- **Copyright:** © 2026 Md. Abu Rise Zunaed. All Rights Reserved.
