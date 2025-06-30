#pragma once

#include <libdragon.h>
#include <stdint.h>

#define MAPTILE_FLIPX 0x4000
#define MAPTILE_FLIPY 0x8000

#define MAPTILE_FLIP_MASK (MAPTILE_FLIPX|MAPTILE_FLIPY)
#define MAPTILE_ID_MASK 0x3FFF

#define MAPTILE_EMPTY 0xFFFF

typedef struct MapTileset_s {
    char *imagePath;
    sprite_t *image;
    uint16_t columns;
    uint8_t tileW;
    uint8_t tileH;
} MapTileset;

typedef struct MapLayer_s {
    char *name;
    uint16_t *tile;
    uint32_t tilesetIdx;
    uint16_t layerW;
    uint16_t layerH;
} MapLayer;

typedef struct MapData_s {
    MapLayer *layer;
    MapTileset *tileset;
    char *strtab;
    uint16_t layerNum;
    uint16_t tilesetNum;
    uint16_t tileW;
    uint16_t tileH;
    uint16_t mapW;
    uint16_t mapH;
} MapData;

typedef struct MapRect_s {
    int x0;
    int y0;
    int w;
    int h;
} MapRect;

MapData *map_data_load(const char *path);
void map_data_free(MapData *map);
MapLayer *map_get_layer(MapData *map, const char *name);
void map_draw_layer(MapData *map, MapLayer *layer, int scrollX, int scrollY, MapRect *rect);
void map_draw_layer_all(MapData *map, int scrollX, int scrollY, MapRect *rect);
int map_get_width(MapData *map);
int map_get_height(MapData *map);