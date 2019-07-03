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

namespace pbr
{

class SamplerInterfaceBlock
{
public:
    class Builder
    {
    public:
        Builder() = default;
        Builder(Builder const&) = default;
        ~Builder() noexcept = default;

        Builder& name(const std::string& interfaceBlockName) {
            mName = interfaceBlockName;
            return *this;
        }

        Builder& add(const std::string& samplerName, SamplerType type, SamplerFormat format,
            Precision precision = Precision::MEDIUM,
            bool multisample = false) noexcept
        {
            mEntries.emplace_back(samplerName, type, format, precision, multisample);
            return *this;
        }

        SamplerInterfaceBlock build() {
            return SamplerInterfaceBlock(*this);
        }

    private:
        friend class SamplerInterfaceBlock;

        std::string mName;

        struct Entry
        {
            Entry(std::string name, SamplerType type, SamplerFormat format, Precision precision, bool multisample) noexcept
                : name(std::move(name))
                , type(type)
                , format(format)
                , multisample(multisample)
                , precision(precision)
            {
            }

            std::string name;
            SamplerType   type;
            SamplerFormat format;
            bool multisample;
            Precision precision;
        };
        std::vector<Entry> mEntries;

    }; // Builder

    struct SamplerInfo { // NOLINT(cppcoreguidelines-pro-type-member-init)
        SamplerInfo() noexcept = default;
        SamplerInfo(std::string name, uint8_t offset, SamplerType type,
            SamplerFormat format, Precision precision, bool multisample) noexcept
            : name(std::move(name))
            , offset(offset)
            , type(type)
            , format(format)
            , multisample(multisample)
            , precision(precision)
        {
        }

        std::string name;       // name of this sampler
        uint8_t offset;         // offset in "Sampler" of this sampler in the buffer
        SamplerType type;       // type of this sampler
        SamplerFormat format;   // format of this sampler
        bool multisample;       // multisample capable
        Precision precision;    // precision of this sampler
    };

public:
    SamplerInterfaceBlock() = default;
    SamplerInterfaceBlock(const SamplerInterfaceBlock& rhs) = default;
    SamplerInterfaceBlock(SamplerInterfaceBlock&& rhs) noexcept = default;
    SamplerInterfaceBlock& operator=(const SamplerInterfaceBlock& rhs) = default;
    SamplerInterfaceBlock& operator=(SamplerInterfaceBlock&& rhs) /*noexcept*/ = default;
    ~SamplerInterfaceBlock() noexcept = default;

    // name of this sampler interface block
    const std::string& getName() const noexcept { return mName; }

    // size needed to store the samplers described by this interface block in a SamplerGroup
    size_t getSize() const noexcept { return mSize; }

    // list of information records for each sampler
    std::vector<SamplerInfo> const& getSamplerInfoList() const noexcept { return mSamplersInfoList; }

    static std::string getUniformName(const char* group, const char* sampler) noexcept;

private:
    explicit SamplerInterfaceBlock(const Builder& builder) noexcept;

private:
    std::string mName;

    std::vector<SamplerInfo> mSamplersInfoList;

    std::unordered_map<std::string, uint32_t> mInfoMap;

    uint32_t mSize = 0; // size in Samplers (i.e.: count)

}; // SamplerInterfaceBlock

}