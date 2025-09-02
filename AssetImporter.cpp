#include "AssetImporter.hpp"
#include "load_save_png.hpp"     // needed for load_png + OriginLocation
#include <glm/glm.hpp>
#include <iostream>
#include <stdexcept>
#include <set>
#include <vector>
#include <array>
#include <cstdint>
#include "data_path.hpp"

using glm::uvec2;
using glm::u8vec4;

// Keep y=0 at the bottom so rows match PPU bitplane convention:
static constexpr OriginLocation ORIGIN = LowerLeftOrigin;


static bool load_png_rgba(const std::string& path, uvec2* size, std::vector<u8vec4>* rgba) {
    try {
        load_png(path, size, rgba, ORIGIN);
        return true;
    } catch (const std::runtime_error& e) {
        std::cerr << "[Importer] PNG load failed: " << e.what() << "\n";
        return false;
    }
}

static bool size_multiples_of_8(const uvec2& size) {
    return (size.x % 8 == 0) && (size.y % 8 == 0);
}

// build NES-style palette: index 0 transparent, 1..3 = opaque (≤3)
// throws error if >3 opaque colors in the whole file.
static void build_palette(
    const std::string& png_path,
    const std::vector<u8vec4>& rgba,
    std::array<u8vec4,4>* out_palette
) {
    auto key32 = [](const glm::u8vec4& c) -> uint32_t {
        return (uint32_t(c.r) << 24) | (uint32_t(c.g) << 16) | (uint32_t(c.b) << 8) | uint32_t(c.a);
    };

    std::set<uint32_t> opaque;  // distinct opaque colors (exact RGBA)
    bool has_transparent = false;

    for (auto const& px : rgba) {
        if (px.a == 0) has_transparent = true; 
        else           opaque.insert(key32(px));
    }

    if (opaque.size() > 3) {
        throw std::runtime_error(
            "[Importer] '" + png_path + "': uses > 3 opaque colors (found "
            + std::to_string(opaque.size()) + "). Limit is 3 opaque + transparent."
        );
    }

    // fill palette: [0] transparent; [1..3] opaque
    out_palette->fill(u8vec4(0,0,0,0));
    size_t cursor = 1;
    for (uint32_t k : opaque) {
        if (cursor >= 4) break;
        u8vec4 c;
        c.r = (k >> 24) & 0xFF;
        c.g = (k >> 16) & 0xFF;
        c.b = (k >>  8) & 0xFF;
        c.a = (k      ) & 0xFF;
        (*out_palette)[cursor++] = c;
    }
}

// map RGBA pixel to the palette index of 0 - 3
static uint8_t rgba_to_index(const glm::u8vec4& px, const std::array<glm::u8vec4, 4>& pal) {
    if (px.a == 0) return 0;
    for (uint8_t i = 1; i < 4; ++i) {
        if ((pal[i].r == px.r) && (pal[i].g == px.g) && (pal[i].b == px.b) && (pal[i].a == px.a)) return i;
    }
    throw std::runtime_error("[Importer] pixel color not found on palette");
}

static uint32_t slice_png_to_tiles(
    const std::vector<u8vec4>& rgba, 
    uvec2 size,
    const std::array<u8vec4, 4>& palette, 
    PPU466& ppu,
    uint32_t dst_tile_offset
) {
    
    const uint32_t tiles_x = size.x / 8u;
    const uint32_t tiles_y = size.y / 8u;
    const uint32_t total_tiles = tiles_x * tiles_y;
    const uint32_t max_tiles = static_cast<uint32_t>(ppu.tile_table.size());

    if (dst_tile_offset >= max_tiles) {
        throw std::runtime_error("[Importer] offset beyond tile table");
    }

    if (dst_tile_offset + total_tiles >= max_tiles) {
        throw std::runtime_error("[Importer] Too many tiles, tile table can't hold");
    }

    uint32_t write_index = dst_tile_offset;

    for (uint32_t ty = 0; ty < tiles_y; ++ty) {
        for (uint32_t tx = 0; tx < tiles_x; ++tx) {
            uint8_t indices[8][8];
            for (uint32_t py = 0; py < 8; ++py) {
                for (uint32_t px = 0; px < 8; ++px) {
                    const uint32_t ix = tx * 8 + px;
                    const uint32_t iy = ty * 8 + py;
                    const u8vec4& px_rgba = rgba[iy * size.x + ix];
                    indices[py][px] = rgba_to_index(px_rgba, palette);
                }
            }

            const PPU466::Tile t = AssetImporter::pack_tile(indices);
            ppu.tile_table[write_index] = t;

            // --- Debug print: first 2 rows of bitplanes for first few tiles ---
            if (write_index < dst_tile_offset + 4) { // only first 4 tiles
                std::cout << "[Importer] Tile " << write_index
                    << " bit0[0..1]=" << std::hex << int(t.bit0[0]) << ","
                    << int(t.bit0[1])
                    << " bit1[0..1]=" << int(t.bit1[0]) << ","
                    << int(t.bit1[1]) << std::dec << "\n";
            }
            write_index++;
        }
    }
    return total_tiles;
}

