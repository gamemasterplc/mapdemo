BUILD_DIR=build
include $(N64_INST)/include/n64.mk

src = mapdemo.c map.c object.c

IMAGES_LIST = $(wildcard assets/*.png)
IMAGES_LIST += $(wildcard assets/tileset/*.png)

maps_tmx = $(wildcard assets/maps/*.tmx)

ASSETS_LIST = $(subst assets, filesystem,$(IMAGES_LIST:%.png=%.sprite))
ASSETS_LIST += $(subst assets, filesystem,$(maps_tmx:%.tmx=%.map))
ASSETS_LIST += $(subst assets, filesystem,$(maps_tmx:%.tmx=%.obj))

MKSPRITE_FLAGS ?=

all: mapdemo.z64

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	@$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o $(dir $@) "$<"

filesystem/%.map: assets/%.tmx
	@mkdir -p $(dir $@)
	@echo "    [MAP] $@"
	@tools/mkmap "$<" "$@"

filesystem/%.obj: assets/%.tmx
	@mkdir -p $(dir $@)
	@echo "    [OBJ] $@"
	@tools/mkobj "$<" "$@"

$(BUILD_DIR)/mapdemo.dfs: $(ASSETS_LIST)
$(BUILD_DIR)/mapdemo.elf: $(src:%.c=$(BUILD_DIR)/%.o)

mapdemo.z64: N64_ROM_TITLE="RSPQ Demo"
mapdemo.z64: $(BUILD_DIR)/mapdemo.dfs 

clean:
	rm -rf $(BUILD_DIR) mapdemo.z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
