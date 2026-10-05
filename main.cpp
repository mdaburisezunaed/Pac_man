#if defined(__has_include)
  #if __has_include(<SDL2/SDL.h>)
    #include <SDL2/SDL.h>
  #elif __has_include(<SDL.h>)
    #include <SDL.h>
  #endif
#else
  #include <SDL.h>
#endif
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

// ============================================================================
// PAC-MAN ARCADE (C++17 / SDL2 Hardware-Accelerated Master Edition)
// - Polyphonic Audio Synthesizer with Iconic Intro Theme, Siren & Sound FX
// - 100% Mathematically Correct Directional Pac-Man Mouth Animation
// - Authentic Grid-Aligned Ghost AI (Blinky, Pinky, Inky & Clyde Personalities)
// - High-Resolution Large Map (672x850 Window, 24px HD Tiles)
// - Smooth Corner Pre-Buffering & Sub-Pixel Continuous Motion
// - Built-in 8-bit Arcade Typography (Zero External Font Dependencies)
// ============================================================================

const int COLS = 28;
const int ROWS = 31;
const int TILE_SIZE = 24; // 28 * 24 = 672px
const int HEADER_HEIGHT = 60;
const int FOOTER_HEIGHT = 46;
const int WINDOW_WIDTH = COLS * TILE_SIZE; // 672
const int WINDOW_HEIGHT = HEADER_HEIGHT + (ROWS * TILE_SIZE) + FOOTER_HEIGHT; // 850

// Classic 28x31 Maze Layout:
// 1 = Wall, 2 = Dot, 3 = Energizer, 0 = Empty, 4 = Gate, 5 = House, 6 = Tunnel
const std::vector<std::string> RAW_MAZE = {
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

enum class Direction { NONE, UP, DOWN, LEFT, RIGHT };
enum class GhostState { IN_HOUSE, LEAVING_HOUSE, CHASE, SCATTER, FRIGHTENED, EATEN };
enum class GameState { START_SCREEN, READY, PLAYING, DYING, GHOST_PAUSE, LEVEL_CLEAR, GAME_OVER, PAUSED };

struct Point { int x; int y; };

struct FruitInfo {
    std::string name;
    int points;
    SDL_Color color;
};

const std::vector<FruitInfo> STAGE_FRUITS = {
    { "Cherry", 100, { 239, 68, 68, 255 } },
    { "Strawberry", 300, { 244, 63, 94, 255 } },
    { "Peach", 500, { 251, 146, 60, 255 } },
    { "Apple", 700, { 34, 197, 94, 255 } },
    { "Grapes", 1000, { 168, 85, 247, 255 } },
    { "Galaxian", 2000, { 56, 189, 248, 255 } },
    { "Bell", 3000, { 234, 179, 8, 255 } },
    { "Key", 5000, { 6, 182, 212, 255 } }
};

struct FloatingScore {
    float x, y;
    std::string text;
    float alpha;
};

// ============================================================================
// BUILT-IN BITMAP 5x7 FONT
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

void drawArcadeText(SDL_Renderer* ren, const std::string& text, int startX, int startY, int scale, SDL_Color col) {
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    int curX = startX;

    for (char c : text) {
        char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
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
// POLYPHONIC RETRO AUDIO ENGINE (SDL Audio Callback)
// ============================================================================
struct Note {
    double freq;
    double duration;
};

// Authentic opening melody notes:
// B4, B5, F#5, D#5, B5, F#5, D#5, C5, C6, G5, E5, C6, G5, E5 ...
const std::vector<Note> INTRO_MELODY = {
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

struct PolyAudioSynth {
    bool muted = false;

    // Music Channel (Melody & Ambient Siren)
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

    void playIntro() {
        SDL_LockAudio();
        isPlayingIntro = true;
        introNoteIdx = 0;
        introNoteTime = 0.0;
        musicPhase = 0.0;
        musicActive = !muted;
        sirenMode = 0;
        SDL_UnlockAudio();
    }

    void stopIntro() {
        SDL_LockAudio();
        isPlayingIntro = false;
        SDL_UnlockAudio();
    }

    void setSiren(int mode) {
        SDL_LockAudio();
        sirenMode = mode;
        if (!isPlayingIntro) {
            musicActive = (mode > 0 && !muted);
        }
        SDL_UnlockAudio();
    }

    void toggleMute() {
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

    void playChomp(bool high) {
        if (muted) return;
        SDL_LockAudio();
        sfxPhase = 0.0;
        sfxStartFreq = high ? 490.0 : 370.0;
        sfxEndFreq = high ? 240.0 : 180.0;
        sfxDuration = 0.09;
        sfxTime = 0.0;
        sfxVolume = 0.22;
        sfxWaveType = 1; // Triangle
        sfxActive = true;
        SDL_UnlockAudio();
    }

    void playEatGhost() {
        if (muted) return;
        SDL_LockAudio();
        sfxPhase = 0.0;
        sfxStartFreq = 320.0;
        sfxEndFreq = 960.0;
        sfxDuration = 0.35;
        sfxTime = 0.0;
        sfxVolume = 0.35;
        sfxWaveType = 2; // Square
        sfxActive = true;
        SDL_UnlockAudio();
    }

    void playFruit() {
        if (muted) return;
        SDL_LockAudio();
        sfxPhase = 0.0;
        sfxStartFreq = 587.0;
        sfxEndFreq = 1175.0;
        sfxDuration = 0.28;
        sfxTime = 0.0;
        sfxVolume = 0.25;
        sfxWaveType = 1;
        sfxActive = true;
        SDL_UnlockAudio();
    }

    void playDeath() {
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
        sfxWaveType = 1;
        sfxActive = true;
        SDL_UnlockAudio();
    }
};

static PolyAudioSynth g_audio;

void audioCallback(void*, Uint8* stream, int len) {
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

                double lfo = std::sin(g_audio.sirenPhase);
                double baseFreq = 250.0;
                double sweep = 50.0;
                if (g_audio.sirenMode == 2) { baseFreq = 220.0; sweep = 70.0; } // Frightened
                if (g_audio.sirenMode == 3) { baseFreq = 540.0; sweep = 180.0; } // Eyes

                double sirenFreq = baseFreq + lfo * sweep;
                g_audio.musicPhase += (2.0 * M_PI * sirenFreq) / sampleRate;
                if (g_audio.musicPhase > 2.0 * M_PI) g_audio.musicPhase -= 2.0 * M_PI;

                musicSample = std::sin(g_audio.musicPhase) * (g_audio.sirenMode == 3 ? 0.09 : 0.06);
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
                    wave = std::sin(g_audio.sfxPhase);
                }

                double vol = g_audio.sfxVolume * (1.0 - progress * 0.4);
                sfxSample = wave * vol;
                g_audio.sfxTime += 1.0 / sampleRate;
            } else {
                g_audio.sfxActive = false;
            }
        }

        double mixed = musicSample + sfxSample;
        mixed = std::clamp(mixed, -0.95, 0.95);
        buffer[i] = static_cast<Sint16>(mixed * 32000.0);
    }
}

// ============================================================================
// GEOMETRY & DIRECTIONAL PAC-MAN RENDERING
// ============================================================================
void drawFilledCircle(SDL_Renderer* ren, int cx, int cy, int radius, SDL_Color col) {
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = static_cast<int>(std::round(std::sqrt(radius * radius - dy * dy)));
        SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

// 100% mathematically correct directional mouth wedge cutout & 360-degree death dissolution
void drawPacmanExact(SDL_Renderer* ren, float cx, float cy, float radius, float mouthTan, Direction dir, SDL_Color col, float deathProgress = 0.0f) {
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
                    float angle = std::atan2(ly, lx);
                    if (std::abs(angle) < deathProgress * static_cast<float>(M_PI)) {
                        continue; // Full 360-degree dissolution
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
                    if (lx > 0 && std::abs(ly) < lx * mouthTan) {
                        continue; // Cutout!
                    }
                }
                SDL_RenderDrawPoint(ren, static_cast<int>(cx + dx), static_cast<int>(cy + dy));
            }
        }
    }
}

