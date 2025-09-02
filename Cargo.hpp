#pragma once
#include "PPU466.hpp"
#include <vector>
#include <cstdint>
#include <random>
#include <array>
#include "AssetImporter.hpp"

// A moving object on the "conveyor". Uses integer pixel-space for logic.
// Draw-time casts to uint8_t to fit the PPU sprite registers.
struct Box {
    float xf = 0.0f;
    float yf = 0.0f;
    int   width = 16;
    int   height = 16;
    float speed_px_s = 40.0f;
    bool  alive = true;
    uint8_t base_tile_index = 0;
    uint8_t palette_index = 6;

    // Update leftward motion, it will kill when fully off-screen
    void update(float dt_s);

    void draw(PPU466& ppu, uint32_t sprite_slot) const;
};

struct Bug {
    float xf = 0.0f;
    float yf = 0.0f;
    int width = 8;
    int height = 8;
    float speed_px_s = 40.0f;
    bool  alive = true;
    uint8_t base_tile_index = 28;
    uint8_t palette_index = 5;

    // bug 2 frame animation
    std::array<FileImportResult, 2> fly_frames{};
    float fps = 6.0f;
    float time_accum = 0.0f;
    uint8_t frame = 0;

    void update(float dt_s);
    void draw(PPU466& ppu, uint32_t sprite_slot) const;
};

struct CargoManager {
    std::vector<Box> boxes;
    std::vector<Bug> bugs;

    bool stopped = false;

    // spawn
    float spawn_interval_s = 0.70f;
    float spawn_timer_s = 0.0f;
    float conveyor_speed_px_s = 40.0f;
    int spawn_x = (int)(PPU466::ScreenWidth - 16);  // starts off-screen and scrolls in
    int lane_y = 100;  
    int min_gap_px = 20;  

    // box sprite
    uint8_t box_base_tile = 0;
    uint8_t box_palette = 6;
    int     box_w = 16;         // default box size (tiles * 8)
    int     box_h = 16;

    // bug sprite
    uint8_t bug_base_tile = 28;
    uint8_t bug_palette = 5;
    int bug_w = 8;
    int bug_h = 8;
    float bug_ratio = 0.3f;

    // random generation
    std::mt19937 rng;
    std::uniform_real_distribution<float> uni01;

    CargoManager();

    void update(float dt_s);
    void draw(PPU466& ppu, uint32_t sprite_slot) const;

private:
    void try_spawn();
};
