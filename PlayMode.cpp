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
	load_box(ppu, box);
	load_bug(ppu, bug);
	load_frogie_assets(ppu, frogie);
	frogie.Frogie_x = PPU466::ScreenWidth / 2;
	frogie.Frogie_y = PPU466::ScreenHeight / 3 - 2;
	load_background_assets(ppu);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	// Frogie sticks tongue out when pressing space
	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_SPACE) {
			frogie.start_tongue();
			//std::cout << "tongue out " << std::endl;
			return true;
		}
	}
	/*(void)evt;
	(void)window_size;*/
	return false;
}

void PlayMode::update(float elapsed) {

	cargoManager.update(elapsed);
	frogie.update(elapsed);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---
	
	//Frogie frogie;
	frogie.draw(ppu, 0);

	// Cargo boxes
	cargoManager.draw(ppu, 10);

	//--- actually draw ---
	ppu.draw(drawable_size);
}