// ============================================================================
// GAME ENTITIES
// ============================================================================
class PacmanEntity {
public:
    float x = 13.5f * TILE_SIZE;
    float y = 23.0f * TILE_SIZE + TILE_SIZE / 2.0f;
    Direction dir = Direction::LEFT;
    Direction nextDir = Direction::LEFT;
    float speed = 2.4f;

    // Smooth mouth chomping animation
    float mouthTan = 0.35f;
    float mouthSpeed = 0.035f;
    bool mouthClosing = false;
    float deathProgress = 0.0f;

    void reset() {
        x = 13.5f * TILE_SIZE;
        y = 23.0f * TILE_SIZE + TILE_SIZE / 2.0f;
        dir = Direction::LEFT;
        nextDir = Direction::LEFT;
        mouthTan = 0.35f;
        mouthClosing = false;
        deathProgress = 0.0f;
    }
};

class GhostEntity {
public:
    std::string name;
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

    GhostEntity(std::string n, SDL_Color col, float sx, float sy, Point scTarget, GhostState st)
        : name(n), color(col), x(sx * TILE_SIZE), y(sy * TILE_SIZE + TILE_SIZE / 2.0f),
          startX(sx * TILE_SIZE), startY(sy * TILE_SIZE + TILE_SIZE / 2.0f),
          scatterTarget(scTarget), state(st) {}

