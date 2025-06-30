#include "map.h"

#define SWAP(a, b) ({ typeof(a) t = a; a = b; b = t; })

static inline void* patch_pointer(void *ptr, uint32_t offset)
{
    return (void*)(offset + ptr);
}

MapData *map_data_load(const char *path)
{
    int size;
    MapData *map = asset_load(path, &size);
    uint32_t ptrOffset = (uint32_t)(void*)map;
    map->layer = patch_pointer(map->layer, ptrOffset);
    map->tileset = patch_pointer(map->tileset, ptrOffset);
    map->strtab = patch_pointer(map->strtab, ptrOffset);
    uint32_t strtabOfs = (uint32_t)(void*)map->strtab;
    for(uint16_t i=0; i<map->tilesetNum; i++) {
        MapTileset *tset = &map->tileset[i];
        if(tset->imagePath != NULL) {
            tset->imagePath = patch_pointer(tset->imagePath, strtabOfs);
            tset->image = sprite_load(tset->imagePath);
        }
    }
    for(uint16_t i=0; i<map->layerNum; i++) {
        MapLayer *layer = &map->layer[i];
        if(layer->name != NULL) {
            layer->name = patch_pointer(layer->name, strtabOfs);
        }
        layer->tile = patch_pointer(layer->tile, ptrOffset);
    }
    return map;
}

void map_data_free(MapData *map)
{
    for(uint16_t i=0; i<map->tilesetNum; i++) {
        if(map->tileset[i].image) {
            sprite_free(map->tileset[i].image);
        }
    }
}

MapLayer *map_get_layer(MapData *map, const char *name)
{
    for(uint16_t i=0; i<map->layerNum; i++) {
        MapLayer *layer = &map->layer[i];
        if(layer->name && !strcmp(layer->name, name)) {
            return layer;
        }
    }
    return NULL;
}

void map_draw_layer(MapData *map, MapLayer *layer, int scrollX, int scrollY, MapRect *rect)
{
    if(!layer) {
        return;
    }
    MapRect dispRect;
    if(rect) {
        dispRect = *rect;
    } else {
        dispRect = (MapRect){ 0 };
        dispRect.w = display_get_width();
        dispRect.h = display_get_height();
    }
    int ofsX = -scrollX;
    int ofsY = -scrollY;
    
    int tileStartX = (scrollX < 0) ? 0 : scrollX/map->tileW;
    int tileStartY = (scrollY < 0) ? 0 : scrollY/map->tileH;
    int tileW = (dispRect.w+(map->tileW*2)-1)/map->tileW;
    int tileH = (dispRect.h+(map->tileH*2)-1)/map->tileH;
    int tileEndX = (tileStartX+tileW > layer->layerW) ? (layer->layerW) : (tileStartX+tileW);
    int tileEndY = (tileStartY+tileH > layer->layerH) ? (layer->layerH) : (tileStartY+tileH);
    
    uint16_t prevTile = MAPTILE_EMPTY;
    
    MapTileset *tileset = &map->tileset[layer->tilesetIdx];
    surface_t surface = sprite_get_pixels(tileset->image);
    rdpq_tlut_t tlutFmt = rdpq_tlut_from_format(surface_get_format(&surface));
    bool copy = true;
    int s0, t0, s1, t1;
    
    rdpq_set_mode_copy(true);
    rdpq_mode_tlut(tlutFmt);
    for(int i=tileStartY; i<tileEndY; i++) {
        for(int j=tileStartX; j<tileEndX; j++) {
            uint16_t tile = layer->tile[(i*layer->layerW)+j];
            bool flipX = (tile & MAPTILE_FLIPX) ? true : false;
            bool flipY = (tile & MAPTILE_FLIPY) ? true : false;
            if(tile == MAPTILE_EMPTY) {
                continue;
            }
            tile &= MAPTILE_ID_MASK;
            int x0 = ofsX+(j*map->tileW);
            int y0 = ofsY+(i*map->tileH);
            int x1 = x0+map->tileW;
            int y1 = y0+map->tileH;
            if(prevTile != tile) {
                s0 = (tile%tileset->columns)*tileset->tileW;
                t0 = (tile/tileset->columns)*tileset->tileH;
                s1 = s0+tileset->tileW;
                t1 = t0+tileset->tileH;
                rdpq_tex_upload_sub(TILE0, &surface, NULL, s0, t0, s1, t1);
                prevTile = tile;
            }
            if (flipX || flipY) {
              if (copy) {
                    copy = false;
                    rdpq_set_mode_standard();
                    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
                    rdpq_mode_tlut(tlutFmt);
              }
            } else if (!copy) {
                copy = true;
                rdpq_set_mode_copy(true);
                rdpq_mode_tlut(tlutFmt);
            }
            if (flipX) {
                SWAP(x0, x1);
                x0 -= 1;
                x1 -= 1;
            }
            if (flipY) {
                SWAP(y0, y1);
                y0 -= 1;
                y1 -= 1;
            }
            rdpq_texture_rectangle(TILE0, x0, y0, x1, y1, s0, t0);
        }
    }
}

void map_draw_layer_all(MapData *map, int scrollX, int scrollY, MapRect *rect)
{
    for(uint16_t i=0; i<map->layerNum; i++) {
        map_draw_layer(map, &map->layer[i], scrollX, scrollY, rect);
    }
}

int map_get_width(MapData *map)
{
    return map->mapW;
}

int map_get_height(MapData *map)
{
    return map->mapH;
}