#pragma once
//#include "PPU466.hpp"
#include "AssetImporter.hpp"
#include <array>

class Frogie {
public:
	std::array<FileImportResult, 3> idle_frames{};
	std::array<FileImportResult, 6> tongue_frames{};

	// Position of Frogie
	uint8_t Frogie_x = 10;
	uint8_t Frogie_y = 10;

	// Animation
	float fps = 6.0f;
	void reset_anim();
	void update(float dt);
	void start_tongue();
	uint8_t tongue_frame = 0;

	// Visibility
	bool visible = true;

	uint32_t draw(PPU466& ppu, uint32_t sprite_slot) const;

private:
	// Idle
	float time_accum = 0.0f;
	uint8_t frame = 0;

	// Tongue
	bool tongue_active = false;
	//uint8_t tongue_frame = 0;
	float tongue_time = 0.0f;
	float tongue_fps = 18.0f;
	float tongue_cooldown_left = 0.0f;

	int tongue_off_x = 4;
	int tongue_off_y = 14;

};