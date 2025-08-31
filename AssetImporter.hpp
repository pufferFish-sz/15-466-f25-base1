#pragma once
#include "PPU466.hpp"
#include <string>

struct FileImportResult {
    std::string filename;
    uint8_t     palette_index;
    uint32_t    first_tile_index;
    uint32_t    tile_count;
};

struct AssetImporter {
    static FileImportResult import_png(
        const std::string& png_path,
        PPU466& ppu,
        uint8_t palette_slot,
        uint32_t dst_tile_offset
    );

    static PPU466::Tile pack_tile(const uint8_t tiles[8][8]);
};