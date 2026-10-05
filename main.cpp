#if defined(__has_include) && __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
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

using namespace std;

// pacman 1980 arcade recreation
// developer: Md. Abu Rise Zunaed

const int COLS = 28;
const int ROWS = 31;
const int TILE_SIZE = 24;
const int HEADER_H = 60;
const int FOOTER_H = 46;
const int WIN_W = COLS * TILE_SIZE; // 672
const int WIN_H = HEADER_H + (ROWS * TILE_SIZE) + FOOTER_H; // 850

// 1: wall, 2: dot, 3: energizer, 0: empty, 4: gate, 5: house, 6: tunnel
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

enum class Direction { NONE, UP, DOWN, LEFT, RIGHT };
enum class GhostState { IN_HOUSE, LEAVING_HOUSE, CHASE, SCATTER, FRIGHTENED, EATEN };
enum class GameState { START_SCREEN, READY, PLAYING, DYING, GHOST_PAUSE, LEVEL_CLEAR, GAME_OVER, PAUSED };

struct Pos {
    int x, y;
};

struct Fruit {
    string name;
    int points;
    SDL_Color color;
};

const vector<Fruit> STAGE_FRUITS = {
    { "Cherry", 100, { 239, 68, 68, 255 } },
    { "Strawberry", 300, { 244, 63, 94, 255 } },
    { "Peach", 500, { 251, 146, 60, 255 } },
    { "Apple", 700, { 34, 197, 94, 255 } },
    { "Grapes", 1000, { 168, 85, 247, 255 } },
    { "Galaxian", 2000, { 56, 189, 248, 255 } },
    { "Bell", 3000, { 234, 179, 8, 255 } },
    { "Key", 5000, { 6, 182, 212, 255 } }
};

struct ScorePopup {
    float x, y;
    string text;
    float alpha;
};

// 5x7 font table
const uint8_t FONT_5X7[][7] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 },
    { 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A },
    { 0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04 },
    { 0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03 },
    { 0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D },
    { 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02 },
    { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 },
    { 0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00 },
    { 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08 },
    { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04 },
    { 0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00 },
    { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E },
    { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E },
    { 0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F },
    { 0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E },
    { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 },
    { 0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E },
    { 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E },
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 },
    { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E },
    { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C },
    { 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00 },
    { 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x08 },
    { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02 },
    { 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00 },
    { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08 },
    { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 },
    { 0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0F },
    { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 },
    { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E },
    { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E },
    { 0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C },
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F },
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 },
    { 0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F },
    { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 },
    { 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E },
    { 0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C },
    { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 },
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F },
    { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11 },
    { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 },
    { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E },
    { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 },
    { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D },
    { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 },
    { 0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E },
    { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 },
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E },
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 },
    { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A },
    { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 },
    { 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04 },
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F },
    { 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E },
    { 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 },
    { 0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E }
};

void draw_text(SDL_Renderer* ren, const string& str, int sx, int sy, int sc, SDL_Color col) {
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    int cx = sx;
    for (char ch : str) {
        char uc = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
        if (uc >= ' ' && uc <= ']') {
            int idx = uc - ' ';
            for (int r = 0; r < 7; r++) {
                uint8_t row = FONT_5X7[idx][r];
                for (int b = 0; b < 5; b++) {
                    if (row & (0x10 >> b)) {
                        SDL_Rect rc = { cx + b * sc, sy + r * sc, sc, sc };
                        SDL_RenderFillRect(ren, &rc);
                    }
                }
            }
        }
        cx += 6 * sc;
    }
}

struct Note {
    double freq;
    double duration;
};

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

struct AudioSys {
    bool muted = false;
    bool music_active = false;
    bool playing_intro = false;
    size_t intro_idx = 0;
    double intro_time = 0.0;
    double music_phase = 0.0;
    double siren_phase = 0.0;
    int siren_mode = 0;

    bool sfx_active = false;
    double sfx_phase = 0.0;
    double sfx_time = 0.0;
    double sfx_dur = 0.0;
    double sfx_start_freq = 0.0;
    double sfx_end_freq = 0.0;
    int sfx_type = 0; // 0 sine, 1 tri, 2 sq
    double sfx_vol = 0.0;

    void toggle_mute() {
        muted = !muted;
    }

    void set_siren(int mode) {
        SDL_LockAudio();
        siren_mode = mode;
        if (mode == 0) {
            if (!playing_intro) music_active = false;
        } else {
            if (!playing_intro) music_active = true;
        }
        SDL_UnlockAudio();
    }

    void play_intro() {
        SDL_LockAudio();
        playing_intro = true;
        music_active = true;
        intro_idx = 0;
        intro_time = 0.0;
        music_phase = 0.0;
        SDL_UnlockAudio();
    }

    void play_chomp(bool high) {
        play_tone(high ? 540.0 : 380.0, high ? 320.0 : 220.0, 0.09, 1, 0.25);
    }

    void play_eat_ghost() {
        play_tone(300.0, 850.0, 0.35, 1, 0.35);
    }

    void play_fruit() {
        play_tone(900.0, 1600.0, 0.25, 0, 0.30);
    }

    void play_death() {
        play_tone(600.0, 80.0, 1.2, 0, 0.35);
    }

    void play_tone(double f1, double f2, double d, int wt, double v) {
        SDL_LockAudio();
        sfx_start_freq = f1;
        sfx_end_freq = f2;
        sfx_dur = d;
        sfx_type = wt;
        sfx_vol = v;
        sfx_time = 0.0;
        sfx_phase = 0.0;
        sfx_active = true;
        SDL_UnlockAudio();
    }
};

