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
	if (!cargoManager.stopped) {
		cargoManager.update(elapsed);
		frogie.update(elapsed);

		// if bug's x is within 16 pixels of the frog's x and the tongue is out
		// then the bug is considered eaten

		const int frog_x_line = int(frogie.Frogie_x);

		for (auto& g : cargoManager.bugs) {
			if (!g.alive) continue;

			const int bug_x = int(std::floor(g.xf));
			const int dx = frog_x_line - bug_x;  

			// only eats if tongue is out and bug is within 16 px in front
			if (0 <= dx && dx <= 16) {
				if (frogie.tongue_frame >= 3) {
					g.alive = false;              // eaten
					std::cout << "bug eaten\n";
					break;
				}
			
			}
			if (bug_x < frog_x_line && frogie.tongue_frame <= 3) {
				cargoManager.stopped = true;
				frogie.fired = true;
				std::cout << "frogie fired\n";
				break;
			}
		}
	}

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
