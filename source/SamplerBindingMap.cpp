/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "pbr/SamplerBindingMap.h"
#include "pbr/DriverEnums.h"
#include "pbr/EngineEnums.h"
#include "pbr/SibGenerator.h"
#include "pbr/SamplerInterfaceBlock.h"

#include <iostream>

namespace pbr
{

void SamplerBindingMap::populate(const SamplerInterfaceBlock* perMaterialSib,
            const char* materialName) {
    uint8_t offset = 0;
    size_t maxSamplerIndex = MAX_SAMPLER_COUNT - 1;
    bool overflow = false;
    for (uint8_t blockIndex = 0; blockIndex < BindingPoints::COUNT; blockIndex++) 
    {
        mSamplerBlockOffsets[blockIndex] = offset;
        SamplerInterfaceBlock const* sib;
        if (blockIndex == BindingPoints::PER_MATERIAL_INSTANCE) {
            sib = perMaterialSib;
        } else if (perMaterialSib && blockIndex == BindingPoints::POST_PROCESS) {
            sib = nullptr;
        } else {
            sib = SibGenerator::getSib(blockIndex);
        }
        if (sib) {
            auto sibFields = sib->getSamplerInfoList();
            for (const auto& sInfo : sibFields) {
                if (offset > maxSamplerIndex) {
                    overflow = true;
                }
                addSampler({
                    blockIndex,
                    sInfo.offset,
                    offset++,
                });
            }
        }
    }

    // If an overflow occurred, go back through and list all sampler names. This is helpful to
    // material authors who need to understand where the samplers are coming from.
    if (overflow) {
        std::cerr << "WARNING: Exceeded max sampler count of " << MAX_SAMPLER_COUNT;
        if (materialName) {
            std::cerr << " (" << materialName << ")";
        }
        std::cerr << std::endl;
        offset = 0;
        for (uint8_t blockIndex = 0; blockIndex < BindingPoints::COUNT; blockIndex++) {
            SamplerInterfaceBlock const* sib;
            if (blockIndex == BindingPoints::PER_MATERIAL_INSTANCE) {
                sib = perMaterialSib;
            } else {
                sib = SibGenerator::getSib(blockIndex);
            }
            if (sib) {
                auto sibFields = sib->getSamplerInfoList();
                for (auto sInfo : sibFields) {
                    std::cerr << "  " << offset << " " << sInfo.name.c_str() << std::endl;
                    offset++;
                }
            }
        }
    }
}

void SamplerBindingMap::addSampler(SamplerBindingInfo info) {
    if (info.globalOffset < mSamplerBlockOffsets[info.blockIndex]) {
        mSamplerBlockOffsets[info.blockIndex] = info.globalOffset;
    }
    mBindingMap[getBindingKey(info.blockIndex, info.localOffset)] = info;
}

}