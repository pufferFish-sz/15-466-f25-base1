#include "Cargo.hpp"
#include <algorithm> // remove_if
#include <iostream>


uint8_t make_sprite_attributes_1(uint8_t palette_index, bool behind_background = false) {
    uint8_t attr = 0;
    attr |= (palette_index & 0x07);  
    if (behind_background) attr |= (1u << 7); 
    return attr;
}

//Box
void Box::update(float dt) {
    // Move left in logic space
    xf -= speed_px_s * dt;
    //std::cout << "box x: " << xf << "box y: " << yf << std::endl;
    if (xf <= 0.0f) {
        alive = false;
    }
}

void Box::draw(PPU466& ppu, uint32_t sprite_slot) const {
    if (!alive) return;
    //std::cout << "box x: " << x << "box y: " << y << std::endl;
    int xi = (int)std::floor(xf);
    int yi = (int)std::floor(yf);
    if (xi + width <= 0) return;
    if (xi >= (int)PPU466::ScreenWidth) return;
    if (yi >= (int)PPU466::ScreenHeight) return;

    const uint8_t attr = make_sprite_attributes_1(palette_index, false);

    auto& s0 = ppu.sprites[sprite_slot + 0];
    auto& s1 = ppu.sprites[sprite_slot + 1];
    auto& s2 = ppu.sprites[sprite_slot + 2];
    auto& s3 = ppu.sprites[sprite_slot + 3];

    s0.x = uint8_t(xi); s0.y = uint8_t(yi);
    s1.x = uint8_t(xi + 8); s1.y = uint8_t(yi);
    s2.x = uint8_t(xi); s2.y = uint8_t(yi + 8);
    s3.x = uint8_t(xi + 8); s3.y = uint8_t(yi + 8);

    s0.index = uint8_t(base_tile_index + 0);
    s1.index = uint8_t(base_tile_index + 1);
    s2.index = uint8_t(base_tile_index + 2);
    s3.index = uint8_t(base_tile_index + 3);

    s0.attributes = s1.attributes = s2.attributes = s3.attributes = attr;
}

// Bugs
void Bug::update(float dt) {
    xf -= speed_px_s * dt;
    if (xf <= 0.0f) {
        alive = false;
    }

    // flip the 2 frames
    time_accum += dt;
    const float frame_time = 1.0f / fps;
    while (time_accum >= frame_time) {
        time_accum -= frame_time;
        frame = (frame + 1) % 2;
    }
}

void Bug::draw(PPU466& ppu, uint32_t sprite_slot) const {
    if (!alive) return;

    const int xi = (int)std::floor(xf);
    const int yi = (int)std::floor(yf);
    if (xi + width <= 0) return;
    if (xi >= (int)PPU466::ScreenWidth) return;
    if (yi >= (int)PPU466::ScreenHeight) return;

    const uint8_t attr = make_sprite_attributes_1(palette_index, false);

    auto& s = ppu.sprites[sprite_slot];   
    s.x = uint8_t(xi);
    s.y = uint8_t(yi);
    s.index = uint8_t(base_tile_index + frame); // 0 or 1
    s.attributes = attr;
}


//Cargo Manager：set random seed https://www.learncpp.com/cpp-tutorial/generating-random-numbers-using-mersenne-twister/
CargoManager::CargoManager() {
    std::random_device rd;        
    rng = std::mt19937(rd());    
    uni01 = std::uniform_real_distribution<float>(0.0f, 1.0f); 
}

void CargoManager::update(float dt) {

    if (stopped) return;
    // Move all
    for (auto& b : boxes) {
        b.update(dt);
    }
    for (auto& bug : bugs) {
        bug.update(dt);
    }

    // Make dead/off-screen: https://stackoverflow.com/questions/39019806/using-erase-remove-if-idiom
    boxes.erase(
        std::remove_if(boxes.begin(), boxes.end(),
            [](const Box& b) { return !b.alive; }),
        boxes.end()
    );
    bugs.erase(
        std::remove_if(bugs.begin(), bugs.end(),
            [](const Bug& bug) { return !bug.alive; }),
        bugs.end()
    );

    // spawn interval
    spawn_timer_s -= dt;
    if (spawn_timer_s <= 0.0f) {
        try_spawn();
        spawn_timer_s += spawn_interval_s;
    }
}

void CargoManager::draw(PPU466& ppu, uint32_t sprite_slot) const {
    uint32_t curr_location = sprite_slot;
    // draw boxes
    for (auto const& b : boxes) {
        if (curr_location + 3 >= ppu.sprites.size()) break; // 4 sprites per box
        b.draw(ppu, curr_location);
        curr_location += 4;
    }

    // draw bugs
    for (auto const& bug : bugs) {
        if (curr_location >= ppu.sprites.size()) break;
        bug.draw(ppu, curr_location);
        curr_location += 1;
    }

    // hide any leftover sprites from previous frame:
    for (uint32_t i = curr_location; i < ppu.sprites.size(); ++i) {
        ppu.sprites[i].y = 255; // move offscreen
    }
}

void CargoManager::try_spawn() {
    
    int rightmost = INT_MIN;

    if (!boxes.empty()) {
        const Box& b = boxes.back(); // last spawned box is also the rightmost among boxes
        rightmost = std::max(rightmost, int(std::floor(b.xf)) + b.width);
    }
    if (!bugs.empty()) {
        const Bug& bug = bugs.back(); 
        rightmost = std::max(rightmost, int(std::floor(bug.xf)) + bug.width);
    }

    if (rightmost != INT_MIN) {
        const int gap = spawn_x - rightmost;
        if (gap < min_gap_px) return; // too close to last cargo, skip this spawn
    }

    // spawn type
    const float r = uni01(rng);
    const bool spawn_bug = (r < bug_ratio);

    if (spawn_bug) {
        Bug bug;
        bug.xf = float(spawn_x);
        bug.yf = float(lane_y);
        bug.width = bug_w;
        bug.height = bug_h;
        bug.speed_px_s = conveyor_speed_px_s;
        bug.base_tile_index = bug_base_tile;
        bug.palette_index = bug_palette;
        bugs.emplace_back(bug);
    }
    else {
        Box b;
        b.xf = float(spawn_x);
        b.yf = float(lane_y);
        b.width = box_w;
        b.height = box_h;
        b.speed_px_s = conveyor_speed_px_s;
        b.base_tile_index = box_base_tile;
        b.palette_index = box_palette;

        boxes.emplace_back(b);
    }
   
}