    void reset(GhostState st) {
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
// PAC-MAN ENGINE
// ============================================================================
class PacmanArcade {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    std::vector<std::vector<int>> maze;
    PacmanEntity pacman;
    std::vector<GhostEntity> ghosts;
    std::vector<FloatingScore> floatingScores;

    GameState state = GameState::START_SCREEN;
    int score = 0;
    int highScore = 0;
    int lives = 3;
    int level = 1;
    int dotsLeft = 0;
    int totalDots = 0;

    int frightenedTimer = 0;
    int ghostsEatenMultiplier = 1;
    int modeTimer = 0;
    GhostState globalGhostMode = GhostState::SCATTER;

    // Fruit
    bool fruitActive = false;
    int fruitTimer = 0;
    int fruitIndex = 0;

    bool chompHigh = false;

    // Level Clear flash
    int clearFlashCount = 0;
    bool clearFlashWhite = false;
    int freezeTimer = 0;

public:
    PacmanArcade() {
        loadHighScore();
        initMaze();

        ghosts.push_back(GhostEntity("Blinky", { 239, 68, 68, 255 }, 13.5f, 11.0f, { 27, 0 }, GhostState::SCATTER));
        ghosts.push_back(GhostEntity("Pinky",  { 244, 114, 182, 255 }, 13.5f, 14.0f, { 2, 0 }, GhostState::IN_HOUSE));
        ghosts.push_back(GhostEntity("Inky",   { 6, 182, 212, 255 }, 11.5f, 14.0f, { 27, 31 }, GhostState::IN_HOUSE));
        ghosts.push_back(GhostEntity("Clyde",  { 249, 115, 22, 255 }, 15.5f, 14.0f, { 0, 31 }, GhostState::IN_HOUSE));
    }

    ~PacmanArcade() {
        saveHighScore();
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_CloseAudio();
        SDL_Quit();
    }

    void loadHighScore() {
        std::ifstream file("highscore.dat");
        if (file.is_open()) {
            if (!(file >> highScore) || highScore < 0) {
                highScore = 0;
            }
            file.close();
        }
    }

    void saveHighScore() {
        if (score > highScore) highScore = score;
        std::ofstream file("highscore.dat");
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }

    void initMaze() {
        maze.clear();
        dotsLeft = 0;
        for (int r = 0; r < ROWS; r++) {
            std::vector<int> row;
            for (int c = 0; c < COLS; c++) {
                int val = RAW_MAZE[r][c] - '0';
                row.push_back(val);
                if (val == 2 || val == 3) dotsLeft++;
            }
            maze.push_back(row);
        }
        totalDots = dotsLeft;
    }

    void resetPositions() {
        pacman.reset();
        ghosts[0].reset(GhostState::SCATTER);
        ghosts[0].dir = Direction::LEFT;
        ghosts[1].reset(GhostState::IN_HOUSE);
        ghosts[2].reset(GhostState::IN_HOUSE);
        ghosts[3].reset(GhostState::IN_HOUSE);
        frightenedTimer = 0;
        modeTimer = 0;
        globalGhostMode = GhostState::SCATTER;
        fruitActive = false;
        clearFlashWhite = false;
        clearFlashCount = 0;
        g_audio.setSiren(0);
    }

    void startNewGame() {
        score = 0;
        lives = 3;
        level = 1;
        initMaze();
        resetPositions();
        state = GameState::READY;
        freezeTimer = 240; // ~4 seconds for full intro melody
        g_audio.playIntro();
    }

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow(
            "PAC-MAN Arcade Edition (C++17)",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN
        );
        if (!window) return false;

        renderer = SDL_CreateRenderer(
            window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );
        if (!renderer) return false;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // Initialize Audio
        SDL_AudioSpec wanted;
        SDL_zero(wanted);
        wanted.freq = 44100;
        wanted.format = AUDIO_S16SYS;
        wanted.channels = 1;
        wanted.samples = 1024;
        wanted.callback = audioCallback;

        if (SDL_OpenAudio(&wanted, nullptr) == 0) {
            SDL_PauseAudio(0);
        }

        return true;
    }

    bool isTilePassable(int tileX, int tileY, Direction d, bool isGhost = false, GhostState gst = GhostState::CHASE) {
        int nx = tileX;
        int ny = tileY;
        if (d == Direction::UP) ny--;
        else if (d == Direction::DOWN) ny++;
        else if (d == Direction::LEFT) nx--;
        else if (d == Direction::RIGHT) nx++;

        // Wrap tunnel
        if (ny == 14 && (nx < 0 || nx >= COLS)) return true;
        if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) return false;

        int cell = maze[ny][nx];
        if (cell == 1) return false; // Wall
        if (cell == 4) return isGhost && (gst == GhostState::LEAVING_HOUSE || gst == GhostState::EATEN);
        if (cell == 5) return isGhost && (gst == GhostState::IN_HOUSE || gst == GhostState::LEAVING_HOUSE || gst == GhostState::EATEN);
        return true;
    }

