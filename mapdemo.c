#include "libdragon.h"
#include <malloc.h>
#include <math.h>
#include "map.h"
#include "object.h"

static MapData *map;
static sprite_t *bg_tile;
static int scroll_x, scroll_y;
static rdpq_font_t *font;

void render(int cur_frame)
{
    // Attach and clear the screen
    surface_t *disp = display_get();
    rdpq_attach_clear(disp, NULL);
    
    rdpq_set_mode_standard();
    int bgW = bg_tile->width;
    int bgH = bg_tile->height;
    
    //Get screen size
    float scrWidth = display_get_width();
    float scrHeight = display_get_height();
    //Calculate tile screen size
    float tileW = bg_tile->width;
    float tileH = bg_tile->height;
    //Calculate number of visible tiles (+2 is for potentially offscreen tiles)
    int tileNumX = (scrWidth/tileW)+2;
    int tileNumY = (scrHeight/tileH)+2;
    
    int ofsX = -((scroll_x/3)%bgW);
    int ofsY = -((scroll_y/3)%bgH);
    if(ofsX > 0) {
        ofsX -= bgW;
    }
    if(ofsY > 0) {
        ofsY -= bgH;
    }
    //Iterate over visible tiles
    for(int i=0; i<tileNumY; i++) {
        for(int j=0; j<tileNumX; j++) {
            rdpq_sprite_blit(bg_tile, ofsX+(j*tileW), ofsY+(i*tileH), NULL);
        }
    }
    map_draw_layer_all(map, scroll_x, scroll_y, NULL);
    object_draw(scroll_x, scroll_y);
    rdpq_text_printf(NULL, 1, 22, 22, "FPS %d", (int)(display_get_fps()+0.5f));
    rdpq_detach_show();
}

int main()
{
    debug_init_isviewer();
    debug_init_usblog();

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

    joypad_init();
    timer_init();

    dfs_init(DFS_DEFAULT_LOCATION);

    rdpq_init();
    rdpq_debug_start();

    map = map_data_load("rom:/maps/testmap.map");
    object_load("rom:/maps/testmap.obj");
    bg_tile = sprite_load("rom:/bg_tile.sprite");
    font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(1, font);

    int cur_frame = 0;
    while (1)
    {
        render(cur_frame);

        joypad_poll();
        joypad_buttons_t ckeys = joypad_get_buttons_held(JOYPAD_PORT_1);
        if(ckeys.c_up) {
            if(scroll_y > 0) {
                scroll_y--;
            }
        }
        if(ckeys.c_down) {
            if(scroll_y < map_get_height(map)-display_get_height()) {
                scroll_y++;
            }
            
        }
        if(ckeys.c_left) {
            if(scroll_x > 0) {
                scroll_x--;
            }
            
        }
        if(ckeys.c_right) {
            if(scroll_x < map_get_width(map)-display_get_width()) {
                scroll_x++;
            }
        }
    }
}
