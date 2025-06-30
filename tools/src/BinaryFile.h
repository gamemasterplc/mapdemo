/**
* @copyright 2023 - Max Bebök
* @license MIT
*/
#pragma once

#include <cstdio>
#include <unordered_map>
#include "bit.h"

class BinaryFile
{
    private:
        std::unordered_map<std::string, uint32_t> symbolMap{};
        std::unordered_map<uint32_t, std::string> relocTbl{};
        
        std::vector<uint32_t> posStack{};
        std::vector<uint8_t> data{};
        uint32_t dataPos{};
        uint32_t dataSize{};

        void writeRaw(const uint8_t* ptr, size_t size) {
            if(dataPos+size > data.size()) {
                data.resize(data.size() + size);
            }
            for(size_t i=0; i<size; ++i) {
                data[dataPos++] = ptr[i];
            }
            dataSize = std::max(dataSize, dataPos);
        }

    public:

        void skip(uint32_t bytes) {
            for(uint32_t i=0; i<bytes; ++i) {
                write<uint8_t>(0);
            }
        }

        template<typename T>
        void write(T value) {
            if constexpr (std::is_same_v<T, float>) {
                uint32_t val = Bit::byteswap(Bit::bit_cast<uint32_t>(value));
                writeRaw(reinterpret_cast<uint8_t*>(&val), sizeof(T));
            } else {
                auto val = Bit::byteswap(value);
                writeRaw(reinterpret_cast<uint8_t*>(&val), sizeof(T));
            }
        }
        
        void writeReloc(std::string symName)
        {
            relocTbl[dataPos] = symName;
            write<uint32_t>(0);
        }
        
        void setSymbol(std::string symName)
        {
            symbolMap[symName] = getPos();
        }
        
        void setSymbol(std::string symName, uint32_t addr)
        {
            symbolMap[symName] = addr;
        }
        
        void write(const std::string &str) {
            writeChars(str.c_str(), str.size());
        }

        void writeChars(const char* str, size_t len) {
            for(size_t i=0; i<len; ++i)write<uint8_t>(str[i]);
        }

        template<typename T>
        void writeArray(const T* arr, size_t count) {
            for(size_t i=0; i<count; ++i) {
                write(arr[i]);
            }
        }

        void writeMemFile(const BinaryFile& memFile) {
            writeRaw(memFile.data.data(), memFile.dataSize);
        }

        void writeChunkPointer(char type, uint32_t offset) {
            offset = offset & 0xFF'FFFF;
            offset |= (uint32_t)type << 24;
            write(offset);
        }
        
        uint32_t getPos() {
            return dataPos;
        }

        void setPos(uint32_t pos) {
            dataPos = pos;
        }

        uint32_t posPush() {
            uint32_t oldPos = getPos();
            posStack.push_back(oldPos);
            return oldPos;
        }

        uint32_t posPop() {
            uint32_t oldPos = getPos();
            setPos(posStack.back());
            posStack.pop_back();
            return oldPos;
        }

        void align(uint32_t alignment) {
            uint32_t pos = getPos();
            uint32_t offset = pos % alignment;
            if(offset != 0) {
                uint32_t padding = alignment - offset;
                for(uint32_t i=0; i<padding; ++i) {
                    write<uint8_t>(0);
                }
            }
        }

        uint32_t getSize() const {
            return dataSize;
        }

        void writeToFile(const char* filename) {
            //Fixup relocations
            for(const auto &r : relocTbl) {
                setPos(r.first);
                if(symbolMap.find(r.second) != symbolMap.end()) {
                    write<uint32_t>(symbolMap[r.second]);
                }
            }
            FILE* file = fopen(filename, "wb");
            fwrite(data.data(), 1, dataSize, file);
            fclose(file);
        }
};