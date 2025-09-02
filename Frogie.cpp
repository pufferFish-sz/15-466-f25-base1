#include "Frogie.hpp"
#include <string>
#include <iostream>

void Frogie::reset_anim() {
	time_accum = 0;
	frame = 0;
	tongue_active = false;
	tongue_frame = 0;
	tongue_time = 0.0f;
	tongue_cooldown_left = 0.0f;
}

void Frogie::start_tongue() {
	if (!visible) return;
	if (tongue_cooldown_left > 0.0f) return;
	tongue_active = true;
	tongue_frame = 0;
	tongue_time = 0.0f;
}

void Frogie::update(float dt) {
	if (!visible) return;
	// Idle
	time_accum += dt;
	const float frame_time = 1.0f / fps;
	while (time_accum >= frame_time) {
		time_accum -= frame_time;
		frame = (uint8_t)((frame + 1) % idle_frames.size());
	}

	// Tongue cooldown
	if (tongue_cooldown_left > 0.0f) {
		tongue_cooldown_left = std::max(0.0f, tongue_cooldown_left - dt);
	}

	// Tongue animation
	if (tongue_active) {
		tongue_time += dt;
		const float t_frame_time = 1.0f / tongue_fps;
		while ((tongue_time >= t_frame_time) && tongue_active) {
			tongue_time -= t_frame_time;
			if (tongue_frame + 1 < tongue_frames.size()) {
				tongue_frame++;
			}
			else {
				tongue_active = false;
				tongue_cooldown_left = 0.15f;
			}
		}
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
	if (sprite_slot + 10 >= ppu.sprites.size()) return 0;

	// Frogie body (16*16)
	const FileImportResult& H = idle_frames[frame];
	const uint8_t base = (uint8_t)(H.first_tile_index);
	const uint8_t attr = make_sprite_attributes(H.palette_index, false);
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

	s0.attributes = s1.attributes = s2.attributes = s3.attributes = attr;

	uint32_t sprite_used = 4;

	// tongue (8*8)
	auto& st = ppu.sprites[sprite_slot + sprite_used];
	if (tongue_active) {
		//if (sprite_slot + sprite_used >= ppu.sprites.size()) return sprite_used;
		const FileImportResult& T = tongue_frames[tongue_frame];

		st.x = uint8_t(int(Frogie_x) + tongue_off_x);
		st.y = uint8_t(int(Frogie_y) + tongue_off_y);
		//std::cout << "tongue x " << int(Frogie_x) + TONGUE_OFF_X << "tongue y " << int(Frogie_y) + TONGUE_OFF_Y << std::endl;
		//std::cout << "curr tongue frame is " << (int)tongue_frame << std::endl;
		st.index = uint8_t(T.first_tile_index);
		st.attributes = make_sprite_attributes(T.palette_index, /*behind_background = */ false);

		sprite_used++;
	}
	else {
		// when inactive, hide the sprite
		st.x = 0;
		st.y = 255;
		st.index = 0;
		st.attributes = 0;
	}
	return sprite_used;
}