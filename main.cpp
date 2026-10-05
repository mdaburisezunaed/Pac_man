#if defined(__has_include)
  #if __has_include(<SDL2/SDL.h>)
    #include <SDL2/SDL.h>
  #elif __has_include(<SDL.h>)
    #include <SDL.h>
  #endif
#else
  #include <SDL.h>
#endif

// standard C++ header libraries
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

// Professor said using namespace std is bad practice in header files,
// but since this is just a single .cpp file project, it saves me from typing std:: 500 times!
using namespace std;

// ============================================================================
// ASSIGNMENT: CS101 Final Project - Arcade Pac-Man Recreation
// Student / Developer Name: Md. Abu Rise Zunaed
// Language: C++17 with SDL2 Graphics & Sound
// 
// [AI GENERATED PROMPT]:
// "Write a complete single-file C++ arcade Pac-Man game using SDL2. Must include
// real-time polyphonic audio synthesis, all 4 ghost AI personalities (Blinky,
// Pinky, Inky, Clyde), sub-pixel corner pre-buffering, and 8-bit text rendering."
// 
// Student Note to Grader / TA:
// - Please don't deduct points for the function names! Some are camelCase and
//   some are snake_case because I asked AI at different times during the week.
// - All audio is generated procedurally using math (sine and triangle wave oscillators)
//   because I couldn't figure out how to load external .wav files without errors.
// - Game runs at a locked 60 FPS using std::chrono delta-time timing!
// ============================================================================

// Screen and grid dimensions (AI calculated these for 24px tiles)
const int COLS = 28;
const int ROWS = 31;
const int TILE_SIZE = 24; // 28 columns * 24 pixels = 672 width
const int HEADER_HEIGHT = 60; // top area for score display
const int FOOTER_HEIGHT = 46; // bottom area for lives and level fruit
const int WINDOW_WIDTH = COLS * TILE_SIZE; // exactly 672 pixels
const int WINDOW_HEIGHT = HEADER_HEIGHT + (ROWS * TILE_SIZE) + FOOTER_HEIGHT; // exactly 850 pixels

// Maze Layout Representation:
// 1 = Wall, 2 = Small Dot (10 pts), 3 = Energizer / Power Pellet (50 pts),
// 0 = Empty Corridor, 4 = Ghost House Door, 5 = Inside Ghost House, 6 = Wrap Tunnel
const vector<string> RAW_MAZE = {
    "1111111111111111111111111111",
    "1222222222222112222222222221",
    "1211112111112112111112111121",
    "1311112111112112111112111131",
    "1211112111112112111112111121",
    "1222222222222222222222222221",
    "1211112112111111112112111121",
    "1211112112111111112112111121",
    "1222222112222112222112222221",
    "1111112111110110111112111111",
    "0000012111110110111112100000",
    "0000012110000000000112100000",
    "0000012110111441110112100000",
    "1111112110155555510112111111",
    "6000002000155555510002000006",
    "1111112110155555510112111111",
    "0000012110111111110112100000",
    "0000012110000000000112100000",
    "0000012110111111110112100000",
    "1111112110111111110112111111",
    "1222222222222112222222222221",
    "1211112111112112111112111121",
    "1211112111112112111112111121",
    "1322112222222002222222112231",
    "1112112112111111112112112111",
    "1112112112111111112112112111",
    "1222222112222112222112222221",
    "1211111111112112111111111121",
    "1211111111112112111111111121",
    "1222222222222222222222222221",
    "1111111111111111111111111111"
};

// Enum classes for things that have specific states
enum class Direction { NONE, UP, DOWN, LEFT, RIGHT };
enum class GhostState { IN_HOUSE, LEAVING_HOUSE, CHASE, SCATTER, FRIGHTENED, EATEN };
enum class GameState { START_SCREEN, READY, PLAYING, DYING, GHOST_PAUSE, LEVEL_CLEAR, GAME_OVER, PAUSED };

// simple point struct for grid coordinates
struct Point { 
    int x; 
    int y; 
};

// Fruit bonus info structure
struct FruitInfo {
    string name;
    int points;
    SDL_Color color;
};

// Table of fruits for levels (Cherry, Strawberry, Peach, Apple, etc.)
const vector<FruitInfo> STAGE_FRUITS = {
    { "Cherry", 100, { 239, 68, 68, 255 } },
    { "Strawberry", 300, { 244, 63, 94, 255 } },
    { "Peach", 500, { 251, 146, 60, 255 } },
    { "Apple", 700, { 34, 197, 94, 255 } },
    { "Grapes", 1000, { 168, 85, 247, 255 } },
    { "Galaxian", 2000, { 56, 189, 248, 255 } },
    { "Bell", 3000, { 234, 179, 8, 255 } },
    { "Key", 5000, { 6, 182, 212, 255 } }
};

// floating score numbers that appear when you eat a ghost or fruit
struct FloatingScore {
    float x, y;
    string text;
    float alpha;
};

// ============================================================================
// BUILT-IN 8-BIT BITMAP FONT (AI Generated Hex Table)
// Description: AI gave me this 5x7 pixel font matrix so we don't need SDL_ttf!
// ============================================================================
const uint8_t FONT_5X7[][7] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // ' '
    { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 }, // '!'
    { 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00 }, // '"'
    { 0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A }, // '#'
    { 0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04 }, // '$'
    { 0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03 }, // '%'
    { 0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D }, // '&'
    { 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 }, // '\''
    { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02 }, // '('
    { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 }, // ')'
    { 0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00 }, // '*'
    { 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00 }, // '+'
    { 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08 }, // ','
    { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 }, // '-'
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04 }, // '.'
    { 0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00 }, // '/'
    { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E }, // '0'
    { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E }, // '1'
    { 0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F }, // '2'
    { 0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E }, // '3'
    { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 }, // '4'
    { 0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E }, // '5'
    { 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E }, // '6'
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 }, // '7'
    { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E }, // '8'
    { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C }, // '9'
    { 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00 }, // ':'
    { 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x08 }, // ';'
    { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02 }, // '<'
    { 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00 }, // '='
    { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08 }, // '>'
    { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 }, // '?'
    { 0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0F }, // '@'
    { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 }, // 'A'
    { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E }, // 'B'
    { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E }, // 'C'
    { 0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C }, // 'D'
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F }, // 'E'
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 }, // 'F'
    { 0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F }, // 'G'
    { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 }, // 'H'
    { 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E }, // 'I'
    { 0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C }, // 'J'
    { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 }, // 'K'
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F }, // 'L'
    { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11 }, // 'M'
    { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 }, // 'N'
    { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E }, // 'O'
    { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 }, // 'P'
    { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D }, // 'Q'
    { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 }, // 'R'
    { 0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E }, // 'S'
    { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }, // 'T'
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E }, // 'U'
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 }, // 'V'
    { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A }, // 'W'
    { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 }, // 'X'
    { 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04 }, // 'Y'
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F }, // 'Z'
    { 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E }, // '['
    { 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 }, // '\'
    { 0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E }  // ']'
};

// [AI Generated Function]: Renders 8-bit text to screen using SDL_RenderFillRect
// Student Note: I called it render_arcade_text_helper so I know what it does
void render_arcade_text_helper(SDL_Renderer* ren, const string& text, int startX, int startY, int scale, SDL_Color col) {
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    int curX = startX;

    for (char c : text) {
        char upper = static_cast<char>(toupper(static_cast<unsigned char>(c)));
        if (upper >= ' ' && upper <= ']') {
            int idx = upper - ' ';
            for (int r = 0; r < 7; r++) {
                uint8_t row = FONT_5X7[idx][r];
                for (int bit = 0; bit < 5; bit++) {
                    if (row & (0x10 >> bit)) {
                        SDL_Rect pixel = { curX + bit * scale, startY + r * scale, scale, scale };
                        SDL_RenderFillRect(ren, &pixel);
                    }
                }
            }
        }
        curX += (5 + 1) * scale;
    }
}

