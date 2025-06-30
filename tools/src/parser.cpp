#include <tmxlite/Layer.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/Tileset.hpp>

#include "parser.h"

#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

MapData parseMap(const char *tmxPath)
{
    MapData mapOut{};
    fs::path basePath{tmxPath};
    basePath = basePath.parent_path();
    
    tmx::Map map;
    if(!map.load(tmxPath)) {
        throw std::runtime_error("Failed to load map.");
    }
    tmx::FloatRect mapRect = map.getBounds();
    mapOut.mapW = mapRect.width;
    mapOut.mapH = mapRect.height;
    mapOut.tileW = map.getTileSize().x;
    mapOut.tileH = map.getTileSize().y;
    //Generate list of layer tilesets
    std::map<int, const tmx::Tileset *> layerTilesets;
    const auto& layers = map.getLayers();
    int layerIdx = 0;
    for (const auto& layer : layers)
    {
        if (layer->getType() == tmx::Layer::Type::Tile) {
            const auto& tiles = layer->getLayerAs<tmx::TileLayer>().getTiles();
            for(const auto& tile : tiles) {
                uint16_t id = tile.ID;
                if(tile.ID == 0) {
                    continue;
                }
                //Find tileset corresponding to tile ID
                int tilesetIdx = 0;
                for (const auto& tileset : map.getTilesets())  {
                    uint16_t firstGID = tileset.getFirstGID();
                    uint16_t lastGID = firstGID+tileset.getTileCount();
                    if(id >= firstGID && id < lastGID) {
                        if(layerTilesets.find(layerIdx) != layerTilesets.end()) {
                            if(layerTilesets[layerIdx]->getName() != tileset.getName()) {
                                fprintf(stderr, "Multiple tilesets found for layer %s.\n", layer->getName().c_str());
                                throw std::runtime_error("Failed to parse map layers.");
                            }
                        }
                        //Add to list of layer tilesets
                        layerTilesets[layerIdx] = &tileset;
                        break;
                    }
                    tilesetIdx++;
                }
            }
            layerIdx++;
        }
    }
    //Add tileset names
    for (auto& t : layerTilesets)
    {
        MapTileset tileset;
        std::string imagePath = t.second->getImagePath();
        tileset.imagePath = imagePath;
        tileset.columns = t.second->getColumnCount();
        tileset.tileW = t.second->getTileSize().x;
        tileset.tileH = t.second->getTileSize().y;
        mapOut.tilesets.push_back(tileset);
    }
    //Add map layers
    layerIdx = 0;
    for (const auto& layer : layers)
    {
        MapLayer mapLayer;
        mapLayer.name = layer->getName();
        if (layer->getType() == tmx::Layer::Type::Tile) {
            mapLayer.layerW = layer->getSize().x;
            mapLayer.layerH = layer->getSize().y;
            const auto& tileset = *layerTilesets[layerIdx];
            uint16_t firstGID = tileset.getFirstGID();
            uint16_t lastGID = firstGID+tileset.getTileCount();
            //Build tile data
            const auto& tiles = layer->getLayerAs<tmx::TileLayer>().getTiles();
            for(const auto& tile : tiles) {
                uint16_t id = tile.ID;
                if(tile.ID == 0) {
                    mapLayer.tileData.push_back(UINT16_MAX);
                    continue;
                }
                id -= firstGID; //Make ID tileset-relative
                //Add flip flags
                if(tile.flipFlags & tmx::TileLayer::FlipFlag::Horizontal) {
                    id |= 0x4000;
                }
                if(tile.flipFlags & tmx::TileLayer::FlipFlag::Vertical) {
                    id |= 0x8000;
                }
                mapLayer.tileData.push_back(id);
            }
            mapOut.layers.push_back(mapLayer);
            layerIdx++;
        }
    }
    
    return mapOut;
}

static const tmx::Tileset::Tile *SearchTileGid(tmx::Map &map, uint32_t tileId)
{
    for (const auto& tileset : map.getTilesets())  {
        const tmx::Tileset::Tile *tile = tileset.getTile(tileId);
        if(tile) {
            return tile;
        }
    }
    return NULL;
}

ObjectData parseObject(const char *tmxPath)
{
    ObjectData objOut{};
    tmx::Map map;
    if(!map.load(tmxPath)) {
        throw std::runtime_error("Failed to load map.");
    }
    const auto& layers = map.getLayers();
    for (const auto& layer : layers)
    {
        if (layer->getType() == tmx::Layer::Type::Object) {
            const auto& objects = layer->getLayerAs<tmx::ObjectGroup>().getObjects();
            for(const auto& object : objects)
            {
                Object obj;
                const tmx::Tileset::Tile *tile = SearchTileGid(map, object.getTileID());
                std::string name = object.getClass();
                if(name.empty()) {
                    name = tile->className;
                }
                obj.name = name;
                obj.posX = object.getPosition().x;
                obj.posY = object.getPosition().y;
                objOut.objects.push_back(obj);
            }
        }
    }
    return objOut;
}