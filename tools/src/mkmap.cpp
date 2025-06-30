#include <tmxlite/Map.hpp>
#include <stdio.h>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cassert>
#include "structs.h"
#include "args.h"
#include "parser.h"
#include "BinaryFile.h"

namespace fs = std::filesystem;

Config config;

uint32_t insertString(std::string &stringTable, std::string newString) {
    newString += '\0';
    auto strPos = stringTable.find(newString);
    if(strPos == std::string::npos) {
        strPos = stringTable.size();
        stringTable += newString;
    }
    return strPos;
}

int main(int argc, char **argv)
{
    EnvArgs args{argc, argv};
    if(argc < 3 || args.checkArg("--help")) {
        printf("Usage: %s <tmx-file> <map-file> [--asset-path=assets]\n", argv[0]);
        return 1;
    }
    const std::string tmxPath = args.getFilenameArg(0);
    const std::string mapPath = args.getFilenameArg(1);
    config.assetPath = args.getStringArg("--asset-path");
    if(config.assetPath.empty()) {
        config.assetPath = "assets/";
    }
    MapData mapData = parseMap(tmxPath.c_str());
    fs::path basePath{tmxPath};
    basePath = basePath.parent_path();
    BinaryFile outFile;
    
    
    std::string stringTable = "S";
    stringTable += '\0';
    
    outFile.writeReloc("layerData"); //Layer Pointer
    outFile.writeReloc("tilesetData"); //Tileset Pointer
    outFile.writeReloc("stringTable"); //String Table Pointer
    outFile.write<uint16_t>(mapData.layers.size()); //Number of layers
    outFile.write<uint16_t>(mapData.tilesets.size()); //Number of tilesets
    outFile.write(mapData.tileW);
    outFile.write(mapData.tileH);
    outFile.write(mapData.mapW);
    outFile.write(mapData.mapH);
    outFile.align(4);
    outFile.setSymbol("layerData");
    size_t layerNum = 0;
    for(const auto &layer : mapData.layers) {
        outFile.write(insertString(stringTable, layer.name));
        std::string tileSymName = "layerTile" + std::to_string(layerNum);
        outFile.writeReloc(tileSymName);
        outFile.write(layer.tilesetIdx);
        outFile.write(layer.layerW);
        outFile.write(layer.layerH);
        layerNum++;
    }
    layerNum = 0;
    for(const auto &layer : mapData.layers) {
        outFile.align(2);
        std::string tileSymName = "layerTile" + std::to_string(layerNum);
        outFile.setSymbol(tileSymName);
        outFile.writeArray(layer.tileData.data(), layer.layerW*layer.layerH);
        layerNum++;
    }
    outFile.setSymbol("tilesetData");
    for(const auto &tileset : mapData.tilesets) {
        std::string path = tileset.imagePath;
        if(!path.empty()) {
            path = fs::relative(path, std::filesystem::current_path()).string();
            std::replace(path.begin(), path.end(), '\\', '/');
            if(path.find(config.assetPath) == 0) {
                path.replace(0, config.assetPath.size(), "rom:/");
            }
            if(path.find(".png") != std::string::npos) {
                path.replace(path.find(".png"), 4, ".sprite");
            }
        }
        if(!path.empty()) {
            outFile.write(insertString(stringTable, path));
            outFile.write<uint32_t>(0);
        } else {
            outFile.write<uint32_t>(0);
            outFile.write<uint32_t>(0);
        }
        outFile.write(tileset.columns);
        outFile.write(tileset.tileW);
        outFile.write(tileset.tileH);
    }
    outFile.setSymbol("stringTable");
    outFile.write(stringTable);
    outFile.writeToFile(mapPath.c_str());
}