    void handleInput(bool& running) {
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
                        if (state == GameState::PLAYING) {
                            state = GameState::PAUSED;
                            g_audio.setSiren(0);
                        } else if (state == GameState::PAUSED) {
                            state = GameState::PLAYING;
                            g_audio.setSiren(frightenedTimer > 0 ? 2 : 1);
                        }
                        break;
                    case SDLK_m:
                        g_audio.toggleMute();
                        break;
                    case SDLK_SPACE:
                    case SDLK_RETURN:
                        if (state == GameState::START_SCREEN || state == GameState::GAME_OVER) {
                            startNewGame();
                        } else if (state == GameState::PAUSED) {
                            state = GameState::PLAYING;
                            g_audio.setSiren(frightenedTimer > 0 ? 2 : 1);
                        }
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                }
            }
        }
    }

    void updatePacman() {
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
        float distToCenter = std::hypot(pacman.x - centerX, pacman.y - centerY);
        if (pacman.nextDir != pacman.dir && distToCenter < 6.0f) {
            if (isTilePassable(tileX, tileY, pacman.nextDir)) {
                pacman.x = centerX;
                pacman.y = centerY;
                pacman.dir = pacman.nextDir;
            }
        }

        // Check if next move in current direction is blocked
        bool blocked = false;
        if (distToCenter < pacman.speed) {
            if (!isTilePassable(tileX, tileY, pacman.dir)) {
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
        int curTileX = static_cast<int>(std::floor(pacman.x / TILE_SIZE));
        int curTileY = static_cast<int>(std::floor(pacman.y / TILE_SIZE));

        if (curTileX >= 0 && curTileX < COLS && curTileY >= 0 && curTileY < ROWS) {
            int& cell = maze[curTileY][curTileX];
            if (cell == 2) { // Dot
                cell = 0;
                dotsLeft--;
                score += 10;
                chompHigh = !chompHigh;
                g_audio.playChomp(chompHigh);
                checkFruitSpawn();
                checkLevelWin();
            } else if (cell == 3) { // Energizer
                cell = 0;
                dotsLeft--;
                score += 50;
                frightenedTimer = 480; // 8 seconds
                ghostsEatenMultiplier = 1;
                g_audio.playChomp(true);
                g_audio.setSiren(2); // Frightened siren

                for (auto& g : ghosts) {
                    if (g.state != GhostState::EATEN && g.state != GhostState::IN_HOUSE && g.state != GhostState::LEAVING_HOUSE) {
                        g.state = GhostState::FRIGHTENED;
                    }
                }
                checkFruitSpawn();
                checkLevelWin();
            }
        }

        // Eat fruit
        if (fruitActive) {
            float dist = std::hypot(pacman.x - (13.5f * TILE_SIZE), pacman.y - (17.5f * TILE_SIZE));
            if (dist < TILE_SIZE * 0.8f) {
                fruitActive = false;
                int pts = STAGE_FRUITS[fruitIndex].points;
                score += pts;
                floatingScores.push_back({ 13.5f * TILE_SIZE, 17.5f * TILE_SIZE, "+" + std::to_string(pts), 1.0f });
                g_audio.playFruit();
            }
        }

        if (score > highScore) highScore = score;
    }

    void checkFruitSpawn() {
        int eaten = totalDots - dotsLeft;
        if ((eaten == 70 || eaten == 170) && !fruitActive) {
            fruitActive = true;
            fruitTimer = 600; // 10s
            fruitIndex = std::min(level - 1, static_cast<int>(STAGE_FRUITS.size()) - 1);
        }
    }

    void checkLevelWin() {
        if (dotsLeft <= 0) {
            state = GameState::LEVEL_CLEAR;
            clearFlashCount = 0;
            clearFlashWhite = false;
            freezeTimer = 20;
            g_audio.setSiren(0);
        }
    }

    Point getGhostTarget(const GhostEntity& g) {
        int pacTileX = static_cast<int>(pacman.x / TILE_SIZE);
        int pacTileY = static_cast<int>(pacman.y / TILE_SIZE);

        if (g.state == GhostState::EATEN) return { 13, 11 };
        if (g.state == GhostState::SCATTER) return g.scatterTarget;

        if (g.state == GhostState::CHASE) {
            if (g.name == "Blinky") return { pacTileX, pacTileY };
            if (g.name == "Pinky") {
                int dx = (pacman.dir == Direction::RIGHT ? 4 : (pacman.dir == Direction::LEFT ? -4 : 0));
                int dy = (pacman.dir == Direction::DOWN ? 4 : (pacman.dir == Direction::UP ? -4 : 0));
                return { pacTileX + dx, pacTileY + dy };
            }
            if (g.name == "Inky") {
                const auto& b = ghosts[0];
                int bTileX = static_cast<int>(b.x / TILE_SIZE);
                int bTileY = static_cast<int>(b.y / TILE_SIZE);
                int pX = pacTileX + (pacman.dir == Direction::RIGHT ? 2 : (pacman.dir == Direction::LEFT ? -2 : 0));
                int pY = pacTileY + (pacman.dir == Direction::DOWN ? 2 : (pacman.dir == Direction::UP ? -2 : 0));
                return { pX + (pX - bTileX), pY + (pY - bTileY) };
            }
            if (g.name == "Clyde") {
                int myTileX = static_cast<int>(g.x / TILE_SIZE);
                int myTileY = static_cast<int>(g.y / TILE_SIZE);
                if (std::hypot(myTileX - pacTileX, myTileY - pacTileY) > 8.0f) {
                    return { pacTileX, pacTileY };
                } else {
                    return g.scatterTarget;
                }
            }
        }
        return { pacTileX, pacTileY };
    }

    Direction getReverse(Direction d) {
        if (d == Direction::UP) return Direction::DOWN;
        if (d == Direction::DOWN) return Direction::UP;
        if (d == Direction::LEFT) return Direction::RIGHT;
        if (d == Direction::RIGHT) return Direction::LEFT;
        return Direction::NONE;
    }

    void updateGhosts() {
        // Mode wave cycles
        if (frightenedTimer > 0) {
            frightenedTimer--;
            if (frightenedTimer == 0) {
                g_audio.setSiren(1);
                for (auto& g : ghosts) {
                    if (g.state == GhostState::FRIGHTENED) g.state = globalGhostMode;
                }
            }
        } else {
            modeTimer++;
            if (modeTimer < 420) globalGhostMode = GhostState::SCATTER;
            else if (modeTimer < 1620) globalGhostMode = GhostState::CHASE;
            else if (modeTimer < 2040) globalGhostMode = GhostState::SCATTER;
            else globalGhostMode = GhostState::CHASE;

            for (auto& g : ghosts) {
                if (g.state == GhostState::SCATTER || g.state == GhostState::CHASE) {
                    g.state = globalGhostMode;
                }
            }
        }

        // Release ghosts from house
        int dotsEaten = totalDots - dotsLeft;
        if (ghosts[1].state == GhostState::IN_HOUSE) ghosts[1].state = GhostState::LEAVING_HOUSE;
        if (dotsEaten >= 30 && ghosts[2].state == GhostState::IN_HOUSE) ghosts[2].state = GhostState::LEAVING_HOUSE;
        if (dotsEaten >= 60 && ghosts[3].state == GhostState::IN_HOUSE) ghosts[3].state = GhostState::LEAVING_HOUSE;

        // Check siren state priorities: Eyes (3) > Frightened (2) > Normal (1)
        bool hasEyes = false;
        for (const auto& g : ghosts) {
            if (g.state == GhostState::EATEN) hasEyes = true;
        }

        if (state == GameState::PLAYING) {
            if (hasEyes) {
                g_audio.setSiren(3);
            } else if (frightenedTimer > 0) {
                g_audio.setSiren(2);
            } else {
                g_audio.setSiren(1);
            }
        }

        for (auto& g : ghosts) {
            float spd = g.normalSpeed;
            int tileX = static_cast<int>(g.x / TILE_SIZE);
            int tileY = static_cast<int>(g.y / TILE_SIZE);

            // Tunnel slow-down (row 14)
            if (tileY == 14 && (tileX < 6 || tileX > 21)) spd = 1.1f;
            if (g.state == GhostState::FRIGHTENED) spd = g.frightSpeed;
            if (g.state == GhostState::EATEN) spd = g.eatenSpeed;

            // 1. In House Bobbing
            if (g.state == GhostState::IN_HOUSE) {
                g.y += g.bounceDir * 0.6f;
                if (g.y <= 13.8f * TILE_SIZE) g.bounceDir = 1;
                if (g.y >= 14.5f * TILE_SIZE) g.bounceDir = -1;
                continue;
            }

            // 2. Leaving House Sequence
            if (g.state == GhostState::LEAVING_HOUSE) {
                float targetX = 13.5f * TILE_SIZE;
                float targetY = 11.0f * TILE_SIZE + TILE_SIZE / 2.0f;

                if (std::abs(g.x - targetX) > 1.0f) {
                    g.x += (targetX > g.x ? 1.2f : -1.2f);
                } else {
                    g.x = targetX;
                    g.y -= 1.4f;
                    if (g.y <= targetY) {
                        g.y = targetY;
                        g.state = globalGhostMode; // Emerges in active wave mode, not frightened
                        g.dir = Direction::LEFT;
                        g.lastTileX = -1;
                        g.lastTileY = -1;
                    }
                }
                continue;
            }

            // 3. Eaten Eyes Reaching Door
            if (g.state == GhostState::EATEN) {
                float doorX = 13.5f * TILE_SIZE;
                float doorY = 11.0f * TILE_SIZE + TILE_SIZE / 2.0f;
                if (std::hypot(g.x - doorX, g.y - doorY) < 6.0f) {
                    g.x = doorX;
                    g.y = 14.0f * TILE_SIZE;
                    g.state = GhostState::LEAVING_HOUSE;
                    continue;
                }
            }

            // 4. Normal Grid Navigation at Intersections
            float centerX = tileX * TILE_SIZE + TILE_SIZE / 2.0f;
            float centerY = tileY * TILE_SIZE + TILE_SIZE / 2.0f;
            float distToCenter = std::hypot(g.x - centerX, g.y - centerY);

            if (distToCenter < spd && (tileX != g.lastTileX || tileY != g.lastTileY)) {
                g.x = centerX;
                g.y = centerY;
                g.lastTileX = tileX;
                g.lastTileY = tileY;

                Direction rev = getReverse(g.dir);
                std::vector<Direction> choices;
                // Arcade tiebreaker order: UP, LEFT, DOWN, RIGHT
                std::vector<Direction> order = { Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT };

                for (auto d : order) {
                    if (d == rev) continue; // No 180 turnaround
                    if (isTilePassable(tileX, tileY, d, true, g.state)) {
                        choices.push_back(d);
                    }
                }

                // If dead-end, allow reverse
                if (choices.empty()) {
                    choices.push_back(rev);
                }

                if (g.state == GhostState::FRIGHTENED) {
                    g.dir = choices[rand() % choices.size()];
                } else {
                    Point target = getGhostTarget(g);
                    Direction bestDir = choices[0];
                    float bestDist = 1e9f;

                    for (auto d : choices) {
                        int nx = tileX + (d == Direction::RIGHT ? 1 : (d == Direction::LEFT ? -1 : 0));
                        int ny = tileY + (d == Direction::DOWN ? 1 : (d == Direction::UP ? -1 : 0));
                        float dist = std::hypot(nx - target.x, ny - target.y);
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestDir = d;
                        }
                    }
                    g.dir = bestDir;
                }
            }

            // Move ghost
            if (g.dir == Direction::LEFT) {
                g.y = centerY;
                g.x -= spd;
            } else if (g.dir == Direction::RIGHT) {
                g.y = centerY;
                g.x += spd;
            } else if (g.dir == Direction::UP) {
                g.x = centerX;
                g.y -= spd;
            } else if (g.dir == Direction::DOWN) {
                g.x = centerX;
                g.y += spd;
            }

            // Wrap tunnel
            if (tileY == 14) {
                if (g.x < -TILE_SIZE / 2.0f) g.x = WINDOW_WIDTH + TILE_SIZE / 2.0f;
                else if (g.x > WINDOW_WIDTH + TILE_SIZE / 2.0f) g.x = -TILE_SIZE / 2.0f;
            }
        }
    }

    void checkCollisions() {
        for (auto& g : ghosts) {
            float dist = std::hypot(pacman.x - g.x, pacman.y - g.y);
            if (dist < TILE_SIZE * 0.75f) {
                if (g.state == GhostState::FRIGHTENED) {
                    g.state = GhostState::EATEN;
                    int pts = std::min(1600, 200 * ghostsEatenMultiplier);
                    ghostsEatenMultiplier *= 2;
                    score += pts;
                    floatingScores.push_back({ g.x, g.y, "+" + std::to_string(pts), 1.0f });
                    g_audio.playEatGhost();

                    state = GameState::GHOST_PAUSE;
                    freezeTimer = 25; // 0.4s freeze frame
                    break;
                } else if (g.state == GhostState::CHASE || g.state == GhostState::SCATTER || g.state == GhostState::LEAVING_HOUSE) {
                    state = GameState::DYING;
                    g_audio.playDeath();
                    pacman.deathProgress = 0.0f;
                    break;
                }
            }
        }
    }

    void update() {
        if (state == GameState::GHOST_PAUSE) {
            freezeTimer--;
            if (freezeTimer <= 0) state = GameState::PLAYING;
            return;
        }

        if (state == GameState::LEVEL_CLEAR) {
            freezeTimer--;
            if (freezeTimer <= 0) {
                clearFlashWhite = !clearFlashWhite;
                clearFlashCount++;
                freezeTimer = 10;
                if (clearFlashCount >= 8) {
                    clearFlashWhite = false;
                    clearFlashCount = 0;
                    level++;
                    initMaze();
                    resetPositions();
                    state = GameState::READY;
                    freezeTimer = 180;
                    g_audio.playIntro();
                }
            }
            return;
        }

        if (state == GameState::READY) {
            freezeTimer--;
            if (freezeTimer <= 0) {
                state = GameState::PLAYING;
                g_audio.setSiren(1);
            }
            return;
        }

        if (state == GameState::DYING) {
            pacman.deathProgress += 0.025f;
            if (pacman.deathProgress >= 1.0f) {
                lives--;
                if (lives <= 0) {
                    state = GameState::GAME_OVER;
                    saveHighScore();
                } else {
                    resetPositions();
                    state = GameState::READY;
                    freezeTimer = 120;
                    g_audio.playIntro();
                }
            }
            return;
        }

        if (state != GameState::PLAYING) return;

        updatePacman();
        updateGhosts();
        checkCollisions();

        if (fruitActive) {
            fruitTimer--;
            if (fruitTimer <= 0) fruitActive = false;
        }

        for (int i = static_cast<int>(floatingScores.size()) - 1; i >= 0; i--) {
            floatingScores[i].y -= 0.6f;
            floatingScores[i].alpha -= 0.02f;
            if (floatingScores[i].alpha <= 0.0f) {
                floatingScores.erase(floatingScores.begin() + i);
            }
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 5, 8, 20, 255);
        SDL_RenderClear(renderer);

        // Header
        drawArcadeText(renderer, "1UP SCORE", 24, 14, 2, { 56, 189, 248, 255 });
        drawArcadeText(renderer, std::to_string(score), 24, 34, 3, { 255, 255, 255, 255 });

        drawArcadeText(renderer, "HIGH SCORE", WINDOW_WIDTH - 190, 14, 2, { 239, 68, 68, 255 });
        drawArcadeText(renderer, std::to_string(highScore), WINDOW_WIDTH - 190, 34, 3, { 255, 255, 255, 255 });

        // Maze
        int offsetY = HEADER_HEIGHT;
        SDL_Color wallBorderColor = clearFlashWhite ? SDL_Color{ 255, 255, 255, 255 } : SDL_Color{ 37, 99, 235, 255 };
        SDL_Color wallFillColor = clearFlashWhite ? SDL_Color{ 240, 240, 255, 255 } : SDL_Color{ 10, 17, 40, 255 };

        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                int cell = maze[r][c];
                int x = c * TILE_SIZE;
                int y = offsetY + r * TILE_SIZE;

                if (cell == 1) {
                    SDL_Rect fillRect = { x + 2, y + 2, TILE_SIZE - 4, TILE_SIZE - 4 };
                    SDL_SetRenderDrawColor(renderer, wallFillColor.r, wallFillColor.g, wallFillColor.b, 255);
                    SDL_RenderFillRect(renderer, &fillRect);

                    SDL_Rect outlineRect = { x + 1, y + 1, TILE_SIZE - 2, TILE_SIZE - 2 };
                    SDL_SetRenderDrawColor(renderer, wallBorderColor.r, wallBorderColor.g, wallBorderColor.b, 255);
                    SDL_RenderDrawRect(renderer, &outlineRect);
                } else if (cell == 4) {
                    SDL_SetRenderDrawColor(renderer, 251, 113, 133, 255);
                    SDL_Rect gate = { x, y + TILE_SIZE / 2 - 2, TILE_SIZE, 4 };
                    SDL_RenderFillRect(renderer, &gate);
                } else if (cell == 2) {
                    drawFilledCircle(renderer, x + TILE_SIZE / 2, y + TILE_SIZE / 2, 3, { 254, 215, 170, 255 });
                } else if (cell == 3) {
                    float pulse = (std::sin(SDL_GetTicks() * 0.008f) + 1.0f) * 0.5f;
                    int rad = static_cast<int>(5.0f + pulse * 3.0f);
                    drawFilledCircle(renderer, x + TILE_SIZE / 2, y + TILE_SIZE / 2, rad, { 254, 240, 138, 255 });
                }
            }
        }

        // Draw Fruit
        if (fruitActive) {
            float fx = 13.5f * TILE_SIZE;
            float fy = offsetY + 17.5f * TILE_SIZE;
            drawFilledCircle(renderer, static_cast<int>(fx), static_cast<int>(fy), 9, STAGE_FRUITS[fruitIndex].color);
            drawArcadeText(renderer, STAGE_FRUITS[fruitIndex].name.substr(0, 2), static_cast<int>(fx - 8), static_cast<int>(fy - 5), 1, { 255, 255, 255, 255 });
        }

        // Draw Pac-Man
        if (state != GameState::LEVEL_CLEAR) {
            float px = pacman.x;
            float py = offsetY + pacman.y;
            float rad = TILE_SIZE * 0.46f;

            if (state == GameState::DYING) {
                drawPacmanExact(renderer, px, py, rad, 0.0f, pacman.dir, { 250, 204, 21, 255 }, pacman.deathProgress);
            } else {
                drawPacmanExact(renderer, px, py, rad, pacman.mouthTan, pacman.dir, { 250, 204, 21, 255 });
            }
        }

        // Draw Ghosts
        if (state != GameState::DYING && state != GameState::LEVEL_CLEAR) {
            for (const auto& g : ghosts) {
                float gx = g.x;
                float gy = offsetY + g.y;
                float rad = TILE_SIZE * 0.46f;

                if (g.state == GhostState::EATEN) {
                    // Floating Eyes
                    int eyeOffX = (g.dir == Direction::RIGHT ? 2 : (g.dir == Direction::LEFT ? -2 : 0));
                    int eyeOffY = (g.dir == Direction::DOWN ? 2 : (g.dir == Direction::UP ? -2 : 0));

                    drawFilledCircle(renderer, static_cast<int>(gx - 4), static_cast<int>(gy - 2), 4, { 255, 255, 255, 255 });
                    drawFilledCircle(renderer, static_cast<int>(gx + 4), static_cast<int>(gy - 2), 4, { 255, 255, 255, 255 });
                    drawFilledCircle(renderer, static_cast<int>(gx - 4 + eyeOffX), static_cast<int>(gy - 2 + eyeOffY), 2, { 30, 58, 138, 255 });
                    drawFilledCircle(renderer, static_cast<int>(gx + 4 + eyeOffX), static_cast<int>(gy - 2 + eyeOffY), 2, { 30, 58, 138, 255 });
                    continue;
                }

                SDL_Color gColor = g.color;
                if (g.state == GhostState::FRIGHTENED) {
                    if (frightenedTimer < 150 && (frightenedTimer / 12) % 2 == 0) {
                        gColor = { 255, 255, 255, 255 }; // Flashing Warning
                    } else {
                        gColor = { 30, 64, 175, 255 }; // Deep Blue
                    }
                }

                // Head
                drawFilledCircle(renderer, static_cast<int>(gx), static_cast<int>(gy - 2), static_cast<int>(rad), gColor);

                // Body Skirt
                int rx = static_cast<int>(gx - rad);
                int rw = static_cast<int>(rad * 2);
                SDL_Rect skirt = { rx, static_cast<int>(gy - 2), rw, static_cast<int>(rad + 2) };
                SDL_SetRenderDrawColor(renderer, gColor.r, gColor.g, gColor.b, 255);
                SDL_RenderFillRect(renderer, &skirt);

                // 3 Undulating Ruffles at the base
                float ruffleWave = std::sin(SDL_GetTicks() * 0.015f) * 2.5f;
                int ruffleY = static_cast<int>(gy + rad);
                drawFilledCircle(renderer, static_cast<int>(gx - rad * 0.6f), static_cast<int>(ruffleY + ruffleWave), 3, gColor);
                drawFilledCircle(renderer, static_cast<int>(gx), static_cast<int>(ruffleY - ruffleWave), 3, gColor);
                drawFilledCircle(renderer, static_cast<int>(gx + rad * 0.6f), static_cast<int>(ruffleY + ruffleWave), 3, gColor);

                // Eyes
                if (g.state == GhostState::FRIGHTENED) {
                    drawFilledCircle(renderer, static_cast<int>(gx - 4), static_cast<int>(gy - 3), 2, { 254, 215, 170, 255 });
                    drawFilledCircle(renderer, static_cast<int>(gx + 4), static_cast<int>(gy - 3), 2, { 254, 215, 170, 255 });
                } else {
                    int eyeOffX = (g.dir == Direction::RIGHT ? 2 : (g.dir == Direction::LEFT ? -2 : 0));
                    int eyeOffY = (g.dir == Direction::DOWN ? 2 : (g.dir == Direction::UP ? -2 : 0));

                    drawFilledCircle(renderer, static_cast<int>(gx - 4), static_cast<int>(gy - 3), 4, { 255, 255, 255, 255 });
                    drawFilledCircle(renderer, static_cast<int>(gx + 4), static_cast<int>(gy - 3), 4, { 255, 255, 255, 255 });
                    drawFilledCircle(renderer, static_cast<int>(gx - 4 + eyeOffX), static_cast<int>(gy - 3 + eyeOffY), 2, { 30, 58, 138, 255 });
                    drawFilledCircle(renderer, static_cast<int>(gx + 4 + eyeOffX), static_cast<int>(gy - 3 + eyeOffY), 2, { 30, 58, 138, 255 });
                }
            }
        }

        // Floating points
        for (const auto& fs : floatingScores) {
            Uint8 a = static_cast<Uint8>(std::clamp(fs.alpha, 0.0f, 1.0f) * 255.0f);
            drawArcadeText(renderer, fs.text, static_cast<int>(fs.x - 12), static_cast<int>(offsetY + fs.y), 2, { 56, 189, 248, a });
        }

        // Footer Bar
        int footerY = WINDOW_HEIGHT - FOOTER_HEIGHT + 10;
        drawArcadeText(renderer, "LIVES:", 24, footerY + 8, 2, { 148, 163, 184, 255 });
        for (int i = 0; i < lives - 1; i++) {
            drawPacmanExact(renderer, 95.0f + i * 24.0f, footerY + 14.0f, 8.0f, 0.35f, Direction::RIGHT, { 250, 204, 21, 255 });
        }

        drawArcadeText(renderer, "LVL " + std::to_string(level), WINDOW_WIDTH - 220, footerY + 8, 2, { 34, 197, 94, 255 });
        drawArcadeText(renderer, "[P]AUSE [M]UTE", WINDOW_WIDTH - 140, footerY + 8, 1, { 148, 163, 184, 255 });

        // Overlays
        if (state == GameState::START_SCREEN) {
            SDL_Rect darkBack = { WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 140, 400, 260 };
            SDL_SetRenderDrawColor(renderer, 3, 7, 18, 240);
            SDL_RenderFillRect(renderer, &darkBack);
            SDL_SetRenderDrawColor(renderer, 59, 130, 246, 255);
            SDL_RenderDrawRect(renderer, &darkBack);

            drawArcadeText(renderer, "PAC-MAN", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 110, 4, { 250, 204, 21, 255 });
            drawArcadeText(renderer, "PRESS SPACE OR ENTER", WINDOW_WIDTH / 2 - 130, WINDOW_HEIGHT / 2 - 30, 2, { 255, 255, 255, 255 });
            drawArcadeText(renderer, "TO START GAME", WINDOW_WIDTH / 2 - 85, WINDOW_HEIGHT / 2, 2, { 255, 255, 255, 255 });
            drawArcadeText(renderer, "CONTROLS: ARROWS / WASD", WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 + 50, 1, { 148, 163, 184, 255 });
        } else if (state == GameState::READY) {
            drawArcadeText(renderer, "READY!", WINDOW_WIDTH / 2 - 50, offsetY + 17 * TILE_SIZE + 4, 3, { 253, 224, 71, 255 });
        } else if (state == GameState::PAUSED) {
            drawArcadeText(renderer, "PAUSED", WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 - 15, 3, { 56, 189, 248, 255 });
        } else if (state == GameState::GAME_OVER) {
            SDL_Rect darkBack = { WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 - 100, 360, 200 };
            SDL_SetRenderDrawColor(renderer, 3, 7, 18, 240);
            SDL_RenderFillRect(renderer, &darkBack);
            SDL_SetRenderDrawColor(renderer, 239, 68, 68, 255);
            SDL_RenderDrawRect(renderer, &darkBack);

            drawArcadeText(renderer, "GAME OVER", WINDOW_WIDTH / 2 - 95, WINDOW_HEIGHT / 2 - 60, 3, { 239, 68, 68, 255 });
            drawArcadeText(renderer, "FINAL SCORE: " + std::to_string(score), WINDOW_WIDTH / 2 - 110, WINDOW_HEIGHT / 2 - 10, 2, { 255, 255, 255, 255 });
            drawArcadeText(renderer, "PRESS SPACE TO PLAY AGAIN", WINDOW_WIDTH / 2 - 130, WINDOW_HEIGHT / 2 + 40, 1, { 250, 204, 21, 255 });
        }

        SDL_RenderPresent(renderer);
    }

    void run() {
        bool running = true;
        const int TARGET_FPS = 60;
        const int FRAME_DELAY = 1000 / TARGET_FPS; // ~16.6ms per frame

        while (running) {
            Uint32 frameStart = SDL_GetTicks();

            handleInput(running);
            update();
            render();

            Uint32 frameTime = SDL_GetTicks() - frameStart;
            if (frameTime < FRAME_DELAY) {
                SDL_Delay(FRAME_DELAY - frameTime);
            }
        }
    }
};

int main(int, char*[]) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    PacmanArcade game;
    if (!game.init()) {
        std::cerr << "Failed to initialize Pac-Man SDL2 window!" << std::endl;
        return 1;
    }
    game.run();
    return 0;
}
