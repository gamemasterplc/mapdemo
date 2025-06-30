#include "object.h"


typedef struct MapObject_s {
    char *name;
    int16_t posX;
    int16_t posY;
} MapObject;

typedef struct MapObjectHeader_s {
    MapObject *obj;
    char *strtab;
    uint32_t objNum;
} MapObjectHeader;


static Object object[OBJECT_MAX];
static int objectNum;

static inline void* patch_pointer(void *ptr, uint32_t offset)
{
    return (void*)(offset + ptr);
}

void object_load(const char *path)
{
    int size;
    MapObjectHeader *objHdr = asset_load(path, &size);
    uint32_t ptrOffset = (uint32_t)(void*)objHdr;
    objHdr->obj = patch_pointer(objHdr->obj, ptrOffset);
    objHdr->strtab = patch_pointer(objHdr->strtab, ptrOffset);
    uint32_t strtabOfs = (uint32_t)(void*)objHdr->strtab;
    for(uint32_t i=0; i<objHdr->objNum; i++) {
        MapObject *obj = &objHdr->obj[i];
        if(obj->name) {
            obj->name = patch_pointer(obj->name, strtabOfs);
        }
    }
    objectNum = 0;
    for(uint32_t i=0; i<objHdr->objNum; i++) {
        MapObject *obj = &objHdr->obj[i];
        if(obj->name) {
            object[objectNum].x = obj->posX;
            object[objectNum].y = obj->posY;
            if(!strcmp(obj->name, "player")) {
                object[objectNum].sprite = sprite_load("rom:/player.sprite");
            } else if(!strcmp(obj->name, "coin")) {
                object[objectNum].sprite = sprite_load("rom:/coin.sprite");
            }
            objectNum++;
        }
    }
}

void object_draw(float scrollX, float scrollY)
{
    float dispW = display_get_width();
    float dispH = display_get_height();
    for(int i=0; i<objectNum; i++) {
        Object *obj = &object[i];
        float x = obj->x-scrollX;
        float y = obj->y-scrollY;
        sprite_t *sprite = obj->sprite;
        if(x < -sprite->width || x > dispW || y < 0 || y > dispH+sprite->height) {
            continue;
        }
        rdpq_sprite_blit(sprite, x, y, &(rdpq_blitparms_t){
            .cy = sprite->height
        });
    }
}