static AudioSys g_audio;

void audio_cb(void*, Uint8* stream, int len) {
    Sint16* buf = reinterpret_cast<Sint16*>(stream);
    int n = len / sizeof(Sint16);
    double sr = 44100.0;

    for (int i = 0; i < n; i++) {
        double mus = 0.0;
        double sfx = 0.0;

        if (g_audio.music_active && !g_audio.muted) {
            if (g_audio.playing_intro) {
                if (g_audio.intro_idx < INTRO_MELODY.size()) {
                    const Note& nt = INTRO_MELODY[g_audio.intro_idx];
                    g_audio.music_phase += (2.0 * M_PI * nt.freq) / sr;
                    if (g_audio.music_phase > 2.0 * M_PI) g_audio.music_phase -= 2.0 * M_PI;

                    double t = (g_audio.music_phase / M_PI) - 1.0;
                    if (t < 0.0) t = -t;
                    mus = ((t * 2.0) - 1.0) * 0.24;

                    g_audio.intro_time += 1.0 / sr;
                    if (g_audio.intro_time >= nt.duration) {
                        g_audio.intro_time = 0.0;
                        g_audio.intro_idx++;
                    }
                } else {
                    g_audio.playing_intro = false;
                    g_audio.music_active = (g_audio.siren_mode > 0);
                }
            } else if (g_audio.siren_mode > 0) {
                double speed = (g_audio.siren_mode == 2 ? 4.5 : (g_audio.siren_mode == 3 ? 9.0 : 1.8));
                g_audio.siren_phase += (2.0 * M_PI * speed) / sr;
                if (g_audio.siren_phase > 2.0 * M_PI) g_audio.siren_phase -= 2.0 * M_PI;

                double lfo = sin(g_audio.siren_phase);
                double base = 250.0, sw = 50.0;
                if (g_audio.siren_mode == 2) { base = 220.0; sw = 70.0; }
                if (g_audio.siren_mode == 3) { base = 540.0; sw = 180.0; }

                double freq = base + lfo * sw;
                g_audio.music_phase += (2.0 * M_PI * freq) / sr;
                if (g_audio.music_phase > 2.0 * M_PI) g_audio.music_phase -= 2.0 * M_PI;

                mus = sin(g_audio.music_phase) * (g_audio.siren_mode == 3 ? 0.09 : 0.06);
            }
        }

        if (g_audio.sfx_active && !g_audio.muted) {
            if (g_audio.sfx_time < g_audio.sfx_dur) {
                double p = g_audio.sfx_time / g_audio.sfx_dur;
                double cur_f = g_audio.sfx_start_freq + (g_audio.sfx_end_freq - g_audio.sfx_start_freq) * p;
                g_audio.sfx_phase += (2.0 * M_PI * cur_f) / sr;
                if (g_audio.sfx_phase > 2.0 * M_PI) g_audio.sfx_phase -= 2.0 * M_PI;

                double wave = 0.0;
                if (g_audio.sfx_type == 1) {
                    double t = (g_audio.sfx_phase / M_PI) - 1.0;
                    if (t < 0.0) t = -t;
                    wave = (t * 2.0) - 1.0;
                } else if (g_audio.sfx_type == 2) {
                    wave = (g_audio.sfx_phase < M_PI) ? 1.0 : -1.0;
                } else {
                    wave = sin(g_audio.sfx_phase);
                }

                sfx = wave * (g_audio.sfx_vol * (1.0 - p * 0.4));
                g_audio.sfx_time += 1.0 / sr;
            } else {
                g_audio.sfx_active = false;
            }
        }

        double val = mus + sfx;
        val = clamp(val, -0.95, 0.95);
        buf[i] = static_cast<Sint16>(val * 32000.0);
    }
}

