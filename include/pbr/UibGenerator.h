#pragma once

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

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace pbr
{

class UniformInterfaceBlock;

class UibGenerator {
public:
    static UniformInterfaceBlock const& getPerViewUib() noexcept;
    static UniformInterfaceBlock const& getPerRenderableUib() noexcept;
    static UniformInterfaceBlock const& getLightsUib() noexcept;
    static UniformInterfaceBlock const& getPostProcessingUib() noexcept;
    static UniformInterfaceBlock const& getPerRenderableBonesUib() noexcept;
};

/*
 * These structures are only used to call offsetof() and make it easy to visualize the UBO.
 *
 * IMPORTANT NOTE: Respect std140 layout, don't update without updating getUib()
 */

struct PerViewUib { // NOLINT(cppcoreguidelines-pro-type-member-init)
    static const UniformInterfaceBlock& getUib() noexcept {
        return UibGenerator::getPerViewUib();
    }
    glm::mat4x4 viewFromWorldMatrix;
    glm::mat4x4 worldFromViewMatrix;
    glm::mat4x4 clipFromViewMatrix;
    glm::mat4x4 viewFromClipMatrix;
    glm::mat4x4 clipFromWorldMatrix;
    glm::mat4x4 worldFromClipMatrix;
    glm::mat4x4 lightFromWorldMatrix;

    glm::vec4 resolution; // viewport width, height, 1/width, 1/height

    glm::vec3 cameraPosition;
    float time; // time in seconds, with a 1 second period

    glm::vec4 lightColorIntensity; // directional light

    glm::vec4 sun; // cos(sunAngle), sin(sunAngle), 1/(sunAngle*HALO_SIZE-sunAngle), HALO_EXP

    glm::vec3 lightDirection;
    uint32_t fParamsX; // stride-x

    glm::vec3 shadowBias; // unused, normal bias, unused
    float oneOverFroxelDimensionY;

    glm::vec4 zParams; // froxel Z parameters

    glm::ivec2 fParams; // stride-y, stride-z
    glm::vec2 origin; // viewport left, viewport bottom

    float oneOverFroxelDimensionX;
    float iblLuminance;
    float exposure;
    float ev100;

    alignas(16) glm::vec4 iblSH[9]; // actually float3 entries (std140 requires float4 alignment)

    glm::vec4 userTime;  // time(s), (double)time - (float)time, 0, 0

    glm::vec2 iblMaxMipLevel; // maxlevel, float(1<<maxlevel)
    glm::vec2 padding0;

    // bring PerViewUib to 1 KiB
    glm::vec4 padding1[16];
};


// PerRenderableUib must have an alignment of 256 to be compatible with all versions of GLES.
struct alignas(256) PerRenderableUib {
    glm::mat4x4 worldFromModelMatrix;
    glm::mat3x3 worldFromModelNormalMatrix;
};

struct LightsUib {
    static const UniformInterfaceBlock& getUib() noexcept {
        return UibGenerator::getLightsUib();
    }
    glm::vec4 positionFalloff;   // { float3(pos), 1/falloff^2 }
    glm::vec4 colorIntensity;    // { float3(col), intensity }
    glm::vec4 directionIES;      // { float3(dir), IES index }
    glm::vec4 spotScaleOffset;   // { scale, offset, unused, unused }
};

struct PostProcessingUib {
    static const UniformInterfaceBlock& getUib() noexcept {
        return UibGenerator::getPostProcessingUib();
    }
    glm::vec2 uvScale;
    float time;             // time in seconds, with a 1 second period, used for dithering
    float yOffset;
    int dithering;          // type of dithering 0=none, 1=enabled
};

// This is not the UBO proper, but just an element of a bone array.
struct PerRenderableUibBone {
    glm::quat q = { 1, 0, 0, 0 };
    glm::vec4 t = {};
    glm::vec4 s = { 1, 1, 1, 0 };
    glm::vec4 ns = { 1, 1, 1, 0 };
};

}