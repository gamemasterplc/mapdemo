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
        printf("Usage: %s <tmx-file> <obj-file>\n", argv[0]);
        return 1;
    }
    const std::string tmxPath = args.getFilenameArg(0);
    const std::string objPath = args.getFilenameArg(1);
    ObjectData objData = parseObject(tmxPath.c_str());
    BinaryFile outFile;

    std::string stringTable = "S";
    stringTable += '\0';
    outFile.writeReloc("objData"); //Layer Pointer
    outFile.writeReloc("stringTable"); //String Table Pointer
    outFile.write<uint32_t>(objData.objects.size()); //Number of objects
    outFile.align(4);
    outFile.setSymbol("objData");
    for(const auto &obj : objData.objects) {
        outFile.write(insertString(stringTable, obj.name));
        outFile.write(obj.posX);
        outFile.write(obj.posY);
    }
    outFile.setSymbol("stringTable");
    outFile.write(stringTable);
    outFile.writeToFile(objPath.c_str());
}