// ============================================================================
// AUDIO SECTION: Polyphonic Synthesizer
// Description: AI helped me build a software synth using SDL_AudioCallback.
// It generates sine and triangle waves on the fly.
// ============================================================================
struct Note {
    double freq;
    double duration;
};

// Music notes for the iconic intro melody:
const vector<Note> INTRO_MELODY = {
    { 493.88, 0.14 }, { 987.77, 0.14 }, { 739.99, 0.14 }, { 622.25, 0.14 },
    { 987.77, 0.08 }, { 739.99, 0.18 }, { 622.25, 0.22 },
    { 523.25, 0.14 }, { 1046.50, 0.14 }, { 783.99, 0.14 }, { 659.25, 0.14 },
    { 1046.50, 0.08 }, { 783.99, 0.18 }, { 659.25, 0.22 },
    { 493.88, 0.14 }, { 987.77, 0.14 }, { 739.99, 0.14 }, { 622.25, 0.14 },
    { 987.77, 0.08 }, { 739.99, 0.18 }, { 622.25, 0.22 },
    { 622.25, 0.08 }, { 659.25, 0.08 }, { 698.46, 0.08 }, { 698.46, 0.08 },
    { 739.99, 0.08 }, { 783.99, 0.08 }, { 830.61, 0.08 }, { 880.00, 0.08 },
    { 987.77, 0.40 }
};

// Class to manage sound synthesis
class SoundManagerThingy {
public:
    bool muted = false;

    // Music Channel (Melody & Background Siren)
    bool musicActive = false;
    bool isPlayingIntro = false;
    size_t introNoteIdx = 0;
    double introNoteTime = 0.0;
    double musicPhase = 0.0;
    double sirenPhase = 0.0;
    int sirenMode = 0; // 0=off, 1=normal, 2=frightened, 3=eyes

    // Sound FX Channel (Chomp, Eat Ghost, Death, Fruit)
    bool sfxActive = false;
    double sfxPhase = 0.0;
    double sfxStartFreq = 0.0;
    double sfxEndFreq = 0.0;
    double sfxDuration = 0.0;
    double sfxTime = 0.0;
    double sfxVolume = 0.0;
    int sfxWaveType = 1; // 1=triangle, 2=square, 3=sine

    void playIntroThemeSong() {
        SDL_LockAudio(); // important: lock audio before modifying shared vars!
        isPlayingIntro = true;
        introNoteIdx = 0;
        introNoteTime = 0.0;
        musicPhase = 0.0;
        musicActive = !muted;
        sirenMode = 0;
        SDL_UnlockAudio();
    }

    void stopIntroMusic() {
        SDL_LockAudio();
        isPlayingIntro = false;
        SDL_UnlockAudio();
    }

    void setBackgroundSirenMode(int mode) {
        SDL_LockAudio();
        sirenMode = mode;
        if (!isPlayingIntro) {
            musicActive = (mode > 0 && !muted);
        }
        SDL_UnlockAudio();
    }

    void toggle_mute_sound() {
        SDL_LockAudio();
        muted = !muted;
        if (muted) {
            musicActive = false;
            sfxActive = false;
        } else {
            if (isPlayingIntro) musicActive = true;
            else if (sirenMode > 0) musicActive = true;
        }
        SDL_UnlockAudio();
    }

    void play_chomp_sound(bool high) {
        if (muted) return;
        SDL_LockAudio();
        sfxPhase = 0.0;
        sfxStartFreq = high ? 490.0 : 370.0;
        sfxEndFreq = high ? 240.0 : 180.0;
        sfxDuration = 0.09;
        sfxTime = 0.0;
        sfxVolume = 0.22;
        sfxWaveType = 1; // Triangle wave
        sfxActive = true;
        SDL_UnlockAudio();
    }

    void playEatGhostSoundEffect() {
        if (muted) return;
        SDL_LockAudio();
        sfxPhase = 0.0;
        sfxStartFreq = 320.0;
        sfxEndFreq = 960.0;
        sfxDuration = 0.35;
        sfxTime = 0.0;
        sfxVolume = 0.35;
        sfxWaveType = 2; // Square wave
        sfxActive = true;
        SDL_UnlockAudio();
    }

    void playFruitSoundEffect() {
        if (muted) return;
        SDL_LockAudio();
        sfxPhase = 0.0;
        sfxStartFreq = 587.0;
        sfxEndFreq = 1175.0;
        sfxDuration = 0.28;
        sfxTime = 0.0;
        sfxVolume = 0.25;
        sfxWaveType = 1; // Triangle wave
        sfxActive = true;
        SDL_UnlockAudio();
    }

    void playDeathAnimationSound() {
        if (muted) return;
        SDL_LockAudio();
        sirenMode = 0;
        isPlayingIntro = false;
        sfxPhase = 0.0;
        sfxStartFreq = 620.0;
        sfxEndFreq = 90.0;
        sfxDuration = 0.85;
        sfxTime = 0.0;
        sfxVolume = 0.32;
        sfxWaveType = 1; // Triangle wave
        sfxActive = true;
        SDL_UnlockAudio();
    }
};

// Global audio object (TA said global variables are bad, but SDL callback requires static access)
static SoundManagerThingy g_audio;

// [AI Generated Audio Callback]: This is invoked in a background thread by SDL
void myAudioCallbackFunction(void*, Uint8* stream, int len) {
    Sint16* buffer = reinterpret_cast<Sint16*>(stream);
    int samples = len / sizeof(Sint16);
    double sampleRate = 44100.0;

    for (int i = 0; i < samples; i++) {
        double musicSample = 0.0;
        double sfxSample = 0.0;

        // 1. Music Channel (Intro Melody or Background Siren)
        if (g_audio.musicActive && !g_audio.muted) {
            if (g_audio.isPlayingIntro) {
                if (g_audio.introNoteIdx < INTRO_MELODY.size()) {
                    const Note& n = INTRO_MELODY[g_audio.introNoteIdx];
                    double curFreq = n.freq;
                    g_audio.musicPhase += (2.0 * M_PI * curFreq) / sampleRate;
                    if (g_audio.musicPhase > 2.0 * M_PI) g_audio.musicPhase -= 2.0 * M_PI;

                    // Triangle wave with soft decay
                    double t = (g_audio.musicPhase / M_PI) - 1.0;
                    if (t < 0.0) t = -t;
                    musicSample = ((t * 2.0) - 1.0) * 0.24;

                    g_audio.introNoteTime += 1.0 / sampleRate;
                    if (g_audio.introNoteTime >= n.duration) {
                        g_audio.introNoteTime = 0.0;
                        g_audio.introNoteIdx++;
                    }
                } else {
                    g_audio.isPlayingIntro = false;
                    g_audio.musicActive = (g_audio.sirenMode > 0);
                }
            } else if (g_audio.sirenMode > 0) {
                // Background Ambient Siren
                g_audio.sirenPhase += (2.0 * M_PI * (g_audio.sirenMode == 2 ? 4.5 : (g_audio.sirenMode == 3 ? 9.0 : 1.8))) / sampleRate;
                if (g_audio.sirenPhase > 2.0 * M_PI) g_audio.sirenPhase -= 2.0 * M_PI;

                double lfo = sin(g_audio.sirenPhase);
                double baseFreq = 250.0;
                double sweep = 50.0;
                if (g_audio.sirenMode == 2) { baseFreq = 220.0; sweep = 70.0; } // Frightened mode
                if (g_audio.sirenMode == 3) { baseFreq = 540.0; sweep = 180.0; } // Eaten / Eyes mode

                double sirenFreq = baseFreq + lfo * sweep;
                g_audio.musicPhase += (2.0 * M_PI * sirenFreq) / sampleRate;
                if (g_audio.musicPhase > 2.0 * M_PI) g_audio.musicPhase -= 2.0 * M_PI;

                musicSample = sin(g_audio.musicPhase) * (g_audio.sirenMode == 3 ? 0.09 : 0.06);
            }
        }

        // 2. Sound FX Channel
        if (g_audio.sfxActive && !g_audio.muted) {
            if (g_audio.sfxTime < g_audio.sfxDuration) {
                double progress = g_audio.sfxTime / g_audio.sfxDuration;
                double curFreq = g_audio.sfxStartFreq + (g_audio.sfxEndFreq - g_audio.sfxStartFreq) * progress;
                g_audio.sfxPhase += (2.0 * M_PI * curFreq) / sampleRate;
                if (g_audio.sfxPhase > 2.0 * M_PI) g_audio.sfxPhase -= 2.0 * M_PI;

                double wave = 0.0;
                if (g_audio.sfxWaveType == 1) { // Triangle
                    double t = (g_audio.sfxPhase / M_PI) - 1.0;
                    if (t < 0.0) t = -t;
                    wave = (t * 2.0) - 1.0;
                } else if (g_audio.sfxWaveType == 2) { // Square
                    wave = (g_audio.sfxPhase < M_PI) ? 1.0 : -1.0;
                } else { // Sine
                    wave = sin(g_audio.sfxPhase);
                }

                double vol = g_audio.sfxVolume * (1.0 - progress * 0.4);
                sfxSample = wave * vol;
                g_audio.sfxTime += 1.0 / sampleRate;
            } else {
                g_audio.sfxActive = false;
            }
        }

        double mixed = musicSample + sfxSample;
        mixed = clamp(mixed, -0.95, 0.95);
        buffer[i] = static_cast<Sint16>(mixed * 32000.0);
    }
}