FileImportResult AssetImporter::import_png(
    const std::string& png_path,
    PPU466& ppu,
    uint8_t palette_slot,
    uint32_t dst_tile_offset
) {
    if (palette_slot >= ppu.palette_table.size()) {
        throw std::runtime_error("[Importer] no palette slots left for " + png_path);
    }

    uvec2 size{};
    std::vector<u8vec4> rgba;
    if (!load_png_rgba(png_path, &size, &rgba)) {
        throw std::runtime_error("[Importer] failed to load " + png_path);
    }

    if (!size_multiples_of_8(size)) {
        throw std::runtime_error(
            "[Importer] '" + png_path + "' size must be multiples of 8 (got "
            + std::to_string(size.x) + "x" + std::to_string(size.y) + ")"
        ); 
    }

    // build palette from the whole png and write into the PPU slot:
    std::array<u8vec4,4> pal{};
    build_palette(png_path, rgba, &pal);
    for (int i = 0; i < 4; ++i) {
        ppu.palette_table[palette_slot][i] = pal[i];
    }

    // Slice the png into 8x8 tiles
    uint32_t num_tiles = slice_png_to_tiles(rgba, size, pal, ppu, dst_tile_offset);

    // Print out the info regarding this import:
    FileImportResult r{};
    r.filename         = png_path;        
    r.palette_index    = palette_slot;
    r.first_tile_index = dst_tile_offset;
    r.tile_count       = num_tiles;   


    return r;
}

static void verify_colors_in_palette(
    const std::string& png_path,
    const std::vector<glm::u8vec4>& rgba,
    const std::array<glm::u8vec4, 4>& pal
) {
    auto same = [](const glm::u8vec4& a, const glm::u8vec4& b) {
        return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
        };
    for (auto const& px : rgba) {
        if (px.a == 0) continue; // transparent is index 0 by convention
        bool ok = false;
        for (uint8_t i = 1; i < 4; ++i) {
            if (same(px, pal[i])) { ok = true; break; }
        }
        if (!ok) {
            throw std::runtime_error(
                "[Importer] '" + png_path + "': color "
                + std::to_string(px.r) + "," + std::to_string(px.g) + ","
                + std::to_string(px.b) + "," + std::to_string(px.a)
                + " not in fixed palette."
            );
        }
    }
}

FileImportResult AssetImporter::import_png_with_fixed_palette(
    const std::string& png_path,
    PPU466& ppu,
    uint8_t palette_slot,
    uint32_t dst_tile_offset,
    const std::array<glm::u8vec4, 4>& fixed_palette
) {
    if (palette_slot >= ppu.palette_table.size()) {
        throw std::runtime_error("[Importer] no palette slots left for " + png_path);
    }

    uvec2 size{};
    std::vector<u8vec4> rgba;
    if (!load_png_rgba(png_path, &size, &rgba)) {
        throw std::runtime_error("[Importer] failed to load " + png_path);
    }

    if (!size_multiples_of_8(size)) {
        throw std::runtime_error(
            "[Importer] '" + png_path + "' size must be multiples of 8 (got "
            + std::to_string(size.x) + "x" + std::to_string(size.y) + ")"
        );
    }

    // Ensure every opaque pixel is one of the 3 colors in fixed palette:
    verify_colors_in_palette(png_path, rgba, fixed_palette);

    // Pack using the fixed palette (indices 0..3 come from 'fixed_palette'):
    uint32_t num_tiles = slice_png_to_tiles(rgba, size, fixed_palette, ppu, dst_tile_offset);

    FileImportResult r{};
    r.filename = png_path;
    r.palette_index = palette_slot;    // same slot every frame
    r.first_tile_index = dst_tile_offset;
    r.tile_count = num_tiles;
    return r;
}


PPU466::Tile AssetImporter::pack_tile(const uint8_t tiles[8][8]) {
    PPU466::Tile t{};
    for (int y = 0; y < 8; ++y) {
        uint8_t r0 = 0, r1 = 0;
        for (int x = 0; x < 8; ++x) {
            uint8_t v = tiles[y][x] & 0x3;
            r0 |= ((v & 0x1) ? (1u << x) : 0);
            r1 |= ((v & 0x2) ? (1u << x) : 0);
        }
        t.bit0[y] = r0;
        t.bit1[y] = r1;
    }
    return t;
}