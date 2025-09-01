#pragma once
//#include "PPU466.hpp"
#include "AssetImporter.hpp"
#include <array>
class Frogie {
public:
	std::array<FileImportResult, 3> idle_frames{};

	// Position of Frogie
	uint8_t Frogie_x = 10;
	uint8_t Frogie_y = 10;

	// Animation
	float fps = 6.0f;
	void reset_anim();
	void update(float dt);

	// Visibility
	bool visible = true;

	uint32_t draw(PPU466& ppu, uint32_t sprite_slot) const;

private:
	float time_accum = 0.0f;
	uint8_t frame = 0;

};