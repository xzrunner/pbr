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

#include "pbr/MaterialBuilder.h"
#include "pbr/GLSLTools.h"
#include "pbr/ShaderGenerator.h"
#include "pbr/MaterialInfo.h"
#include "pbr/DriverEnums.h"
#include "pbr/MaterialInfo.h"

namespace pbr
{

MaterialBuilder::MaterialBuilder()
{
    std::fill_n(mProperties, MATERIAL_PROPERTIES_COUNT, false);
}

bool MaterialBuilder::RunSemanticAnalysis() noexcept
{
    GLSLTools glslTools;

    CodeGenParams params{ ShaderModel::GL_ES_30, TargetApi::OPENGL, TargetLanguage::GLSL };

    ShaderModel model = ShaderModel::GL_ES_30;
    std::string shaderCode = Peek(ShaderType::VERTEX, params, mProperties);
    printf("++++++++ vs ++++++++\n%s\n", shaderCode.c_str());
    bool result = glslTools.AnalyzeVertexShader(shaderCode, model, TargetApi::OPENGL);
    if (!result) return false;

    shaderCode = Peek(ShaderType::FRAGMENT, params, mProperties);
    printf("++++++++ fs ++++++++\n%s\n", shaderCode.c_str());
    result = glslTools.AnalyzeFragmentShader(shaderCode, model, TargetApi::OPENGL);
    return result;
}

std::string MaterialBuilder::Peek(ShaderType type, const CodeGenParams& params,
                                  const PropertyList& properties) noexcept
{
    ShaderGenerator sg(properties, mVariables,
            mMaterialCode, mMaterialLineOffset, mMaterialVertexCode, mMaterialVertexLineOffset);

    MaterialInfo info;
    PrepareToBuild(info);

    SamplerBindingMap map;
    map.populate(&info.sib, mMaterialName.c_str());
    info.samplerBindings = std::move(map);

    if (type == ShaderType::VERTEX) {
        return sg.createVertexProgram(ShaderModel(params.shaderModel),
                params.targetApi, params.targetLanguage, info, 0, mInterpolation, mVertexDomain);
    } else {
        return sg.createFragmentProgram(ShaderModel(params.shaderModel), params.targetApi,
                params.targetLanguage, info, 0, mInterpolation);
    }

    return std::string("");
}

void MaterialBuilder::PrepareToBuild(MaterialInfo& info) noexcept
{
    // Build the per-material sampler block and uniform block.
    SamplerInterfaceBlock::Builder sbb;
    UniformInterfaceBlock::Builder ibb;
    for (size_t i = 0, c = mParameterCount; i < c; i++) {
        auto const& param = mParameters[i];
        if (param.isSampler) {
            sbb.add(param.name, param.samplerType, param.samplerFormat, param.samplerPrecision);
        } else {
            ibb.add(param.name, param.size, param.uniformType);
        }
    }

    if (mSpecularAntiAliasing) {
        ibb.add("_specularAntiAliasingVariance", 1, UniformType::FLOAT);
        ibb.add("_specularAntiAliasingThreshold", 1, UniformType::FLOAT);
    }

    if (mBlendingMode == BlendingMode::MASKED) {
        ibb.add("_maskThreshold", 1, UniformType::FLOAT);
    }

    if (mDoubleSidedCapability) {
        ibb.add("_doubleSided", 1, UniformType::BOOL);
    }

    mRequiredAttributes.set(static_cast<int>(VertexAttribute::POSITION));
    if (mShading != Shading::UNLIT || mShadowMultiplier) {
        mRequiredAttributes.set(static_cast<int>(VertexAttribute::TANGENTS));
    }

    info.sib = sbb.name("MaterialParams").build();
    info.uib = ibb.name("MaterialParams").build();

    info.isLit = isLit();
    info.hasDoubleSidedCapability = mDoubleSidedCapability;
    info.hasExternalSamplers = hasExternalSampler();
    info.specularAntiAliasing = mSpecularAntiAliasing;
    info.clearCoatIorChange = mClearCoatIorChange;
    info.flipUV = mFlipUV;
    info.requiredAttributes = mRequiredAttributes;
    info.blendingMode = mBlendingMode;
    info.postLightingBlendingMode = mPostLightingBlendingMode;
    info.shading = mShading;
    info.hasShadowMultiplier = mShadowMultiplier;
    info.multiBounceAO = mMultiBounceAO;
    info.multiBounceAOSet = mMultiBounceAOSet;
    info.specularAO = mSpecularAO;
    info.specularAOSet = mSpecularAOSet;
}

bool MaterialBuilder::hasExternalSampler() const noexcept
{
    for (size_t i = 0, c = mParameterCount; i < c; i++) {
        auto const& param = mParameters[i];
        if (param.isSampler && param.samplerType == SamplerType::SAMPLER_EXTERNAL) {
            return  true;
        }
    }
    return false;
}

}