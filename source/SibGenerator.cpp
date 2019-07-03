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

#include "pbr/SibGenerator.h"
#include "pbr/EngineEnums.h"
#include "pbr/SamplerInterfaceBlock.h"

#include <assert.h>

namespace pbr
{

SamplerInterfaceBlock const& SibGenerator::getPerViewSib() noexcept 
{
    // TODO: ideally we'd want this to be constexpr, this is a compile time structure
    static SamplerInterfaceBlock sib = SamplerInterfaceBlock::Builder()
            .name("Light")
            .add("shadowMap",     SamplerType::SAMPLER_2D,      SamplerFormat::SHADOW,Precision::LOW)
            .add("records",       SamplerType::SAMPLER_2D,      SamplerFormat::UINT,  Precision::MEDIUM)
            .add("froxels",       SamplerType::SAMPLER_2D,      SamplerFormat::UINT,  Precision::MEDIUM)
            .add("iblDFG",        SamplerType::SAMPLER_2D,      SamplerFormat::FLOAT, Precision::MEDIUM)
            .add("iblSpecular",   SamplerType::SAMPLER_CUBEMAP, SamplerFormat::FLOAT, Precision::MEDIUM)
            .add("ssao",          SamplerType::SAMPLER_2D,      SamplerFormat::FLOAT, Precision::MEDIUM)
            .build();

    assert(sib.getSize() == PerViewSib::SAMPLER_COUNT);

    return sib;
}

SamplerInterfaceBlock const & SibGenerator::getPostProcessSib() noexcept 
{
    // TODO: ideally we'd want this to be constexpr, this is a compile time structure
    static SamplerInterfaceBlock sib = SamplerInterfaceBlock::Builder()
            .name("PostProcess")
            .add("colorBuffer", SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, Precision::MEDIUM, false)
            .add("depthBuffer", SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, Precision::MEDIUM, false)
            .build();

    assert(sib.getSize() == PostProcessSib::SAMPLER_COUNT);

    return sib;
}

SamplerInterfaceBlock const* SibGenerator::getSib(uint8_t bindingPoint) noexcept {
    switch (bindingPoint) {
        case BindingPoints::PER_VIEW:
            return &getPerViewSib();
        case BindingPoints::PER_RENDERABLE:
            return nullptr;
        case BindingPoints::LIGHTS:
            return nullptr;
        case BindingPoints::POST_PROCESS:
            return &getPostProcessSib();
        default:
            return nullptr;
    }
}

}