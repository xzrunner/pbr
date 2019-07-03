/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "pbr/DriverEnums.h"
#include "pbr/MaterialEnums.h"

#include <string>

namespace pbr
{

struct MaterialInfo;

class MaterialBuilder
{
public:
    static constexpr size_t MATERIAL_VARIABLES_COUNT = 4;
    enum class Variable : uint8_t {
        CUSTOM0,
        CUSTOM1,
        CUSTOM2,
        CUSTOM3
        // when adding more variables, make sure to update MATERIAL_VARIABLES_COUNT
    };

    static constexpr size_t MATERIAL_PROPERTIES_COUNT = 19;
    enum class Property : uint8_t {
        BASE_COLOR,              // float4, all shading models
        ROUGHNESS,               // float,  lit shading models only
        METALLIC,                // float,  all shading models, except unlit and cloth
        REFLECTANCE,             // float,  all shading models, except unlit and cloth
        AMBIENT_OCCLUSION,       // float,  lit shading models only, except subsurface and cloth
        CLEAR_COAT,              // float,  lit shading models only, except subsurface and cloth
        CLEAR_COAT_ROUGHNESS,    // float,  lit shading models only, except subsurface and cloth
        CLEAR_COAT_NORMAL,       // float,  lit shading models only, except subsurface and cloth
        ANISOTROPY,              // float,  lit shading models only, except subsurface and cloth
        ANISOTROPY_DIRECTION,    // float3, lit shading models only, except subsurface and cloth
        THICKNESS,               // float,  subsurface shading model only
        SUBSURFACE_POWER,        // float,  subsurface shading model only
        SUBSURFACE_COLOR,        // float3, subsurface and cloth shading models only
        SHEEN_COLOR,             // float3, cloth shading model only
        SPECULAR_COLOR,          // float3, specular-glossiness shading model only
        GLOSSINESS,              // float,  specular-glossiness shading model only
        EMISSIVE,                // float4, all shading models
        NORMAL,                  // float3, all shading models only, except unlit
        POST_LIGHTING_COLOR,     // float4, all shading models
        // when adding new Properties, make sure to update MATERIAL_PROPERTIES_COUNT
    };

    using PropertyList = bool[MATERIAL_PROPERTIES_COUNT];
    using VariableList = std::string[MATERIAL_VARIABLES_COUNT];

    using SamplerPrecision = Precision;

    // The methods and types below are for internal use
    struct Parameter {
        Parameter() noexcept = default;
        Parameter(const char* paramName, SamplerType t, SamplerFormat f, SamplerPrecision p)
            : name(paramName), size(1), samplerType(t), samplerFormat(f), samplerPrecision(p),
            isSampler(true) { }
        Parameter(const char* paramName, UniformType t, size_t typeSize)
            : name(paramName), size(typeSize), uniformType(t), isSampler(false) { }
        std::string name;
        size_t size;
        union {
            UniformType uniformType;
            struct {
                SamplerType samplerType;
                SamplerFormat samplerFormat;
                SamplerPrecision samplerPrecision;
            };
        };
        bool isSampler;
    };

    static constexpr size_t MAX_PARAMETERS_COUNT = 32;
    using ParameterList = Parameter[MAX_PARAMETERS_COUNT];

    enum class TargetApi : uint8_t {
        OPENGL = 0x01,
        VULKAN = 0x02,
        METAL  = 0x04,
        ALL    = OPENGL | VULKAN | METAL
    };

    enum class TargetLanguage {
        GLSL,
        SPIRV
    };

    struct CodeGenParams {
        ShaderModel    shaderModel;
        TargetApi      targetApi;
        TargetLanguage targetLanguage;
    };

public:
    MaterialBuilder();
    bool RunSemanticAnalysis() noexcept;

private:
    std::string Peek(ShaderType type, const CodeGenParams& params, 
        const PropertyList& properties) noexcept;

    void Prepare() noexcept;
    void PrepareToBuild(MaterialInfo& info) noexcept;

    // Returns true if any of the parameter samplers is of type samplerExternal
    bool hasExternalSampler() const noexcept;

    bool isLit() const noexcept { return mShading != Shading::UNLIT; }

private:
    std::string mMaterialName;

    std::string mMaterialCode;
    std::string mMaterialVertexCode;
    size_t mMaterialLineOffset = 0;
    size_t mMaterialVertexLineOffset = 0;

    PropertyList  mProperties;
    ParameterList mParameters;
    VariableList  mVariables;

    BlendingMode mBlendingMode = BlendingMode::B_OPAQUE;
    BlendingMode mPostLightingBlendingMode = BlendingMode::B_TRANSPARENT;
    CullingMode mCullingMode = CullingMode::BACK;
    Shading mShading = Shading::LIT;
    MaterialDomain mMaterialDomain = MaterialDomain::SURFACE;
    Interpolation mInterpolation = Interpolation::SMOOTH;
    VertexDomain mVertexDomain = VertexDomain::OBJECT;
    TransparencyMode mTransparencyMode = TransparencyMode::DEFAULT;

    AttributeBitset mRequiredAttributes;

    float mMaskThreshold = 0.4f;
    float mSpecularAntiAliasingVariance = 0.15f;
    float mSpecularAntiAliasingThreshold = 0.2f;

    bool mShadowMultiplier = false;

    uint8_t mParameterCount = 0;

    bool mDoubleSided = false;
    bool mDoubleSidedCapability = false;
    bool mColorWrite = true;
    bool mDepthTest = true;
    bool mDepthWrite = true;
    bool mDepthWriteSet = false;

    bool mSpecularAntiAliasing = false;
    bool mClearCoatIorChange = true;

    bool mFlipUV = true;

    bool mMultiBounceAO = false;
    bool mMultiBounceAOSet = false;
    bool mSpecularAO = false;
    bool mSpecularAOSet = false;

}; // MaterialBuilder

}