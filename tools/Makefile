CXXFLAGS += -g -I./src/tmxlite/include
OBJDIR = build
SRCDIR = src

TMXLITE_OBJ = build/tmxlite/src/FreeFuncs.o \
build/tmxlite/src/FreeFuncs.o \
build/tmxlite/src/ImageLayer.o \
build/tmxlite/src/LayerGroup.o \
build/tmxlite/src/Map.o \
build/tmxlite/src/miniz.o \
build/tmxlite/src/Object.o \
build/tmxlite/src/ObjectGroup.o \
build/tmxlite/src/ObjectTypes.o \
build/tmxlite/src/Property.o \
build/tmxlite/src/TileLayer.o \
build/tmxlite/src/Tileset.o \
build/tmxlite/src/detail/pugixml.o

MKMAP_OBJ = build/mkmap.o build/parser.o $(TMXLITE_OBJ)
MKOBJ_OBJ = build/mkobj.o build/parser.o $(TMXLITE_OBJ)

all: mkmap mkobj

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAGS) -std=c++20

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CXXFLAGS) -std=c11

mkmap: $(MKMAP_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $ $(LINKFLAGS)

mkobj: $(MKOBJ_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $ $(LINKFLAGS)

clean:
	rm -rf ./build ./mkmap