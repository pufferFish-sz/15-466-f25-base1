#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

#include "Frogie.hpp"
#include "AssetImporter.hpp"
#include "GameAssets.hpp"
#include "Load.hpp"
#include <iostream>
#include <stdexcept>
#include <string>

PlayMode::PlayMode() {
	load_frogie_assets(ppu, frogie);
	frogie.Frogie_x = PPU466::ScreenWidth / 2;
	frogie.Frogie_y = PPU466::ScreenHeight / 4;
	load_background_assets(ppu);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	/*if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}*/
	(void)evt;
	(void)window_size;
	return false;
}

void PlayMode::update(float elapsed) {

	////slowly rotates through [0,1):
	//// (will be used to set background color)
	//background_fade += elapsed / 10.0f;
	//background_fade -= std::floor(background_fade);

	//constexpr float PlayerSpeed = 30.0f;
	//if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	//if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	//if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	//if (up.pressed) player_at.y += PlayerSpeed * elapsed;

	////reset button press counters:
	//left.downs = 0;
	//right.downs = 0;
	//up.downs = 0;
	//down.downs = 0;

	frogie.update(elapsed);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//ppu.background_color = glm::u8vec4(0x00, 0x00, 0x00, 0xFF);

	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	//for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
	//	for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
	//		//TODO: make weird plasma thing
	//		//ppu.background[x+PPU466::BackgroundWidth*y] = ((x+y)%16);
	//		ppu.background[x + PPU466::BackgroundWidth * y] = 0;
	//	}
	//}

	////background scroll:
	//ppu.background_position.x = int32_t(-0.5f * player_at.x);
	//ppu.background_position.y = int32_t(-0.5f * player_at.y);

	//player sprite:
	/*ppu.sprites[0].x = int8_t(player_at.x);
	ppu.sprites[0].y = int8_t(player_at.y);
	ppu.sprites[0].index = 32;
	ppu.sprites[0].attributes = 7;*/
	
	//Frogie frogie;
	frogie.draw(ppu, 0);

	//--- actually draw ---
	ppu.draw(drawable_size);
}
