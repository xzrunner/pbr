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

#include "pbr/UibGenerator.h"
#include "pbr/UniformInterfaceBlock.h"
#include "pbr/EngineEnums.h"

namespace pbr
{

static_assert(sizeof(PerRenderableUib) % 256 == 0,
        "sizeof(Transform) should be a multiple of 256");

static_assert(CONFIG_MAX_BONE_COUNT * sizeof(PerRenderableUibBone) <= 16384,
        "Bones exceed max UBO size");


UniformInterfaceBlock const& UibGenerator::getPerViewUib() noexcept  {
    // IMPORTANT NOTE: Respect std140 layout, don't update without updating Engine::PerViewUib
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name("FrameUniforms")
            // transforms
            .add("viewFromWorldMatrix",     1, UniformType::MAT4, Precision::HIGH)
            .add("worldFromViewMatrix",     1, UniformType::MAT4, Precision::HIGH)
            .add("clipFromViewMatrix",      1, UniformType::MAT4, Precision::HIGH)
            .add("viewFromClipMatrix",      1, UniformType::MAT4, Precision::HIGH)
            .add("clipFromWorldMatrix",     1, UniformType::MAT4, Precision::HIGH)
            .add("worldFromClipMatrix",     1, UniformType::MAT4, Precision::HIGH)
            .add("lightFromWorldMatrix",    1, UniformType::MAT4, Precision::HIGH)
            // view
            .add("resolution",              1, UniformType::FLOAT4, Precision::HIGH)
            // camera
            .add("cameraPosition",          1, UniformType::FLOAT3, Precision::HIGH)
            // time
            .add("time",                    1, UniformType::FLOAT, Precision::HIGH)
            // directional light
            .add("lightColorIntensity",     1, UniformType::FLOAT4)
            .add("sun",                     1, UniformType::FLOAT4)
            .add("lightDirection",          1, UniformType::FLOAT3)
            .add("fParamsX",                1, UniformType::UINT)
            // shadow
            .add("shadowBias",              1, UniformType::FLOAT3)
            .add("oneOverFroxelDimensionY", 1, UniformType::FLOAT)
            // froxels
            .add("zParams",                 1, UniformType::FLOAT4)
            .add("fParams",                 1, UniformType::UINT2)
            .add("origin",                  1, UniformType::FLOAT2)
            // froxels (again, for alignment purposes)
            .add("oneOverFroxelDimension",  1, UniformType::FLOAT)
            // ibl
            .add("iblLuminance",            1, UniformType::FLOAT)
            // camera
            .add("exposure",                1, UniformType::FLOAT)
            .add("ev100",                   1, UniformType::FLOAT)
            // ibl
            .add("iblSH",                   9, UniformType::FLOAT3)
            // user time
            .add("userTime",                1, UniformType::FLOAT4)
            // ibl max mip level
            .add("iblMaxMipLevel",          1, UniformType::FLOAT2)
            .add("padding10",               1, UniformType::FLOAT2)
            // bring size to 1 KiB
            .add("padding1",                16, UniformType::FLOAT4)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableUib() noexcept {
    static UniformInterfaceBlock uib =  UniformInterfaceBlock::Builder()
            .name("ObjectUniforms")
            .add("worldFromModelMatrix",       1, UniformType::MAT4, Precision::HIGH)
            .add("worldFromModelNormalMatrix", 1, UniformType::MAT3, Precision::HIGH)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getLightsUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name("LightsUniforms")
            .add("lights", CONFIG_MAX_LIGHT_COUNT, UniformType::MAT4, Precision::HIGH)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPostProcessingUib() noexcept {
    static UniformInterfaceBlock uib =  UniformInterfaceBlock::Builder()
            .name("PostProcessUniforms")
            .add("uvScale",   1, UniformType::FLOAT2)
            .add("time",      1, UniformType::FLOAT)
            .add("yOffset",   1, UniformType::FLOAT)
            .add("dithering", 1, UniformType::INT)
            .build();
    return uib;
}

UniformInterfaceBlock const& UibGenerator::getPerRenderableBonesUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name("BonesUniforms")
            .add("bones", CONFIG_MAX_BONE_COUNT * 4, UniformType::FLOAT4, Precision::MEDIUM)
            .build();
    return uib;
}

}