#pragma once

#include <vector>
#include <string>
#include <stdint.h>

struct MapLayer {
    std::vector<uint16_t> tileData{};
    std::string name{};
    uint32_t tilesetIdx{};
    uint16_t layerW{};
    uint16_t layerH{};
};

struct MapTileset {
    std::string imagePath{};
    uint16_t columns{};
    uint8_t tileW{};
    uint8_t tileH{};
};

struct MapData {
    std::vector<MapLayer> layers{};
    std::vector<MapTileset> tilesets{};
    uint16_t tileW{};
    uint16_t tileH{};
    uint16_t mapW{};
    uint16_t mapH{};
};

struct Object {
    std::string name{};
    int16_t posX{};
    int16_t posY{};
};

struct ObjectData {
    std::vector<Object> objects{};
};

struct Config {
    std::string assetPath;
};

extern Config config;