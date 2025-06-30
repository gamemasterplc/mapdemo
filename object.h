#pragma once

#include <libdragon.h>

#define OBJECT_MAX 64

typedef struct Object_s {
    float x;
    float y;
    sprite_t *sprite;
} Object;

void object_load(const char *path);
void object_draw(float scrollX, float scrollY);