void draw_circle(SDL_Renderer* ren, int cx, int cy, int r, SDL_Color col) {
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    for (int dy = -r; dy <= r; dy++) {
        int dx = static_cast<int>(round(sqrt(r * r - dy * dy)));
        SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void draw_pacman(SDL_Renderer* ren, float cx, float cy, float r, float mouth, Direction dir, SDL_Color col, float death = 0.0f) {
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    int ir = static_cast<int>(r);

    for (int dy = -ir; dy <= ir; dy++) {
        for (int dx = -ir; dx <= ir; dx++) {
            if (dx * dx + dy * dy <= r * r) {
                if (death > 0.0f) {
                    float lx = static_cast<float>(dx);
                    float ly = static_cast<float>(dy);
                    switch (dir) {
                        case Direction::RIGHT: lx = (float)dx;  ly = (float)dy;  break;
                        case Direction::DOWN:  lx = (float)dy;  ly = -(float)dx; break;
                        case Direction::LEFT:  lx = -(float)dx; ly = -(float)dy; break;
                        case Direction::UP:    lx = -(float)dy; ly = (float)dx;  break;
                        default: break;
                    }
                    float a = atan2(ly, lx);
                    if (abs(a) < death * static_cast<float>(M_PI)) continue;
                } else if (mouth > 0.05f) {
                    float lx = 0.0f, ly = 0.0f;
                    switch (dir) {
                        case Direction::RIGHT: lx = (float)dx;  ly = (float)dy;  break;
                        case Direction::DOWN:  lx = (float)dy;  ly = -(float)dx; break;
                        case Direction::LEFT:  lx = -(float)dx; ly = -(float)dy; break;
                        case Direction::UP:    lx = -(float)dy; ly = (float)dx;  break;
                        default: break;
                    }
                    if (lx > 0 && abs(ly) < lx * mouth) continue;
                }
                SDL_RenderDrawPoint(ren, static_cast<int>(cx + dx), static_cast<int>(cy + dy));
            }
        }
    }
}

struct Pacman {
    float x = 13.5f * TILE_SIZE;
    float y = 23.0f * TILE_SIZE + TILE_SIZE / 2.0f;
    Direction dir = Direction::LEFT;
    Direction next_dir = Direction::LEFT;
    float speed = 2.4f;

    float mouth_tan = 0.35f;
    float mouth_speed = 0.035f;
    bool mouth_closing = false;
    float death_prog = 0.0f;

    void reset() {
        x = 13.5f * TILE_SIZE;
        y = 23.0f * TILE_SIZE + TILE_SIZE / 2.0f;
        dir = Direction::LEFT;
        next_dir = Direction::LEFT;
        mouth_tan = 0.35f;
        mouth_closing = false;
        death_prog = 0.0f;
    }
};

struct Ghost {
    string name;
    SDL_Color color;
    float x, y;
    float start_x, start_y;
    Pos scatter_target;
    GhostState state;
    Direction dir = Direction::UP;
    float normal_speed = 2.0f;
    float fright_speed = 1.2f;
    float eaten_speed = 4.2f;
    int bounce_dir = -1;
    int last_tx = -1;
    int last_ty = -1;

    Ghost(string n, SDL_Color c, float sx, float sy, Pos sc, GhostState st)
        : name(n), color(c), x(sx * TILE_SIZE), y(sy * TILE_SIZE + TILE_SIZE / 2.0f),
          start_x(sx * TILE_SIZE), start_y(sy * TILE_SIZE + TILE_SIZE / 2.0f),
          scatter_target(sc), state(st) {}

    void reset(GhostState st) {
        x = start_x;
        y = start_y;
        dir = Direction::UP;
        state = st;
        bounce_dir = -1;
        last_tx = -1;
        last_ty = -1;
    }
};

class Game {
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    vector<vector<int>> maze;
    Pacman pacman;
    vector<Ghost> ghosts;
    vector<ScorePopup> popups;

    GameState state = GameState::START_SCREEN;
    int score = 0;
    int hi_score = 0;
    int lives = 3;
    int level = 1;
    int dots_left = 0;
    int total_dots = 0;

    int fright_timer = 0;
    int ghost_mult = 1;
    int mode_timer = 0;
    GhostState global_mode = GhostState::SCATTER;

    bool fruit_active = false;
    int fruit_timer = 0;
    int fruit_idx = 0;

    bool chomp_high = false;
    int flash_count = 0;
    bool flash_white = false;
    int freeze_timer = 0;

public:
    Game() {
        load_hi_score();
        init_maze();

        // blinky, pinky, inky, clyde
        ghosts.push_back(Ghost("Blinky", { 239, 68, 68, 255 }, 13.5f, 11.0f, { 27, 0 }, GhostState::SCATTER));
        ghosts.push_back(Ghost("Pinky",  { 244, 114, 182, 255 }, 13.5f, 14.0f, { 2, 0 }, GhostState::IN_HOUSE));
        ghosts.push_back(Ghost("Inky",   { 6, 182, 212, 255 }, 11.5f, 14.0f, { 27, 31 }, GhostState::IN_HOUSE));
        ghosts.push_back(Ghost("Clyde",  { 249, 115, 22, 255 }, 15.5f, 14.0f, { 0, 31 }, GhostState::IN_HOUSE));
    }

    ~Game() {
        save_hi_score();
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_CloseAudio();
        SDL_Quit();
    }

    void load_hi_score() {
        ifstream f("highscore.dat");
        if (f.is_open()) {
            if (!(f >> hi_score) || hi_score < 0) hi_score = 0;
            f.close();
        }
    }

    void save_hi_score() {
        if (score > hi_score) hi_score = score;
        ofstream f("highscore.dat");
        if (f.is_open()) {
            f << hi_score;
            f.close();
        }
    }

    void init_maze() {
        maze.clear();
        dots_left = 0;
        for (int r = 0; r < ROWS; r++) {
            vector<int> row;
            for (int c = 0; c < COLS; c++) {
                int v = RAW_MAZE[r][c] - '0';
                row.push_back(v);
                if (v == 2 || v == 3) dots_left++;
            }
            maze.push_back(row);
        }
        total_dots = dots_left;
    }

    void reset_positions() {
        pacman.reset();
        ghosts[0].reset(GhostState::SCATTER);
        ghosts[0].dir = Direction::LEFT;
        ghosts[1].reset(GhostState::IN_HOUSE);
        ghosts[2].reset(GhostState::IN_HOUSE);
        ghosts[3].reset(GhostState::IN_HOUSE);
        fright_timer = 0;
        mode_timer = 0;
        global_mode = GhostState::SCATTER;
        fruit_active = false;
        flash_white = false;
        flash_count = 0;
        g_audio.set_siren(0);
    }

    void new_game() {
        score = 0;
        lives = 3;
        level = 1;
        init_maze();
        reset_positions();
        state = GameState::READY;
        freeze_timer = 240;
        g_audio.play_intro();
    }

    bool init_sdl() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
            cerr << "SDL error: " << SDL_GetError() << endl;
            return false;
        }

        window = SDL_CreateWindow(
            "PAC-MAN Arcade Edition (C++17)",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WIN_W, WIN_H,
            SDL_WINDOW_SHOWN
        );
        if (!window) return false;

        renderer = SDL_CreateRenderer(
            window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );
        if (!renderer) return false;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        SDL_AudioSpec sp;
        SDL_zero(sp);
        sp.freq = 44100;
        sp.format = AUDIO_S16SYS;
        sp.channels = 1;
        sp.samples = 1024;
        sp.callback = audio_cb;

        if (SDL_OpenAudio(&sp, nullptr) == 0) {
            SDL_PauseAudio(0);
        }

        return true;
    }

    bool can_pass(int tx, int ty, Direction d, bool is_ghost = false, GhostState gst = GhostState::CHASE) {
        int nx = tx, ny = ty;
        if (d == Direction::UP) ny--;
        else if (d == Direction::DOWN) ny++;
        else if (d == Direction::LEFT) nx--;
        else if (d == Direction::RIGHT) nx++;

        if (ny == 14 && (nx < 0 || nx >= COLS)) return true;
        if (nx < 0 || nx >= COLS || ny < 0 || ny >= ROWS) return false;

        int cell = maze[ny][nx];
        if (cell == 1) return false;
        if (cell == 4) return is_ghost && (gst == GhostState::LEAVING_HOUSE || gst == GhostState::EATEN);
        if (cell == 5) return is_ghost && (gst == GhostState::IN_HOUSE || gst == GhostState::LEAVING_HOUSE || gst == GhostState::EATEN);
        return true;
    }

    void handle_input(bool& running) {
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
                        pacman.next_dir = Direction::UP;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        pacman.next_dir = Direction::DOWN;
                        break;
                    case SDLK_LEFT:
                    case SDLK_a:
                        pacman.next_dir = Direction::LEFT;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        pacman.next_dir = Direction::RIGHT;
                        break;
                    case SDLK_p:
                        if (state == GameState::PLAYING) {
                            state = GameState::PAUSED;
                            g_audio.set_siren(0);
                        } else if (state == GameState::PAUSED) {
                            state = GameState::PLAYING;
                            g_audio.set_siren(fright_timer > 0 ? 2 : 1);
                        }
                        break;
                    case SDLK_m:
                        g_audio.toggle_mute();
                        break;
                    case SDLK_SPACE:
                    case SDLK_RETURN:
                        if (state == GameState::START_SCREEN || state == GameState::GAME_OVER) {
                            new_game();
                        } else if (state == GameState::PAUSED) {
                            state = GameState::PLAYING;
                            g_audio.set_siren(fright_timer > 0 ? 2 : 1);
                        }
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                }
            }
        }
    }

    void update_player() {
        int tx = static_cast<int>(pacman.x / TILE_SIZE);
        int ty = static_cast<int>(pacman.y / TILE_SIZE);
        float cx = tx * TILE_SIZE + TILE_SIZE / 2.0f;
        float cy = ty * TILE_SIZE + TILE_SIZE / 2.0f;

        if ((pacman.dir == Direction::LEFT && pacman.next_dir == Direction::RIGHT) ||
            (pacman.dir == Direction::RIGHT && pacman.next_dir == Direction::LEFT) ||
            (pacman.dir == Direction::UP && pacman.next_dir == Direction::DOWN) ||
            (pacman.dir == Direction::DOWN && pacman.next_dir == Direction::UP)) {
            pacman.dir = pacman.next_dir;
        }

        float dc = hypot(pacman.x - cx, pacman.y - cy);
        if (pacman.next_dir != pacman.dir && dc < 6.0f) {
            if (can_pass(tx, ty, pacman.next_dir)) {
                pacman.x = cx;
                pacman.y = cy;
                pacman.dir = pacman.next_dir;
            }
        }

        bool blocked = false;
        if (dc < pacman.speed) {
            if (!can_pass(tx, ty, pacman.dir)) {
                pacman.x = cx;
                pacman.y = cy;
                blocked = true;
            }
        }

        if (!blocked) {
            if (pacman.dir == Direction::LEFT || pacman.dir == Direction::RIGHT) {
                pacman.y = cy;
                if (pacman.dir == Direction::LEFT) pacman.x -= pacman.speed;
                else pacman.x += pacman.speed;
            } else if (pacman.dir == Direction::UP || pacman.dir == Direction::DOWN) {
                pacman.x = cx;
                if (pacman.dir == Direction::UP) pacman.y -= pacman.speed;
                else pacman.y += pacman.speed;
            }

            if (pacman.mouth_closing) {
                pacman.mouth_tan -= pacman.mouth_speed;
                if (pacman.mouth_tan <= 0.05f) pacman.mouth_closing = false;
            } else {
                pacman.mouth_tan += pacman.mouth_speed;
                if (pacman.mouth_tan >= 0.65f) pacman.mouth_closing = true;
            }
        }

        // wrap tunnel row 14
        if (ty == 14) {
            if (pacman.x < -TILE_SIZE / 2.0f) pacman.x = WIN_W + TILE_SIZE / 2.0f;
            else if (pacman.x > WIN_W + TILE_SIZE / 2.0f) pacman.x = -TILE_SIZE / 2.0f;
        }

        int ctx = static_cast<int>(floor(pacman.x / TILE_SIZE));
        int cty = static_cast<int>(floor(pacman.y / TILE_SIZE));

        if (ctx >= 0 && ctx < COLS && cty >= 0 && cty < ROWS) {
            int& cell = maze[cty][ctx];
            if (cell == 2) {
                cell = 0;
                dots_left--;
                score += 10;
                chomp_high = !chomp_high;
                g_audio.play_chomp(chomp_high);
                check_fruit();
                check_win();
            } else if (cell == 3) {
                cell = 0;
                dots_left--;
                score += 50;
                fright_timer = 480;
                ghost_mult = 1;
                g_audio.play_chomp(true);
                g_audio.set_siren(2);

                for (auto& g : ghosts) {
                    if (g.state == GhostState::CHASE || g.state == GhostState::SCATTER) {
                        g.state = GhostState::FRIGHTENED;
                        if (g.dir == Direction::UP) g.dir = Direction::DOWN;
                        else if (g.dir == Direction::DOWN) g.dir = Direction::UP;
                        else if (g.dir == Direction::LEFT) g.dir = Direction::RIGHT;
                        else if (g.dir == Direction::RIGHT) g.dir = Direction::LEFT;
                    }
                }
                check_fruit();
                check_win();
            }
        }

        if (fruit_active) {
            float dist = hypot(pacman.x - (13.5f * TILE_SIZE), pacman.y - (17.5f * TILE_SIZE));
            if (dist < TILE_SIZE * 0.8f) {
                fruit_active = false;
                int pts = STAGE_FRUITS[fruit_idx].points;
                score += pts;
                g_audio.play_fruit();
                popups.push_back({ 13.5f * TILE_SIZE, 17.5f * TILE_SIZE, to_string(pts), 1.0f });
            }
        }
    }

    void check_fruit() {
        if (!fruit_active && (dots_left == total_dots - 70 || dots_left == total_dots - 170)) {
            fruit_active = true;
            fruit_timer = 600;
            fruit_idx = min(level - 1, static_cast<int>(STAGE_FRUITS.size() - 1));
        }
    }

    void check_win() {
        if (dots_left <= 0) {
            state = GameState::LEVEL_CLEAR;
            flash_count = 0;
            flash_white = false;
            freeze_timer = 160;
            g_audio.set_siren(0);
        }
    }

    Pos get_target(const Ghost& g) {
        int px = static_cast<int>(pacman.x / TILE_SIZE);
        int py = static_cast<int>(pacman.y / TILE_SIZE);

        if (g.state == GhostState::EATEN) return { 13, 11 };
        if (g.state == GhostState::SCATTER) return g.scatter_target;

        if (g.name == "Blinky") {
            return { px, py };
        } else if (g.name == "Pinky") {
            Pos p = { px, py };
            switch (pacman.dir) {
                case Direction::UP:    p.y -= 4; p.x -= 4; break;
                case Direction::DOWN:  p.y += 4; break;
                case Direction::LEFT:  p.x -= 4; break;
                case Direction::RIGHT: p.x += 4; break;
                default: break;
            }
            return p;
        } else if (g.name == "Inky") {
            Pos p2 = { px, py };
            switch (pacman.dir) {
                case Direction::UP:    p2.y -= 2; p2.x -= 2; break;
                case Direction::DOWN:  p2.y += 2; break;
                case Direction::LEFT:  p2.x -= 2; break;
                case Direction::RIGHT: p2.x += 2; break;
                default: break;
            }
            int bx = static_cast<int>(ghosts[0].x / TILE_SIZE);
            int by = static_cast<int>(ghosts[0].y / TILE_SIZE);
            return { p2.x + (p2.x - bx), p2.y + (p2.y - by) };
        } else if (g.name == "Clyde") {
            int cx = static_cast<int>(g.x / TILE_SIZE);
            int cy = static_cast<int>(g.y / TILE_SIZE);
            if (hypot(cx - px, cy - py) > 8.0f) return { px, py };
            return g.scatter_target;
        }
        return { px, py };
    }

    Direction choose_dir(Ghost& g, int tx, int ty) {
        if (g.state == GhostState::FRIGHTENED) {
            vector<Direction> choices;
            Direction opp = Direction::NONE;
            if (g.dir == Direction::UP) opp = Direction::DOWN;
            else if (g.dir == Direction::DOWN) opp = Direction::UP;
            else if (g.dir == Direction::LEFT) opp = Direction::RIGHT;
            else if (g.dir == Direction::RIGHT) opp = Direction::LEFT;

            for (Direction d : { Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT }) {
                if (d != opp && can_pass(tx, ty, d, true, g.state)) {
                    choices.push_back(d);
                }
            }
            if (!choices.empty()) return choices[rand() % choices.size()];
            return g.dir;
        }

        Pos tgt = get_target(g);
        Direction best_d = Direction::NONE;
        float min_d = 1e9f;

        Direction opp = Direction::NONE;
        if (g.dir == Direction::UP) opp = Direction::DOWN;
        else if (g.dir == Direction::DOWN) opp = Direction::UP;
        else if (g.dir == Direction::LEFT) opp = Direction::RIGHT;
        else if (g.dir == Direction::RIGHT) opp = Direction::LEFT;

        for (Direction d : { Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT }) {
            if (d == opp) continue;

            if (can_pass(tx, ty, d, true, g.state)) {
                int nx = tx, ny = ty;
                if (d == Direction::UP) ny--;
                else if (d == Direction::DOWN) ny++;
                else if (d == Direction::LEFT) nx--;
                else if (d == Direction::RIGHT) nx++;

                float dist = static_cast<float>((nx - tgt.x) * (nx - tgt.x) + (ny - tgt.y) * (ny - tgt.y));
                if (dist < min_d) {
                    min_d = dist;
                    best_d = d;
                }
            }
        }
        return (best_d != Direction::NONE) ? best_d : g.dir;
    }

    void update_ghosts() {
        if (fright_timer > 0) {
            fright_timer--;
            if (fright_timer == 0) {
                g_audio.set_siren(1);
                for (auto& g : ghosts) {
                    if (g.state == GhostState::FRIGHTENED) g.state = global_mode;
                }
            }
        } else {
            mode_timer++;
            if (global_mode == GhostState::SCATTER && mode_timer > 7 * 60) {
                global_mode = GhostState::CHASE;
                mode_timer = 0;
                for (auto& g : ghosts) {
                    if (g.state == GhostState::SCATTER) g.state = GhostState::CHASE;
                }
            } else if (global_mode == GhostState::CHASE && mode_timer > 20 * 60) {
                global_mode = GhostState::SCATTER;
                mode_timer = 0;
                for (auto& g : ghosts) {
                    if (g.state == GhostState::CHASE) g.state = GhostState::SCATTER;
                }
            }
        }

        for (size_t i = 0; i < ghosts.size(); i++) {
            auto& g = ghosts[i];

            if (g.state == GhostState::IN_HOUSE) {
                bool can_leave = false;
                if (g.name == "Pinky") can_leave = true;
                if (g.name == "Inky" && dots_left <= total_dots - 30) can_leave = true;
                if (g.name == "Clyde" && dots_left <= total_dots - 60) can_leave = true;

                if (can_leave) {
                    g.state = GhostState::LEAVING_HOUSE;
                } else {
                    float h_top = 13.5f * TILE_SIZE;
                    float h_bot = 14.5f * TILE_SIZE;
                    if (g.y <= h_top) g.bounce_dir = 1;
                    else if (g.y >= h_bot) g.bounce_dir = -1;
                    g.y += g.bounce_dir * (g.normal_speed * 0.5f);
                    g.dir = (g.bounce_dir > 0) ? Direction::DOWN : Direction::UP;
                    continue;
                }
            }

            if (g.state == GhostState::LEAVING_HOUSE) {
                float door_x = 13.5f * TILE_SIZE;
                float door_y = 11.5f * TILE_SIZE;

                if (abs(g.x - door_x) > 1.0f) {
                    g.x += (g.x < door_x) ? g.normal_speed : -g.normal_speed;
                    g.dir = (g.x < door_x) ? Direction::RIGHT : Direction::LEFT;
                } else {
                    g.x = door_x;
                    if (g.y > door_y) {
                        g.y -= g.normal_speed;
                        g.dir = Direction::UP;
                    } else {
                        g.y = door_y;
                        g.state = global_mode;
                        g.dir = Direction::LEFT;
                        g.last_tx = -1;
                        g.last_ty = -1;
                    }
                }
                continue;
            }

            if (g.state == GhostState::EATEN) {
                float door_x = 13.5f * TILE_SIZE;
                float door_y = 11.5f * TILE_SIZE;
                if (hypot(g.x - door_x, g.y - door_y) < 4.0f) {
                    g.x = door_x;
                    g.y = 14.0f * TILE_SIZE + TILE_SIZE / 2.0f;
                    g.state = GhostState::LEAVING_HOUSE;
                    g.last_tx = -1;
                    g.last_ty = -1;
                    continue;
                }
            }

            float spd = g.normal_speed;
            if (g.state == GhostState::FRIGHTENED) spd = g.fright_speed;
            else if (g.state == GhostState::EATEN) spd = g.eaten_speed;

            int tx = static_cast<int>(g.x / TILE_SIZE);
            int ty = static_cast<int>(g.y / TILE_SIZE);
            if (ty == 14 && (tx <= 5 || tx >= 22)) spd *= 0.5f;

            float cx = tx * TILE_SIZE + TILE_SIZE / 2.0f;
            float cy = ty * TILE_SIZE + TILE_SIZE / 2.0f;
            float dc = hypot(g.x - cx, g.y - cy);

            if (dc < spd && (tx != g.last_tx || ty != g.last_ty)) {
                g.x = cx;
                g.y = cy;
                g.dir = choose_dir(g, tx, ty);
                g.last_tx = tx;
                g.last_ty = ty;
            }

            if (g.dir == Direction::LEFT) g.x -= spd;
            else if (g.dir == Direction::RIGHT) g.x += spd;
            else if (g.dir == Direction::UP) g.y -= spd;
            else if (g.dir == Direction::DOWN) g.y += spd;

            if (ty == 14) {
                if (g.x < -TILE_SIZE / 2.0f) g.x = WIN_W + TILE_SIZE / 2.0f;
                else if (g.x > WIN_W + TILE_SIZE / 2.0f) g.x = -TILE_SIZE / 2.0f;
            }
        }

        bool eaten = false;
        for (const auto& g : ghosts) {
            if (g.state == GhostState::EATEN) eaten = true;
        }
        if (eaten) g_audio.set_siren(3);
        else if (fright_timer > 0) g_audio.set_siren(2);
        else if (state == GameState::PLAYING) g_audio.set_siren(1);
    }

    void check_collisions() {
        for (auto& g : ghosts) {
            if (hypot(pacman.x - g.x, pacman.y - g.y) < TILE_SIZE * 0.75f) {
                if (g.state == GhostState::FRIGHTENED) {
                    g.state = GhostState::EATEN;
                    int pts = 200 * ghost_mult;
                    score += pts;
                    ghost_mult *= 2;
                    g_audio.play_eat_ghost();
                    popups.push_back({ g.x, g.y, to_string(pts), 1.0f });
                    state = GameState::GHOST_PAUSE;
                    freeze_timer = 40;
                    return;
                } else if (g.state == GhostState::CHASE || g.state == GhostState::SCATTER) {
                    state = GameState::DYING;
                    freeze_timer = 90;
                    g_audio.play_death();
                    return;
                }
            }
        }
    }

    void update() {
        for (auto it = popups.begin(); it != popups.end();) {
            it->y -= 0.5f;
            it->alpha -= 0.02f;
            if (it->alpha <= 0.0f) it = popups.erase(it);
            else ++it;
        }

        if (fruit_active && --fruit_timer <= 0) fruit_active = false;

        if (state == GameState::READY) {
            if (--freeze_timer <= 0) {
                state = GameState::PLAYING;
                g_audio.set_siren(1);
            }
            return;
        }

        if (state == GameState::GHOST_PAUSE) {
            if (--freeze_timer <= 0) state = GameState::PLAYING;
            return;
        }

        if (state == GameState::DYING) {
            pacman.death_prog += 0.012f;
            if (--freeze_timer <= 0) {
                if (--lives <= 0) {
                    state = GameState::GAME_OVER;
                    save_hi_score();
                } else {
                    reset_positions();
                    state = GameState::READY;
                    freeze_timer = 120;
                }
            }
            return;
        }

        if (state == GameState::LEVEL_CLEAR) {
            if (--freeze_timer % 20 == 0) {
                flash_white = !flash_white;
                flash_count++;
            }
            if (freeze_timer <= 0) {
                level++;
                init_maze();
                reset_positions();
                state = GameState::READY;
                freeze_timer = 120;
            }
            return;
        }

        if (state != GameState::PLAYING) return;

        update_player();
        update_ghosts();
        check_collisions();
    }

    void draw_maze() {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                int cell = maze[r][c];
                int px = c * TILE_SIZE;
                int py = HEADER_H + r * TILE_SIZE;

                if (cell == 1) {
                    SDL_Color wc = flash_white ? SDL_Color{ 255, 255, 255, 255 } : SDL_Color{ 33, 33, 222, 255 };
                    SDL_SetRenderDrawColor(renderer, wc.r, wc.g, wc.b, 255);

                    SDL_Rect inner = { px + 2, py + 2, TILE_SIZE - 4, TILE_SIZE - 4 };
                    SDL_RenderDrawRect(renderer, &inner);

                    if (c + 1 < COLS && maze[r][c + 1] == 1) {
                        SDL_RenderDrawLine(renderer, px + TILE_SIZE - 2, py + 2, px + TILE_SIZE, py + 2);
                        SDL_RenderDrawLine(renderer, px + TILE_SIZE - 2, py + TILE_SIZE - 3, px + TILE_SIZE, py + TILE_SIZE - 3);
                    }
                    if (r + 1 < ROWS && maze[r + 1][c] == 1) {
                        SDL_RenderDrawLine(renderer, px + 2, py + TILE_SIZE - 2, px + 2, py + TILE_SIZE);
                        SDL_RenderDrawLine(renderer, px + TILE_SIZE - 3, py + TILE_SIZE - 2, px + TILE_SIZE - 3, py + TILE_SIZE);
                    }
                } else if (cell == 4) {
                    SDL_SetRenderDrawColor(renderer, 244, 114, 182, 255);
                    SDL_Rect gate = { px, py + TILE_SIZE / 2 - 2, TILE_SIZE, 4 };
                    SDL_RenderFillRect(renderer, &gate);
                } else if (cell == 2) {
                    draw_circle(renderer, px + TILE_SIZE / 2, py + TILE_SIZE / 2, 3, { 255, 184, 151, 255 });
                } else if (cell == 3) {
                    draw_circle(renderer, px + TILE_SIZE / 2, py + TILE_SIZE / 2, 7, { 255, 184, 151, 255 });
                }
            }
        }
    }

    void draw_fruit() {
        if (!fruit_active) return;
        int fx = static_cast<int>(13.5f * TILE_SIZE);
        int fy = HEADER_H + static_cast<int>(17.5f * TILE_SIZE);

        draw_circle(renderer, fx, fy, 8, STAGE_FRUITS[fruit_idx].color);
        SDL_SetRenderDrawColor(renderer, 34, 197, 94, 255);
        SDL_RenderDrawLine(renderer, fx, fy - 8, fx + 3, fy - 13);
    }

    void draw_ghost(const Ghost& g) {
        if (state == GameState::DYING) return;

        int gx = static_cast<int>(g.x);
        int gy = HEADER_H + static_cast<int>(g.y);
        int r = 11;

        if (g.state == GhostState::EATEN) {
            draw_eyes(gx, gy, g.dir);
            return;
        }

        SDL_Color col = g.color;
        if (g.state == GhostState::FRIGHTENED) {
            if (fright_timer < 120 && (fright_timer / 15) % 2 == 0) {
                col = { 255, 255, 255, 255 };
            } else {
                col = { 33, 33, 255, 255 };
            }
        }

        draw_circle(renderer, gx, gy - 2, r, col);
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
        SDL_Rect skirt = { gx - r, gy - 2, r * 2 + 1, r + 2 };
        SDL_RenderFillRect(renderer, &skirt);

        for (int i = 0; i < 3; i++) {
            draw_circle(renderer, gx - r + 3 + i * 8, gy + r, 3, col);
        }

        if (g.state == GhostState::FRIGHTENED) {
            draw_circle(renderer, gx - 4, gy - 2, 2, { 255, 184, 151, 255 });
            draw_circle(renderer, gx + 4, gy - 2, 2, { 255, 184, 151, 255 });
        } else {
            draw_eyes(gx, gy, g.dir);
        }
    }

    void draw_eyes(int gx, int gy, Direction dir) {
        int ox = 0, oy = 0;
        if (dir == Direction::LEFT) ox = -2;
        else if (dir == Direction::RIGHT) ox = 2;
        else if (dir == Direction::UP) oy = -2;
        else if (dir == Direction::DOWN) oy = 2;

        draw_circle(renderer, gx - 4, gy - 3, 4, { 255, 255, 255, 255 });
        draw_circle(renderer, gx + 4, gy - 3, 4, { 255, 255, 255, 255 });
        draw_circle(renderer, gx - 4 + ox, gy - 3 + oy, 2, { 33, 33, 255, 255 });
        draw_circle(renderer, gx + 4 + ox, gy - 3 + oy, 2, { 33, 33, 255, 255 });
    }

    void draw_hud() {
        draw_text(renderer, "1UP SCORE", 30, 12, 2, { 56, 189, 248, 255 });
        draw_text(renderer, to_string(score), 30, 32, 2, { 255, 255, 255, 255 });

        draw_text(renderer, "HIGH SCORE", WIN_W - 170, 12, 2, { 239, 68, 68, 255 });
        draw_text(renderer, to_string(max(score, hi_score)), WIN_W - 170, 32, 2, { 255, 255, 255, 255 });

        int footer_y = WIN_H - 32;
        draw_text(renderer, "LIVES:", 30, footer_y, 2, { 148, 163, 184, 255 });
        for (int i = 0; i < lives - 1; i++) {
            draw_pacman(renderer, 120 + i * 26, footer_y + 6, 9.0f, 0.35f, Direction::RIGHT, { 250, 204, 21, 255 });
        }

        string lvl = "LVL " + to_string(level);
        draw_text(renderer, lvl, WIN_W - 110, footer_y, 2, { 34, 197, 94, 255 });

        if (state == GameState::START_SCREEN) {
            draw_text(renderer, "PAC-MAN", WIN_W / 2 - 80, WIN_H / 2 - 60, 4, { 250, 204, 21, 255 });
            draw_text(renderer, "PRESS SPACE OR ENTER", WIN_W / 2 - 120, WIN_H / 2 + 10, 2, { 255, 255, 255, 255 });
            draw_text(renderer, "MD. ABU RISE ZUNAED", WIN_W / 2 - 110, WIN_H / 2 + 50, 2, { 56, 189, 248, 255 });
        } else if (state == GameState::READY) {
            draw_text(renderer, "READY!", WIN_W / 2 - 36, HEADER_H + 17 * TILE_SIZE + 6, 2, { 250, 204, 21, 255 });
        } else if (state == GameState::PAUSED) {
            draw_text(renderer, "GAME PAUSED", WIN_W / 2 - 65, HEADER_H + 17 * TILE_SIZE + 6, 2, { 56, 189, 248, 255 });
        } else if (state == GameState::GAME_OVER) {
            draw_text(renderer, "GAME  OVER", WIN_W / 2 - 60, HEADER_H + 17 * TILE_SIZE + 6, 2, { 239, 68, 68, 255 });
        }

        for (const auto& sp : popups) {
            SDL_Color c = { 56, 189, 248, static_cast<Uint8>(sp.alpha * 255) };
            draw_text(renderer, sp.text, static_cast<int>(sp.x) - 12, HEADER_H + static_cast<int>(sp.y) - 6, 2, c);
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 5, 8, 20, 255);
        SDL_RenderClear(renderer);

        draw_maze();
        draw_fruit();

        if (state != GameState::START_SCREEN) {
            draw_pacman(renderer, pacman.x, HEADER_H + pacman.y, 11.0f, pacman.mouth_tan, pacman.dir, { 250, 204, 21, 255 }, pacman.death_prog);
            for (const auto& g : ghosts) {
                draw_ghost(g);
            }
        }

        draw_hud();
        SDL_RenderPresent(renderer);
    }

    void run() {
        bool running = true;
        const auto step = chrono::microseconds(16666);
        auto last = chrono::high_resolution_clock::now();
        auto accum = chrono::microseconds(0);

        while (running) {
            auto now = chrono::high_resolution_clock::now();
            auto dur = chrono::duration_cast<chrono::microseconds>(now - last);
            last = now;

            if (dur > chrono::milliseconds(100)) dur = chrono::milliseconds(100);
            accum += dur;

            handle_input(running);

            while (accum >= step) {
                update();
                accum -= step;
            }

            render();
            SDL_Delay(1);
        }
    }
};

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    srand((unsigned int)time(nullptr));

    Game game;
    if (!game.init_sdl()) {
        cerr << "Failed to init SDL" << endl;
        return 1;
    }

    game.run();
    return 0;
}
