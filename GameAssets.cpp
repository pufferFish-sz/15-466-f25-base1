#include "Frogie.hpp"
#include "AssetImporter.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <string>

void load_background_assets(PPU466& ppu) {
	try {
		// 1) load the PNG to get dimensions 
		std::string png_path = data_path("../assets/background.png");
		glm::uvec2 size{};
		std::vector<glm::u8vec4> rgba;
		load_png(png_path, &size, &rgba, LowerLeftOrigin);

		std::cout << "[PaletteTest] " << png_path
			<< "  size = " << size.x << " x " << size.y << "\n";

		// 2) run palette importer 
		const uint8_t  palette_slot = 7;
		const uint32_t dst_tile_start = 0;

		auto background_tile = AssetImporter::import_png(png_path, ppu, palette_slot, dst_tile_start);

		// Fill entire background with this tile
		for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
			for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
				ppu.background[x + PPU466::BackgroundWidth * y] = uint16_t(dst_tile_start & 0xFF) | uint16_t(palette_slot & 0x7) << 8;
			}
		}

		// 3) print the palette that was written
		const auto& pal = ppu.palette_table[palette_slot];
		auto print_rgba = [](const glm::u8vec4& c) {
			std::cout << "rgba("
				<< int(c.r) << "," << int(c.g) << "," << int(c.b) << "," << int(c.a)
				<< ")  hex=#"
				<< std::hex << std::setw(2) << std::setfill('0') << int(c.r)
				<< std::setw(2) << int(c.g)
				<< std::setw(2) << int(c.b)
				<< std::setw(2) << int(c.a)
				<< std::dec << "\n";
			};

		std::cout << "[PaletteTest] palette slot " << int(palette_slot) << ":\n";
		for (uint8_t j = 0; j < 4; ++j) {
			std::cout << "  [" << j << "] ";
			print_rgba(pal[j]);
		}

		// how many tiles were produced
		std::cout << "[TileTest] tiles written = " << background_tile.tile_count
			<< " starting at index " << background_tile.first_tile_index << "\n";
	}
	catch (const std::exception& e) {
		std::cerr << "[BackgroundPaletteTest] ERROR: " << e.what() << "\n";
	}
}

void load_frogie_assets(PPU466& ppu, Frogie& frogie) {
	
	for (uint8_t i = 0; i < 3; ++i) {
		try {
			// 1) load the PNG to get dimensions 
			std::string png_path = data_path("../assets/frogie_idle_" + std::to_string(i) + ".png");
			glm::uvec2 size{};
			std::vector<glm::u8vec4> rgba;
			load_png(png_path, &size, &rgba, LowerLeftOrigin);

			std::cout << "[PaletteTest] " << png_path
				<< "  size = " << size.x << " x " << size.y << "\n";

			// 2) run palette importer 
			const uint8_t  palette_slot = i;
			const uint32_t dst_tile_start = 16 + 4*i;

			frogie.idle_frames[i] = AssetImporter::import_png(png_path, ppu, palette_slot, dst_tile_start);

			// 3) print the palette that was written
			const auto& pal = ppu.palette_table[palette_slot];
			auto print_rgba = [](const glm::u8vec4& c) {
				std::cout << "rgba("
					<< int(c.r) << "," << int(c.g) << "," << int(c.b) << "," << int(c.a)
					<< ")  hex=#"
					<< std::hex << std::setw(2) << std::setfill('0') << int(c.r)
					<< std::setw(2) << int(c.g)
					<< std::setw(2) << int(c.b)
					<< std::setw(2) << int(c.a)
					<< std::dec << "\n";
				};

			std::cout << "[PaletteTest] palette slot " << int(palette_slot) << ":\n";
			for (uint8_t j = 0; j < 4; ++j) {
				std::cout << "  [" << j << "] ";
				print_rgba(pal[j]);
			}

			// how many tiles were produced
			std::cout << "[TileTest] tiles written = " << frogie.idle_frames[i].tile_count
				<< " starting at index " << frogie.idle_frames[i].first_tile_index << "\n";


		}
		catch (const std::exception& e) {
			std::cerr << "[PaletteTest] ERROR: " << e.what() << "\n";
		}

	}
	
	//frogie.Frogie_x = 110; frogie.Frogie_y = 100;
	//frogie.fps = 6.0f;
}