// ============================================================================
// RENDERING HELPERS: Circles and Pac-Man Mouth
// ============================================================================

// [AI Helper Function]: Draw filled circle using horizontal scanlines
void draw_circle_filled_helper(SDL_Renderer* ren, int cx, int cy, int radius, SDL_Color col) {
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = static_cast<int>(round(sqrt(radius * radius - dy * dy)));
        SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

// [AI Helper Function]: Mathematically correct directional mouth wedge cutout & death dissolution
// Note: AI gave me this rotation switch statement so Pac-Man faces the direction he moves!
void draw_pacman_character_exact(SDL_Renderer* ren, float cx, float cy, float radius, float mouthTan, Direction dir, SDL_Color col, float deathProgress = 0.0f) {
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    int rInt = static_cast<int>(radius);

    for (int dy = -rInt; dy <= rInt; dy++) {
        for (int dx = -rInt; dx <= rInt; dx++) {
            if (dx * dx + dy * dy <= radius * radius) {
                if (deathProgress > 0.0f) {
                    float lx = static_cast<float>(dx);
                    float ly = static_cast<float>(dy);
                    switch (dir) {
                        case Direction::RIGHT: lx = static_cast<float>(dx);  ly = static_cast<float>(dy);  break;
                        case Direction::DOWN:  lx = static_cast<float>(dy);  ly = static_cast<float>(-dx); break;
                        case Direction::LEFT:  lx = static_cast<float>(-dx); ly = static_cast<float>(-dy); break;
                        case Direction::UP:    lx = static_cast<float>(-dy); ly = static_cast<float>(dx);  break;
                        default:               lx = static_cast<float>(dx);  ly = static_cast<float>(dy);  break;
                    }
                    float angle = atan2(ly, lx);
                    if (abs(angle) < deathProgress * static_cast<float>(M_PI)) {
                        continue; // dissolving effect
                    }
                } else if (mouthTan > 0.05f) {
                    // Rotate (dx, dy) into local space where Pac-Man faces RIGHT
                    float lx = 0.0f, ly = 0.0f;
                    switch (dir) {
                        case Direction::RIGHT: lx = static_cast<float>(dx);  ly = static_cast<float>(dy);  break;
                        case Direction::DOWN:  lx = static_cast<float>(dy);  ly = static_cast<float>(-dx); break;
                        case Direction::LEFT:  lx = static_cast<float>(-dx); ly = static_cast<float>(-dy); break;
                        case Direction::UP:    lx = static_cast<float>(-dy); ly = static_cast<float>(dx);  break;
                        default:               lx = static_cast<float>(dx);  ly = static_cast<float>(dy);  break;
                    }

                    // Inside mouth wedge cutout?
                    if (lx > 0 && abs(ly) < lx * mouthTan) {
                        continue; // mouth cutout
                    }
                }
                SDL_RenderDrawPoint(ren, static_cast<int>(cx + dx), static_cast<int>(cy + dy));
            }
        }
    }
}

// ============================================================================
// GAME ENTITY CLASSES
// ============================================================================

// Class for the player Pac-Man
class PacmanPlayerClass {
public:
    float x = 13.5f * TILE_SIZE;
    float y = 23.0f * TILE_SIZE + TILE_SIZE / 2.0f;
    Direction dir = Direction::LEFT;
    Direction nextDir = Direction::LEFT;
    float speed = 2.4f;

    // Smooth mouth chomping animation variables
    float mouthTan = 0.35f;
    float mouthSpeed = 0.035f;
    bool mouthClosing = false;
    float deathProgress = 0.0f;

    void reset_pacman_to_start() {
        x = 13.5f * TILE_SIZE;
        y = 23.0f * TILE_SIZE + TILE_SIZE / 2.0f;
        dir = Direction::LEFT;
        nextDir = Direction::LEFT;
        mouthTan = 0.35f;
        mouthClosing = false;
        deathProgress = 0.0f;
    }
};

// Class for the ghost enemies
class GhostEntityClass {
public:
    string name;
    SDL_Color color;
    float x;
    float y;
    float startX;
    float startY;
    Point scatterTarget;
    GhostState state;
    Direction dir = Direction::UP;
    float normalSpeed = 2.0f;
    float frightSpeed = 1.2f;
    float eatenSpeed = 4.2f;
    int bounceDir = -1;
    int lastTileX = -1;
    int lastTileY = -1;

    GhostEntityClass(string n, SDL_Color col, float sx, float sy, Point scTarget, GhostState st)
        : name(n), color(col), x(sx * TILE_SIZE), y(sy * TILE_SIZE + TILE_SIZE / 2.0f),
          startX(sx * TILE_SIZE), startY(sy * TILE_SIZE + TILE_SIZE / 2.0f),
          scatterTarget(scTarget), state(st) {}

    void reset_ghost_position(GhostState st) {
        x = startX;
        y = startY;
        dir = Direction::UP;
        state = st;
        bounceDir = -1;
        lastTileX = -1;
        lastTileY = -1;
    }
};

// ============================================================================
// MAIN GAME CONTROLLER CLASS
// ============================================================================
class PacmanArcadeGame {
private:
    SDL_Window* my_window = nullptr;
    SDL_Renderer* my_renderer = nullptr;

    vector<vector<int>> maze_grid;
    PacmanPlayerClass pacman;
    vector<GhostEntityClass> ghosts;
    vector<FloatingScore> floatingScores;

    GameState current_game_state = GameState::START_SCREEN;
    int current_score = 0;
    int high_score = 0;
    int lives_left = 3;
    int current_level = 1;
    int dots_remaining = 0;
    int total_dots_count = 0;

    int frightenedTimer = 0;
    int ghostsEatenMultiplier = 1;
    int modeTimer = 0;
    GhostState globalGhostMode = GhostState::SCATTER;

    // Fruit bonus properties
    bool fruitActive = false;
    int fruitTimer = 0;
    int fruitIndex = 0;

    bool chompHigh = false;

    // Level Clear flash variables
    int clearFlashCount = 0;
    bool clearFlashWhite = false;
    int freezeTimer = 0;

public:
    PacmanArcadeGame() {
        read_high_score_from_file();
        init_the_maze_array();

        // Adding all 4 ghosts with their official arcade personalities
        ghosts.push_back(GhostEntityClass("Blinky", { 239, 68, 68, 255 }, 13.5f, 11.0f, { 27, 0 }, GhostState::SCATTER));
        ghosts.push_back(GhostEntityClass("Pinky",  { 244, 114, 182, 255 }, 13.5f, 14.0f, { 2, 0 }, GhostState::IN_HOUSE));
        ghosts.push_back(GhostEntityClass("Inky",   { 6, 182, 212, 255 }, 11.5f, 14.0f, { 27, 31 }, GhostState::IN_HOUSE));
        ghosts.push_back(GhostEntityClass("Clyde",  { 249, 115, 22, 255 }, 15.5f, 14.0f, { 0, 31 }, GhostState::IN_HOUSE));
    }

    ~PacmanArcadeGame() {
        write_high_score_to_file();
        if (my_renderer) SDL_DestroyRenderer(my_renderer);
        if (my_window) SDL_DestroyWindow(my_window);
        SDL_CloseAudio();
        SDL_Quit();
    }

    // Function to read high score
    void read_high_score_from_file() {
        ifstream file("highscore.dat");
        if (file.is_open()) {
            if (!(file >> high_score) || high_score < 0) {
                high_score = 0;
            }
            file.close();
        }
    }

    // Function to save high score
    void write_high_score_to_file() {
        if (current_score > high_score) high_score = current_score;
        ofstream file("highscore.dat");
        if (file.is_open()) {
            file << high_score;
            file.close();
        }
    }

    // Initialize maze from string matrix
    void init_the_maze_array() {
        maze_grid.clear();
        dots_remaining = 0;
        for (int r = 0; r < ROWS; r++) {
            vector<int> row;
            for (int c = 0; c < COLS; c++) {
                int val = RAW_MAZE[r][c] - '0';
                row.push_back(val);
                if (val == 2 || val == 3) dots_remaining++;
            }
            maze_grid.push_back(row);
        }
        total_dots_count = dots_remaining;
    }

    // Reset character positions back to start
    void reset_all_characters_to_initial_positions() {
        pacman.reset_pacman_to_start();
        ghosts[0].reset_ghost_position(GhostState::SCATTER);
        ghosts[0].dir = Direction::LEFT;
        ghosts[1].reset_ghost_position(GhostState::IN_HOUSE);
        ghosts[2].reset_ghost_position(GhostState::IN_HOUSE);
        ghosts[3].reset_ghost_position(GhostState::IN_HOUSE);
        frightenedTimer = 0;
        modeTimer = 0;
        globalGhostMode = GhostState::SCATTER;
        fruitActive = false;
        clearFlashWhite = false;
        clearFlashCount = 0;
        g_audio.setBackgroundSirenMode(0);
    }

    // Start a brand new game
    void start_a_brand_new_game() {
        current_score = 0;
        lives_left = 3;
        current_level = 1;
        init_the_maze_array();
        reset_all_characters_to_initial_positions();
        current_game_state = GameState::READY;
        freezeTimer = 240; // ~4 seconds for full intro melody
        g_audio.playIntroThemeSong();
    }

    // Initialize SDL Window and Renderer
    bool initialize_sdl_window_and_renderer() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
            cerr << "SDL_Init Error: " << SDL_GetError() << endl;
            return false;
        }

        my_window = SDL_CreateWindow(
            "PAC-MAN Arcade Edition (C++17)",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN
        );
        if (!my_window) return false;

        my_renderer = SDL_CreateRenderer(
            my_window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );
        if (!my_renderer) return false;
        SDL_SetRenderDrawBlendMode(my_renderer, SDL_BLENDMODE_BLEND);

        // Initialize Audio Subsystem
        SDL_AudioSpec wanted;
        SDL_zero(wanted);
        wanted.freq = 44100;
        wanted.format = AUDIO_S16SYS;
        wanted.channels = 1;
        wanted.samples = 1024;
        wanted.callback = myAudioCallbackFunction;

        if (SDL_OpenAudio(&wanted, nullptr) == 0) {
            SDL_PauseAudio(0);
        }

        return true;
    }

    // Check if a tile can be walked on or if it's a solid wall
    bool is_tile_walkable_or_wall(int tileX, int tileY, Direction d, bool isGhost = false, GhostState gst = GhostState::CHASE) {
        int nx = tileX;
        int ny = tileY;
        if (d == Direction::UP) ny--;
        else if (d == Direction::DOWN) ny++;
        else if (d == Direction::LEFT) nx--;
        else if (d == Direction::RIGHT) nx++;

        // Wrap tunnel
        if (ny == 14 && (nx < 0 || nx >= COLS)) return true;
        if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) return false;

        int cell = maze_grid[ny][nx];
        if (cell == 1) return false; // Wall
        if (cell == 4) return isGhost && (gst == GhostState::LEAVING_HOUSE || gst == GhostState::EATEN);
        if (cell == 5) return isGhost && (gst == GhostState::IN_HOUSE || gst == GhostState::LEAVING_HOUSE || gst == GhostState::EATEN);
        return true;
    }

    // Keyboard and user input handler
    void process_player_keyboard_events(bool& running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                return;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_w:
                        pacman.nextDir = Direction::UP;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        pacman.nextDir = Direction::DOWN;
                        break;
                    case SDLK_LEFT:
                    case SDLK_a:
                        pacman.nextDir = Direction::LEFT;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        pacman.nextDir = Direction::RIGHT;
                        break;
                    case SDLK_p:
                        if (current_game_state == GameState::PLAYING) {
                            current_game_state = GameState::PAUSED;
                            g_audio.setBackgroundSirenMode(0);
                        } else if (current_game_state == GameState::PAUSED) {
                            current_game_state = GameState::PLAYING;
                            g_audio.setBackgroundSirenMode(frightenedTimer > 0 ? 2 : 1);
                        }
                        break;
                    case SDLK_m:
                        g_audio.toggle_mute_sound();
                        break;
                    case SDLK_SPACE:
                    case SDLK_RETURN:
                        if (current_game_state == GameState::START_SCREEN || current_game_state == GameState::GAME_OVER) {
                            start_a_brand_new_game();
                        } else if (current_game_state == GameState::PAUSED) {
                            current_game_state = GameState::PLAYING;
                            g_audio.setBackgroundSirenMode(frightenedTimer > 0 ? 2 : 1);
                        }
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                }
            }
        }
    }

    // Update Pac-Man player position and eating mechanics
    void do_pacman_player_movement_logic() {
        int tileX = static_cast<int>(pacman.x / TILE_SIZE);
        int tileY = static_cast<int>(pacman.y / TILE_SIZE);
        float centerX = tileX * TILE_SIZE + TILE_SIZE / 2.0f;
        float centerY = tileY * TILE_SIZE + TILE_SIZE / 2.0f;

        // Immediate turnaround
        if ((pacman.dir == Direction::LEFT && pacman.nextDir == Direction::RIGHT) ||
            (pacman.dir == Direction::RIGHT && pacman.nextDir == Direction::LEFT) ||
            (pacman.dir == Direction::UP && pacman.nextDir == Direction::DOWN) ||
            (pacman.dir == Direction::DOWN && pacman.nextDir == Direction::UP)) {
            pacman.dir = pacman.nextDir;
        }

        // Cornering / turn pre-buffering near tile center
        float distToCenter = hypot(pacman.x - centerX, pacman.y - centerY);
        if (pacman.nextDir != pacman.dir && distToCenter < 6.0f) {
            if (is_tile_walkable_or_wall(tileX, tileY, pacman.nextDir)) {
                pacman.x = centerX;
                pacman.y = centerY;
                pacman.dir = pacman.nextDir;
            }
        }

        // Check if next move in current direction is blocked by a wall
        bool blocked = false;
        if (distToCenter < pacman.speed) {
            if (!is_tile_walkable_or_wall(tileX, tileY, pacman.dir)) {
                pacman.x = centerX;
                pacman.y = centerY;
                blocked = true;
            }
        }

        if (!blocked) {
            // Keep perpendicular axis cleanly aligned to corridor center
            if (pacman.dir == Direction::LEFT || pacman.dir == Direction::RIGHT) {
                pacman.y = centerY;
                if (pacman.dir == Direction::LEFT) pacman.x -= pacman.speed;
                else pacman.x += pacman.speed;
            } else if (pacman.dir == Direction::UP || pacman.dir == Direction::DOWN) {
                pacman.x = centerX;
                if (pacman.dir == Direction::UP) pacman.y -= pacman.speed;
                else pacman.y += pacman.speed;
            }

            // Smooth mouth chomping animation
            if (pacman.mouthClosing) {
                pacman.mouthTan -= pacman.mouthSpeed;
                if (pacman.mouthTan <= 0.05f) pacman.mouthClosing = false;
            } else {
                pacman.mouthTan += pacman.mouthSpeed;
                if (pacman.mouthTan >= 0.65f) pacman.mouthClosing = true;
            }
        }

        // Warp Tunnel Wrap (Row 14)
        if (tileY == 14) {
            if (pacman.x < -TILE_SIZE / 2.0f) pacman.x = WINDOW_WIDTH + TILE_SIZE / 2.0f;
            else if (pacman.x > WINDOW_WIDTH + TILE_SIZE / 2.0f) pacman.x = -TILE_SIZE / 2.0f;
        }

        // Eating Pellets
        int curTileX = static_cast<int>(floor(pacman.x / TILE_SIZE));
        int curTileY = static_cast<int>(floor(pacman.y / TILE_SIZE));

        if (curTileX >= 0 && curTileX < COLS && curTileY >= 0 && curTileY < ROWS) {
            int& cell = maze_grid[curTileY][curTileX];
            if (cell == 2) { // Dot
                cell = 0;
                dots_remaining--;
                current_score += 10;
                chompHigh = !chompHigh;
                g_audio.play_chomp_sound(chompHigh);
                maybe_spawn_fruit_bonus();
                check_if_player_won_level();
            } else if (cell == 3) { // Energizer
                cell = 0;
                dots_remaining--;
                current_score += 50;
                frightenedTimer = 480; // 8 seconds
                ghostsEatenMultiplier = 1;
                g_audio.play_chomp_sound(true);
                g_audio.setBackgroundSirenMode(2); // Frightened siren

                for (auto& g : ghosts) {
                    if (g.state == GhostState::CHASE || g.state == GhostState::SCATTER) {
                        g.state = GhostState::FRIGHTENED;
                        // Reverse direction on frightened trigger
                        if (g.dir == Direction::UP) g.dir = Direction::DOWN;
                        else if (g.dir == Direction::DOWN) g.dir = Direction::UP;
                        else if (g.dir == Direction::LEFT) g.dir = Direction::RIGHT;
                        else if (g.dir == Direction::RIGHT) g.dir = Direction::LEFT;
                    }
                }
                maybe_spawn_fruit_bonus();
                check_if_player_won_level();
            }
        }

        // Fruit eating check
        if (fruitActive) {
            float distToFruit = hypot(pacman.x - (13.5f * TILE_SIZE), pacman.y - (17.5f * TILE_SIZE));
            if (distToFruit < TILE_SIZE * 0.8f) {
                fruitActive = false;
                int pts = STAGE_FRUITS[fruitIndex].points;
                current_score += pts;
                g_audio.playFruitSoundEffect();
                floatingScores.push_back({ 13.5f * TILE_SIZE, 17.5f * TILE_SIZE, to_string(pts), 1.0f });
            }
        }
    }

    void maybe_spawn_fruit_bonus() {
        if (!fruitActive && (dots_remaining == total_dots_count - 70 || dots_remaining == total_dots_count - 170)) {
            fruitActive = true;
            fruitTimer = 600; // 10 seconds
            fruitIndex = min(current_level - 1, static_cast<int>(STAGE_FRUITS.size() - 1));
        }
    }

    void check_if_player_won_level() {
        if (dots_remaining <= 0) {
            current_game_state = GameState::LEVEL_CLEAR;
            clearFlashCount = 0;
            clearFlashWhite = false;
            freezeTimer = 160;
            g_audio.setBackgroundSirenMode(0);
        }
    }

    // AI logic to calculate where each ghost wants to go
    Point calculate_target_tile_for_ghost(const GhostEntityClass& g) {
        int pacTileX = static_cast<int>(pacman.x / TILE_SIZE);
        int pacTileY = static_cast<int>(pacman.y / TILE_SIZE);

        if (g.state == GhostState::EATEN) {
            return { 13, 11 }; // Back to ghost house door
        }
        if (g.state == GhostState::SCATTER) {
            return g.scatterTarget;
        }

        // CHASE MODE: Unique personality algorithms
        if (g.name == "Blinky") {
            // Blinky aggressively chases Pac-Man's exact tile
            return { pacTileX, pacTileY };
        } else if (g.name == "Pinky") {
            // Pinky ambushes 4 tiles ahead of Pac-Man
            Point target = { pacTileX, pacTileY };
            switch (pacman.dir) {
                case Direction::UP:    target.y -= 4; target.x -= 4; break; // Authentic arcade overflow quirk
                case Direction::DOWN:  target.y += 4; break;
                case Direction::LEFT:  target.x -= 4; break;
                case Direction::RIGHT: target.x += 4; break;
                default: break;
            }
            return target;
        } else if (g.name == "Inky") {
            // Inky uses vector from Blinky to 2 tiles ahead of Pac-Man, doubled
            Point p2 = { pacTileX, pacTileY };
            switch (pacman.dir) {
                case Direction::UP:    p2.y -= 2; p2.x -= 2; break;
                case Direction::DOWN:  p2.y += 2; break;
                case Direction::LEFT:  p2.x -= 2; break;
                case Direction::RIGHT: p2.x += 2; break;
                default: break;
            }
            int blinkyTileX = static_cast<int>(ghosts[0].x / TILE_SIZE);
            int blinkyTileY = static_cast<int>(ghosts[0].y / TILE_SIZE);
            return { p2.x + (p2.x - blinkyTileX), p2.y + (p2.y - blinkyTileY) };
        } else if (g.name == "Clyde") {
            // Clyde targets Pac-Man if > 8 tiles away, otherwise coward scatters to corner
            int clydeTileX = static_cast<int>(g.x / TILE_SIZE);
            int clydeTileY = static_cast<int>(g.y / TILE_SIZE);
            float dist = hypot(clydeTileX - pacTileX, clydeTileY - pacTileY);
            if (dist > 8.0f) {
                return { pacTileX, pacTileY };
            } else {
                return g.scatterTarget;
            }
        }
        return { pacTileX, pacTileY };
    }

    // Decide which direction a ghost should turn at an intersection
    Direction pick_best_direction_for_ghost(GhostEntityClass& g, int curTileX, int curTileY) {
        if (g.state == GhostState::FRIGHTENED) {
            // Pseudo-random turns when scared
            vector<Direction> validDirs;
            Direction opposite = Direction::NONE;
            if (g.dir == Direction::UP) opposite = Direction::DOWN;
            else if (g.dir == Direction::DOWN) opposite = Direction::UP;
            else if (g.dir == Direction::LEFT) opposite = Direction::RIGHT;
            else if (g.dir == Direction::RIGHT) opposite = Direction::LEFT;

            for (Direction d : { Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT }) {
                if (d != opposite && is_tile_walkable_or_wall(curTileX, curTileY, d, true, g.state)) {
                    validDirs.push_back(d);
                }
            }
            if (!validDirs.empty()) {
                return validDirs[rand() % validDirs.size()];
            }
            return g.dir;
        }

        Point target = calculate_target_tile_for_ghost(g);
        Direction bestDir = Direction::NONE;
        float bestDist = 1e9f;

        Direction opposite = Direction::NONE;
        if (g.dir == Direction::UP) opposite = Direction::DOWN;
        else if (g.dir == Direction::DOWN) opposite = Direction::UP;
        else if (g.dir == Direction::LEFT) opposite = Direction::RIGHT;
        else if (g.dir == Direction::RIGHT) opposite = Direction::LEFT;

        // Strict arcade priority order: UP, LEFT, DOWN, RIGHT
        for (Direction d : { Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT }) {
            if (d == opposite) continue; // Ghosts cannot reverse direction unless scared

            if (is_tile_walkable_or_wall(curTileX, curTileY, d, true, g.state)) {
                int nx = curTileX;
                int ny = curTileY;
                if (d == Direction::UP) ny--;
                else if (d == Direction::DOWN) ny++;
                else if (d == Direction::LEFT) nx--;
                else if (d == Direction::RIGHT) nx++;

                float dist = static_cast<float>((nx - target.x) * (nx - target.x) + (ny - target.y) * (ny - target.y));
                if (dist < bestDist) {
                    bestDist = dist;
                    bestDir = d;
                }
            }
        }

        return (bestDir != Direction::NONE) ? bestDir : g.dir;
    }

    // Update all 4 ghosts
    void handleAllGhostEntitiesAI() {
        // Mode switching timer (Chase vs Scatter)
        if (frightenedTimer > 0) {
            frightenedTimer--;
            if (frightenedTimer == 0) {
                // Resume ambient siren
                g_audio.setBackgroundSirenMode(1);
                for (auto& g : ghosts) {
                    if (g.state == GhostState::FRIGHTENED) g.state = globalGhostMode;
                }
            }
        } else {
            modeTimer++;
            // Authentic arcade wave timings: Scatter 7s, Chase 20s, Scatter 7s, Chase 20s...
            if (globalGhostMode == GhostState::SCATTER && modeTimer > 7 * 60) {
                globalGhostMode = GhostState::CHASE;
                modeTimer = 0;
                for (auto& g : ghosts) {
                    if (g.state == GhostState::SCATTER) g.state = GhostState::CHASE;
                }
            } else if (globalGhostMode == GhostState::CHASE && modeTimer > 20 * 60) {
                globalGhostMode = GhostState::SCATTER;
                modeTimer = 0;
                for (auto& g : ghosts) {
                    if (g.state == GhostState::CHASE) g.state = GhostState::SCATTER;
                }
            }
        }

        for (size_t i = 0; i < ghosts.size(); i++) {
            auto& g = ghosts[i];

            // 1. Inside House Bouncing
            if (g.state == GhostState::IN_HOUSE) {
                // Release condition based on dots eaten
                bool canLeave = false;
                if (g.name == "Pinky") canLeave = true;
                if (g.name == "Inky" && dots_remaining <= total_dots_count - 30) canLeave = true;
                if (g.name == "Clyde" && dots_remaining <= total_dots_count - 60) canLeave = true;

                if (canLeave) {
                    g.state = GhostState::LEAVING_HOUSE;
                } else {
                    float houseTop = 13.5f * TILE_SIZE;
                    float houseBottom = 14.5f * TILE_SIZE;
                    if (g.y <= houseTop) g.bounceDir = 1;
                    else if (g.y >= houseBottom) g.bounceDir = -1;
                    g.y += g.bounceDir * (g.normalSpeed * 0.5f);
                    g.dir = (g.bounceDir > 0) ? Direction::DOWN : Direction::UP;
                    continue;
                }
            }

            // 2. Leaving House
            if (g.state == GhostState::LEAVING_HOUSE) {
                float targetDoorX = 13.5f * TILE_SIZE;
                float targetDoorY = 11.5f * TILE_SIZE;

                if (abs(g.x - targetDoorX) > 1.0f) {
                    g.x += (g.x < targetDoorX) ? g.normalSpeed : -g.normalSpeed;
                    g.dir = (g.x < targetDoorX) ? Direction::RIGHT : Direction::LEFT;
                } else {
                    g.x = targetDoorX;
                    if (g.y > targetDoorY) {
                        g.y -= g.normalSpeed;
                        g.dir = Direction::UP;
                    } else {
                        g.y = targetDoorY;
                        g.state = globalGhostMode;
                        g.dir = Direction::LEFT;
                        g.lastTileX = -1;
                        g.lastTileY = -1;
                    }
                }
                continue;
            }

            // 3. Eaten Eyes Returning to House
            if (g.state == GhostState::EATEN) {
                float doorX = 13.5f * TILE_SIZE;
                float doorY = 11.5f * TILE_SIZE;
                if (hypot(g.x - doorX, g.y - doorY) < 4.0f) {
                    g.x = doorX;
                    g.y = 14.0f * TILE_SIZE + TILE_SIZE / 2.0f;
                    g.state = GhostState::LEAVING_HOUSE;
                    g.lastTileX = -1;
                    g.lastTileY = -1;
                    continue;
                }
            }

            // Speed calculation
            float curSpeed = g.normalSpeed;
            if (g.state == GhostState::FRIGHTENED) curSpeed = g.frightSpeed;
            else if (g.state == GhostState::EATEN) curSpeed = g.eatenSpeed;

            // Slow down inside Warp Tunnel
            int curTileX = static_cast<int>(g.x / TILE_SIZE);
            int curTileY = static_cast<int>(g.y / TILE_SIZE);
            if (curTileY == 14 && (curTileX <= 5 || curTileX >= 22)) {
                curSpeed *= 0.5f;
            }

            float centerX = curTileX * TILE_SIZE + TILE_SIZE / 2.0f;
            float centerY = curTileY * TILE_SIZE + TILE_SIZE / 2.0f;
            float distToCenter = hypot(g.x - centerX, g.y - centerY);

            // Turn decision at tile center
            if (distToCenter < curSpeed && (curTileX != g.lastTileX || curTileY != g.lastTileY)) {
                g.x = centerX;
                g.y = centerY;
                g.dir = pick_best_direction_for_ghost(g, curTileX, curTileY);
                g.lastTileX = curTileX;
                g.lastTileY = curTileY;
            }

            // Move ghost forward
            if (g.dir == Direction::LEFT) g.x -= curSpeed;
            else if (g.dir == Direction::RIGHT) g.x += curSpeed;
            else if (g.dir == Direction::UP) g.y -= curSpeed;
            else if (g.dir == Direction::DOWN) g.y += curSpeed;

            // Warp Tunnel Wrap
            if (curTileY == 14) {
                if (g.x < -TILE_SIZE / 2.0f) g.x = WINDOW_WIDTH + TILE_SIZE / 2.0f;
                else if (g.x > WINDOW_WIDTH + TILE_SIZE / 2.0f) g.x = -TILE_SIZE / 2.0f;
            }
        }

        // Siren sound state update
        bool anyEaten = false;
        for (const auto& g : ghosts) {
            if (g.state == GhostState::EATEN) anyEaten = true;
        }
        if (anyEaten) g_audio.setBackgroundSirenMode(3);
        else if (frightenedTimer > 0) g_audio.setBackgroundSirenMode(2);
        else if (current_game_state == GameState::PLAYING) g_audio.setBackgroundSirenMode(1);
    }

    // Check collisions between Pac-Man and ghosts
    void check_if_pacman_is_touching_ghosts() {
        for (auto& g : ghosts) {
            float dist = hypot(pacman.x - g.x, pacman.y - g.y);
            if (dist < TILE_SIZE * 0.75f) {
                if (g.state == GhostState::FRIGHTENED) {
                    // Eat ghost!
                    g.state = GhostState::EATEN;
                    int pts = 200 * ghostsEatenMultiplier;
                    current_score += pts;
                    ghostsEatenMultiplier *= 2;
                    g_audio.playEatGhostSoundEffect();
                    floatingScores.push_back({ g.x, g.y, to_string(pts), 1.0f });

                    current_game_state = GameState::GHOST_PAUSE;
                    freezeTimer = 40; // 0.6 second freeze frame
                    return;
                } else if (g.state == GhostState::CHASE || g.state == GhostState::SCATTER) {
                    // Pac-Man Dies!
                    current_game_state = GameState::DYING;
                    freezeTimer = 90; // ~1.5 seconds for death animation
                    g_audio.playDeathAnimationSound();
                    return;
                }
            }
        }
    }

    // Main update tick
    void update_game_entities() {
        // Update floating score indicators
        for (auto it = floatingScores.begin(); it != floatingScores.end();) {
            it->y -= 0.5f;
            it->alpha -= 0.02f;
            if (it->alpha <= 0.0f) it = floatingScores.erase(it);
            else ++it;
        }

        if (fruitActive) {
            fruitTimer--;
            if (fruitTimer <= 0) fruitActive = false;
        }

        if (current_game_state == GameState::READY) {
            freezeTimer--;
            if (freezeTimer <= 0) {
                current_game_state = GameState::PLAYING;
                g_audio.setBackgroundSirenMode(1);
            }
            return;
        }

        if (current_game_state == GameState::GHOST_PAUSE) {
            freezeTimer--;
            if (freezeTimer <= 0) {
                current_game_state = GameState::PLAYING;
            }
            return;
        }

        if (current_game_state == GameState::DYING) {
            pacman.deathProgress += 0.012f;
            freezeTimer--;
            if (freezeTimer <= 0) {
                lives_left--;
                if (lives_left <= 0) {
                    current_game_state = GameState::GAME_OVER;
                    write_high_score_to_file();
                } else {
                    reset_all_characters_to_initial_positions();
                    current_game_state = GameState::READY;
                    freezeTimer = 120;
                }
            }
            return;
        }

        if (current_game_state == GameState::LEVEL_CLEAR) {
            freezeTimer--;
            if (freezeTimer % 20 == 0) {
                clearFlashWhite = !clearFlashWhite;
                clearFlashCount++;
            }
            if (freezeTimer <= 0) {
                current_level++;
                init_the_maze_array();
                reset_all_characters_to_initial_positions();
                current_game_state = GameState::READY;
                freezeTimer = 120;
            }
            return;
        }

        if (current_game_state != GameState::PLAYING) return;

        do_pacman_player_movement_logic();
        handleAllGhostEntitiesAI();
        check_if_pacman_is_touching_ghosts();
    }

    // ========================================================================
    // RENDERING PIPELINE
    // ========================================================================

    void render_the_maze_walls() {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                int cell = maze_grid[r][c];
                int px = c * TILE_SIZE;
                int py = HEADER_HEIGHT + r * TILE_SIZE;

                if (cell == 1) { // Wall
                    SDL_Color wallCol = clearFlashWhite ? SDL_Color{ 255, 255, 255, 255 } : SDL_Color{ 33, 33, 222, 255 };
                    SDL_SetRenderDrawColor(my_renderer, wallCol.r, wallCol.g, wallCol.b, 255);

                    SDL_Rect inner = { px + 2, py + 2, TILE_SIZE - 4, TILE_SIZE - 4 };
                    SDL_RenderDrawRect(my_renderer, &inner);

                    // Wall connections
                    if (c + 1 < COLS && maze_grid[r][c + 1] == 1) {
                        SDL_RenderDrawLine(my_renderer, px + TILE_SIZE - 2, py + 2, px + TILE_SIZE, py + 2);
                        SDL_RenderDrawLine(my_renderer, px + TILE_SIZE - 2, py + TILE_SIZE - 3, px + TILE_SIZE, py + TILE_SIZE - 3);
                    }
                    if (r + 1 < ROWS && maze_grid[r + 1][c] == 1) {
                        SDL_RenderDrawLine(my_renderer, px + 2, py + TILE_SIZE - 2, px + 2, py + TILE_SIZE);
                        SDL_RenderDrawLine(my_renderer, px + TILE_SIZE - 3, py + TILE_SIZE - 2, px + TILE_SIZE - 3, py + TILE_SIZE);
                    }
                } else if (cell == 4) { // Ghost door gate
                    SDL_SetRenderDrawColor(my_renderer, 244, 114, 182, 255);
                    SDL_Rect gate = { px, py + TILE_SIZE / 2 - 2, TILE_SIZE, 4 };
                    SDL_RenderFillRect(my_renderer, &gate);
                } else if (cell == 2) { // Dot pellet
                    draw_circle_filled_helper(my_renderer, px + TILE_SIZE / 2, py + TILE_SIZE / 2, 3, { 255, 184, 151, 255 });
                } else if (cell == 3) { // Energizer power pellet
                    draw_circle_filled_helper(my_renderer, px + TILE_SIZE / 2, py + TILE_SIZE / 2, 7, { 255, 184, 151, 255 });
                }
            }
        }
    }

    void render_fruit_item() {
        if (!fruitActive) return;
        int fx = static_cast<int>(13.5f * TILE_SIZE);
        int fy = HEADER_HEIGHT + static_cast<int>(17.5f * TILE_SIZE);
        SDL_Color fCol = STAGE_FRUITS[fruitIndex].color;

        // Fruit body
        draw_circle_filled_helper(my_renderer, fx, fy, 8, fCol);
        // Fruit stem
        SDL_SetRenderDrawColor(my_renderer, 34, 197, 94, 255);
        SDL_RenderDrawLine(my_renderer, fx, fy - 8, fx + 3, fy - 13);
    }

    void render_ghost_sprite(const GhostEntityClass& g) {
        if (current_game_state == GameState::DYING) return; // Ghosts vanish during death animation

        int gx = static_cast<int>(g.x);
        int gy = HEADER_HEIGHT + static_cast<int>(g.y);
        int r = 11;

        if (g.state == GhostState::EATEN) {
            // Only draw eyeballs returning to house
            render_ghost_eyeballs_only(gx, gy, g.dir);
            return;
        }

        SDL_Color bodyCol = g.color;
        if (g.state == GhostState::FRIGHTENED) {
            // Flash white when timer is running out (< 2 seconds)
            if (frightenedTimer < 120 && (frightenedTimer / 15) % 2 == 0) {
                bodyCol = { 255, 255, 255, 255 }; // Flash white
            } else {
                bodyCol = { 33, 33, 255, 255 }; // Scared blue
            }
        }

        // 1. Dome top
        draw_circle_filled_helper(my_renderer, gx, gy - 2, r, bodyCol);

        // 2. Skirt body
        SDL_SetRenderDrawColor(my_renderer, bodyCol.r, bodyCol.g, bodyCol.b, 255);
        SDL_Rect skirt = { gx - r, gy - 2, r * 2 + 1, r + 2 };
        SDL_RenderFillRect(my_renderer, &skirt);

        // 3. Wavy arcade feet
        for (int i = 0; i < 3; i++) {
            int fx = gx - r + 3 + i * 8;
            draw_circle_filled_helper(my_renderer, fx, gy + r, 3, bodyCol);
        }

        // 4. Eyes
        if (g.state == GhostState::FRIGHTENED) {
            // Frightened face: small dots & squiggly mouth
            draw_circle_filled_helper(my_renderer, gx - 4, gy - 2, 2, { 255, 184, 151, 255 });
            draw_circle_filled_helper(my_renderer, gx + 4, gy - 2, 2, { 255, 184, 151, 255 });
        } else {
            render_ghost_eyeballs_only(gx, gy, g.dir);
        }
    }

    void render_ghost_eyeballs_only(int gx, int gy, Direction dir) {
        int offX = 0, offY = 0;
        if (dir == Direction::LEFT) offX = -2;
        else if (dir == Direction::RIGHT) offX = 2;
        else if (dir == Direction::UP) offY = -2;
        else if (dir == Direction::DOWN) offY = 2;

        // White sclera
        draw_circle_filled_helper(my_renderer, gx - 4, gy - 3, 4, { 255, 255, 255, 255 });
        draw_circle_filled_helper(my_renderer, gx + 4, gy - 3, 4, { 255, 255, 255, 255 });

        // Blue pupil pointing in direction of movement
        draw_circle_filled_helper(my_renderer, gx - 4 + offX, gy - 3 + offY, 2, { 33, 33, 255, 255 });
        draw_circle_filled_helper(my_renderer, gx + 4 + offX, gy - 3 + offY, 2, { 33, 33, 255, 255 });
    }

    void render_user_interface_hud() {
        // High Score & Score Header
        render_arcade_text_helper(my_renderer, "1UP SCORE", 30, 12, 2, { 56, 189, 248, 255 });
        render_arcade_text_helper(my_renderer, to_string(current_score), 30, 32, 2, { 255, 255, 255, 255 });

        render_arcade_text_helper(my_renderer, "HIGH SCORE", WINDOW_WIDTH - 170, 12, 2, { 239, 68, 68, 255 });
        render_arcade_text_helper(my_renderer, to_string(max(current_score, high_score)), WINDOW_WIDTH - 170, 32, 2, { 255, 255, 255, 255 });

        // Bottom Footer: Lives Counter
        int footerY = WINDOW_HEIGHT - 32;
        render_arcade_text_helper(my_renderer, "LIVES:", 30, footerY, 2, { 148, 163, 184, 255 });
        for (int i = 0; i < lives_left - 1; i++) {
            draw_pacman_character_exact(my_renderer, 120 + i * 26, footerY + 6, 9.0f, 0.35f, Direction::RIGHT, { 250, 204, 21, 255 });
        }

        // Bottom Footer: Level Fruit Symbol
        string lvlStr = "LVL " + to_string(current_level);
        render_arcade_text_helper(my_renderer, lvlStr, WINDOW_WIDTH - 110, footerY, 2, { 34, 197, 94, 255 });

        // Start Screen / Game Over / Ready Banners
        if (current_game_state == GameState::START_SCREEN) {
            render_arcade_text_helper(my_renderer, "PAC-MAN", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 60, 4, { 250, 204, 21, 255 });
            render_arcade_text_helper(my_renderer, "PRESS SPACE OR ENTER", WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 + 10, 2, { 255, 255, 255, 255 });
            render_arcade_text_helper(my_renderer, "MD. ABU RISE ZUNAED", WINDOW_WIDTH / 2 - 110, WINDOW_HEIGHT / 2 + 50, 2, { 56, 189, 248, 255 });
        } else if (current_game_state == GameState::READY) {
            render_arcade_text_helper(my_renderer, "READY!", WINDOW_WIDTH / 2 - 36, HEADER_HEIGHT + 17 * TILE_SIZE + 6, 2, { 250, 204, 21, 255 });
        } else if (current_game_state == GameState::PAUSED) {
            render_arcade_text_helper(my_renderer, "GAME PAUSED", WINDOW_WIDTH / 2 - 65, HEADER_HEIGHT + 17 * TILE_SIZE + 6, 2, { 56, 189, 248, 255 });
        } else if (current_game_state == GameState::GAME_OVER) {
            render_arcade_text_helper(my_renderer, "GAME  OVER", WINDOW_WIDTH / 2 - 60, HEADER_HEIGHT + 17 * TILE_SIZE + 6, 2, { 239, 68, 68, 255 });
        }

        // Render Floating Scores (+200, +400, fruit bonus, etc.)
        for (const auto& fs : floatingScores) {
            SDL_Color fsCol = { 56, 189, 248, static_cast<Uint8>(fs.alpha * 255) };
            render_arcade_text_helper(my_renderer, fs.text, static_cast<int>(fs.x) - 12, HEADER_HEIGHT + static_cast<int>(fs.y) - 6, 2, fsCol);
        }
    }

    void draw_entire_game_frame() {
        // Deep arcade background
        SDL_SetRenderDrawColor(my_renderer, 5, 8, 20, 255);
        SDL_RenderClear(my_renderer);

        render_the_maze_walls();
        render_fruit_item();

        // Draw Pac-Man
        if (current_game_state != GameState::START_SCREEN) {
            draw_pacman_character_exact(
                my_renderer,
                pacman.x,
                HEADER_HEIGHT + pacman.y,
                11.0f,
                pacman.mouthTan,
                pacman.dir,
                { 250, 204, 21, 255 },
                pacman.deathProgress
            );
        }

        // Draw Ghosts
        if (current_game_state != GameState::START_SCREEN) {
            for (const auto& g : ghosts) {
                render_ghost_sprite(g);
            }
        }

        render_user_interface_hud();
        SDL_RenderPresent(my_renderer);
    }

    // Main 60 FPS Game Loop
    void run_main_game_loop() {
        bool running = true;
        const auto timeStep = chrono::microseconds(16666); // Solid 60 FPS (~16.666 ms)
        auto lastTime = chrono::high_resolution_clock::now();
        auto accumulator = chrono::microseconds(0);

        while (running) {
            auto currentTime = chrono::high_resolution_clock::now();
            auto frameDuration = chrono::duration_cast<chrono::microseconds>(currentTime - lastTime);
            lastTime = currentTime;

            if (frameDuration > chrono::milliseconds(100)) {
                frameDuration = chrono::milliseconds(100); // Prevent spiral of death
            }

            accumulator += frameDuration;
            process_player_keyboard_events(running);

            while (accumulator >= timeStep) {
                update_game_entities();
                accumulator -= timeStep;
            }

            draw_entire_game_frame();
            SDL_Delay(1); // Yield execution time to prevent 100% CPU core usage
        }
    }
};

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int main(int argc, char* argv[]) {
    // Suppress unused parameter warnings
    (void)argc;
    (void)argv;

    srand(static_cast<unsigned int>(time(nullptr)));

    PacmanArcadeGame game;
    if (!game.initialize_sdl_window_and_renderer()) {
        cerr << "Failed to start Pac-Man arcade engine!" << endl;
        return 1;
    }

    // Launch main loop
    game.run_main_game_loop();

    return 0;
}
