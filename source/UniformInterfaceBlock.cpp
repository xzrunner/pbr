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

#include "pbr/UniformInterfaceBlock.h"

namespace pbr
{

UniformInterfaceBlock::UniformInterfaceBlock(Builder const& builder) noexcept
    : mName(builder.mName)
{
    auto& infoMap = mInfoMap;
    auto& uniformsInfoList = mUniformsInfoList;
    infoMap.reserve(builder.mEntries.size());
    uniformsInfoList.resize(builder.mEntries.size());

    uint32_t i = 0;
    uint16_t offset = 0;
    for (auto const& e : builder.mEntries) {
        size_t alignment = baseAlignmentForType(e.type);
        uint8_t stride = strideForType(e.type);
        if (e.size > 1) { // this is an array
            // round the alignment up to that of a float4
            alignment = (alignment + 3) & ~3;
            stride = (stride + uint8_t(3)) & ~uint8_t(3);
        }

        // calculate the offset for this uniform
        size_t padding = (alignment - (offset % alignment)) % alignment;
        offset += padding;

        UniformInfo& info = uniformsInfoList[i];
        info = { e.name, offset, stride, e.type, e.size, e.precision };

        // record this uniform info
        infoMap[info.name.c_str()] = i;

        // advance offset to next slot
        offset += stride * e.size;
        ++i;
    }

    // round size to the next multiple of 4 and convert to bytes
    mSize = sizeof(uint32_t) * ((offset + 3) & ~3);
}

uint8_t UniformInterfaceBlock::baseAlignmentForType(UniformType type) noexcept
{
    switch (type)
    {
        case UniformType::BOOL:
        case UniformType::FLOAT:
        case UniformType::INT:
        case UniformType::UINT:
            return 1;
        case UniformType::BOOL2:
        case UniformType::FLOAT2:
        case UniformType::INT2:
        case UniformType::UINT2:
            return 2;
        case UniformType::BOOL3:
        case UniformType::BOOL4:
        case UniformType::FLOAT3:
        case UniformType::FLOAT4:
        case UniformType::INT3:
        case UniformType::INT4:
        case UniformType::UINT3:
        case UniformType::UINT4:
        case UniformType::MAT3:
        case UniformType::MAT4:
            return 4;
    }
}

uint8_t UniformInterfaceBlock::strideForType(UniformType type) noexcept
{
    switch (type)
    {
        case UniformType::BOOL:
        case UniformType::INT:
        case UniformType::UINT:
        case UniformType::FLOAT:
            return 1;
        case UniformType::BOOL2:
        case UniformType::INT2:
        case UniformType::UINT2:
        case UniformType::FLOAT2:
            return 2;
        case UniformType::BOOL3:
        case UniformType::INT3:
        case UniformType::UINT3:
        case UniformType::FLOAT3:
            return 3;
        case UniformType::BOOL4:
        case UniformType::INT4:
        case UniformType::UINT4:
        case UniformType::FLOAT4:
            return 4;
        case UniformType::MAT3:
            return 12;
        case UniformType::MAT4:
            return 16;
    }
}

}