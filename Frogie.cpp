#include "Frogie.hpp"
#include <string>
#include <iostream>

void Frogie::reset_anim() {
	time_accum = 0;
	frame = 0;
}

void Frogie::update(float dt) {
	if (!visible) return;
	time_accum += dt;
	const float frame_time = 1.0f / fps;
	while (time_accum >= frame_time) {
		time_accum -= frame_time;
		frame = (uint8_t)((frame + 1) % idle_frames.size());
	}
}

uint8_t make_sprite_attributes(uint8_t palette_index, bool behind_background = false) {
	uint8_t attr = 0;
	attr |= (palette_index & 0x07);          // bits 0..2
	if (behind_background) attr |= (1u << 7); // priority bit
	return attr;
};

uint32_t Frogie::draw(PPU466& ppu, uint32_t sprite_slot) const {
	
	if (!visible) return 0;
	if (sprite_slot + 3 >= ppu.sprites.size()) return 0;

	const FileImportResult& H = idle_frames[frame];
	const uint8_t base = (uint8_t)(H.first_tile_index);
	const uint8_t attr = make_sprite_attributes(H.palette_index, /*behind_background = */false);
	//std::cout << "the base is: " << static_cast<int>(base) << "and the palette index is " << static_cast<int>(H.palette_index) << std::endl;
	// Piece Frogie together with 4 tiles
	/*
	[base + 2] [base + 3]
	[base + 0] [base + 1]*/
	auto& s0 = ppu.sprites[sprite_slot + 0];
	auto& s1 = ppu.sprites[sprite_slot + 1];
	auto& s2 = ppu.sprites[sprite_slot + 2];
	auto& s3 = ppu.sprites[sprite_slot + 3];

	s0.x = Frogie_x; s0.y = Frogie_y;
	s1.x = uint8_t(Frogie_x + 8); s1.y = Frogie_y;
	s2.x = Frogie_x; s2.y = uint8_t(Frogie_y + 8);
	s3.x = uint8_t(Frogie_x + 8); s3.y = uint8_t(Frogie_y + 8);

	s0.index = uint8_t(base + 0);
	s1.index = uint8_t(base + 1);
	s2.index = uint8_t(base + 2);
	s3.index = uint8_t(base + 3);

	s0.attributes = attr;
	s1.attributes = attr;
	s2.attributes = attr;
	s3.attributes = attr;
	return 4;
}