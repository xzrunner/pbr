/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <pbr/DriverEnums.h>

#include <string>
#include <vector>
#include <unordered_map>

#include <assert.h>

namespace pbr
{

class UniformInterfaceBlock
{
public:
    class Builder {
    public:
        Builder& name(const std::string& interfaceBlockName) {
            mName = interfaceBlockName;
            return *this;
        }

        Builder& add(const std::string& uniformName, size_t size,
            UniformType type, Precision precision = Precision::DEFAULT) {
            mEntries.emplace_back(uniformName, size, type, precision);
            return *this;
        }

        UniformInterfaceBlock build() {
            return UniformInterfaceBlock(*this);
        }

    private:
        friend class UniformInterfaceBlock;

        std::string mName;

        struct Entry
        {
            Entry(std::string name, uint32_t size, UniformType type, Precision precision) noexcept
                : name(std::move(name))
                , size(size)
                , type(type)
                , precision(precision)
            {
            }

            std::string name;
            uint32_t size;
            UniformType type;
            Precision precision;
        };
        std::vector<Entry> mEntries;

    }; // Builder

    struct UniformInfo {
        std::string name;   // name of this uniform
        uint16_t offset;    // offset in "uint32_t" of this uniform in the buffer
        uint8_t stride;     // stride in "uint32_t" to the next element
        UniformType type;   // type of this uniform
        uint32_t size;      // size of the array in elements, or 1 if not an array
        Precision precision;// precision of this uniform
        // returns offset in bytes of this uniform (at index if an array)
        inline size_t getBufferOffset(size_t index = 0) const {
            assert(index < size);
            return (offset + stride * index) * sizeof(uint32_t);
        }
    };

public:
    UniformInterfaceBlock() = default;
    UniformInterfaceBlock(const UniformInterfaceBlock& rhs) = default;
    UniformInterfaceBlock(UniformInterfaceBlock&& rhs) noexcept = default;
    UniformInterfaceBlock& operator=(const UniformInterfaceBlock& rhs) = default;
    UniformInterfaceBlock& operator=(UniformInterfaceBlock&& rhs) /*noexcept*/ = default;
    ~UniformInterfaceBlock() noexcept = default;

    // name of this uniform interface block
    auto& getName() const noexcept { return mName; }

    // size in bytes needed to store the uniforms described by this interface block in a UniformBuffer
    size_t getSize() const noexcept { return mSize; }

    // list of information records for each uniform
    std::vector<UniformInfo> const& getUniformInfoList() const noexcept { return mUniformsInfoList; }

private:
    explicit UniformInterfaceBlock(const Builder& builder) noexcept;

    static uint8_t baseAlignmentForType(UniformType type) noexcept;
    static uint8_t strideForType(UniformType type) noexcept;

private:
    std::string mName;

    std::vector<UniformInfo> mUniformsInfoList;

    std::unordered_map<std::string, uint32_t> mInfoMap;

    uint32_t mSize = 0; // size in bytes

}; // UniformInterfaceBlock

}