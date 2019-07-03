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

#include "pbr/ShaderGenerator.h"
#include "pbr/MaterialInfo.h"
#include "pbr/CodeGenerator.h"
#include "pbr/Variant.h"
#include "pbr/MaterialEnums.h"
#include "pbr/UibGenerator.h"
#include "pbr/SibGenerator.h"

#include "shaders/ambient_occlusion.fs"
#include "shaders/brdf.fs"
#include "shaders/common_getters.fs"
#include "shaders/common_graphics.fs"
#include "shaders/common_lighting.fs"
#include "shaders/common_material.fs"
#include "shaders/common_math.fs"
#include "shaders/common_shading.fs"
#include "shaders/common_types.fs"
#include "shaders/depth_main.fs"
#include "shaders/depth_main.vs"
#include "shaders/getters.fs"
#include "shaders/getters.vs"
#include "shaders/inputs.fs"
#include "shaders/inputs.vs"
#include "shaders/light_directional.fs"
#include "shaders/light_indirect.fs"
#include "shaders/light_punctual.fs"
#include "shaders/main.vs"
#include "shaders/main.fs"
#include "shaders/material_inputs.fs"
#include "shaders/material_inputs.vs"
#include "shaders/shading_lit.fs"
#include "shaders/shading_model_cloth.fs"
#include "shaders/shading_model_standard.fs"
#include "shaders/shading_model_subsurface.fs"
#include "shaders/shading_parameters.fs"
#include "shaders/shading_unlit.fs"
#include "shaders/shadowing.fs"
#include "shaders/shadowing.vs"

#include <sstream>

#include <assert.h>

namespace
{

const char* getShadingDefine(pbr::Shading shading) noexcept
{
    switch (shading) {
        case pbr::Shading::LIT:                 return "SHADING_MODEL_LIT";
        case pbr::Shading::UNLIT:               return "SHADING_MODEL_UNLIT";
        case pbr::Shading::SUBSURFACE:          return "SHADING_MODEL_SUBSURFACE";
        case pbr::Shading::CLOTH:               return "SHADING_MODEL_CLOTH";
        case pbr::Shading::SPECULAR_GLOSSINESS: return "SHADING_MODEL_SPECULAR_GLOSSINESS";
        default: return "";
    }
}

char const* getConstantName(pbr::MaterialBuilder::Property property) noexcept
{
    using Property = pbr::MaterialBuilder::Property;
    switch (property) {
        case Property::BASE_COLOR:           return "BASE_COLOR";
        case Property::ROUGHNESS:            return "ROUGHNESS";
        case Property::METALLIC:             return "METALLIC";
        case Property::REFLECTANCE:          return "REFLECTANCE";
        case Property::AMBIENT_OCCLUSION:    return "AMBIENT_OCCLUSION";
        case Property::CLEAR_COAT:           return "CLEAR_COAT";
        case Property::CLEAR_COAT_ROUGHNESS: return "CLEAR_COAT_ROUGHNESS";
        case Property::CLEAR_COAT_NORMAL:    return "CLEAR_COAT_NORMAL";
        case Property::ANISOTROPY:           return "ANISOTROPY";
        case Property::ANISOTROPY_DIRECTION: return "ANISOTROPY_DIRECTION";
        case Property::THICKNESS:            return "THICKNESS";
        case Property::SUBSURFACE_POWER:     return "SUBSURFACE_POWER";
        case Property::SUBSURFACE_COLOR:     return "SUBSURFACE_COLOR";
        case Property::SHEEN_COLOR:          return "SHEEN_COLOR";
        case Property::GLOSSINESS:           return "GLOSSINESS";
        case Property::SPECULAR_COLOR:       return "SPECULAR_COLOR";
        case Property::EMISSIVE:             return "EMISSIVE";
        case Property::NORMAL:               return "NORMAL";
        case Property::POST_LIGHTING_COLOR:  return "POST_LIGHTING_COLOR";
        default: return "";
    }
}

char const* getInterpolationQualifier(pbr::Interpolation interpolation) noexcept {
    switch (interpolation) {
        case pbr::Interpolation::SMOOTH: return "";
        case pbr::Interpolation::FLAT:   return "flat ";
        default: return"";
    }
}

char const* getUniformTypeName(pbr::UniformType type) noexcept
{
    switch (type)
    {
    case pbr::UniformType::BOOL:   return "bool";
    case pbr::UniformType::BOOL2:  return "bvec2";
    case pbr::UniformType::BOOL3:  return "bvec3";
    case pbr::UniformType::BOOL4:  return "bvec4";
    case pbr::UniformType::FLOAT:  return "float";
    case pbr::UniformType::FLOAT2: return "vec2";
    case pbr::UniformType::FLOAT3: return "vec3";
    case pbr::UniformType::FLOAT4: return "vec4";
    case pbr::UniformType::INT:    return "int";
    case pbr::UniformType::INT2:   return "ivec2";
    case pbr::UniformType::INT3:   return "ivec3";
    case pbr::UniformType::INT4:   return "ivec4";
    case pbr::UniformType::UINT:   return "uint";
    case pbr::UniformType::UINT2:  return "uvec2";
    case pbr::UniformType::UINT3:  return "uvec3";
    case pbr::UniformType::UINT4:  return "uvec4";
    case pbr::UniformType::MAT3:   return "mat3";
    case pbr::UniformType::MAT4:   return "mat4";
    default: return"";
    }
}

size_t countLines(const char* s) noexcept
{
    size_t lines = 0;
    size_t i = 0;
    while (s[i] != 0) {
        if (s[i++] == '\n') lines++;
    }
    return lines;
}

size_t countLines(const std::string& s) noexcept
{
    size_t lines = 0;
    for (char i : s) {
        if (i == '\n') lines++;
    }
    return lines;
}

void appendShader(pbr::CodeGenerator& cg, const std::string& shader, size_t lineOffset) noexcept
{
    if (!shader.empty())
    {
        size_t lines = countLines(cg.ToText());
        std::stringstream ss;
        ss << "#line " << lineOffset;
        if (shader[0] != '\n') ss << "\n";
        ss << shader.c_str();
        if (shader[shader.size() - 1] != '\n') {
            ss << "\n";
            lines++;
        }
        // + 2 to account for the #line directives we just added
        ss << "#line " << lines + countLines(shader) + 2 << "\n";
        cg.Line(ss.str());
    }
}

bool isMobileTarget(pbr::ShaderModel model)
{
    switch (model) {
    case pbr::ShaderModel::UNKNOWN:
        return false;
    case pbr::ShaderModel::GL_ES_30:
        return true;
    case pbr::ShaderModel::GL_CORE_41:
        return false;
    default:
        return false;
    }
}

}

namespace pbr
{

ShaderGenerator::ShaderGenerator(MaterialBuilder::PropertyList const& properties,
                                 MaterialBuilder::VariableList const& variables,
                                 const std::string& materialCode,
                                 size_t lineOffset,
                                 const std::string& materialVertexCode,
                                 size_t vertexLineOffset) noexcept
{
    mShaderModel    = ShaderModel::GL_ES_30;
    mTargetApi      = MaterialBuilder::TargetApi::OPENGL;
    mTargetLanguage = MaterialBuilder::TargetLanguage::GLSL;

    std::copy(std::begin(properties), std::end(properties), std::begin(mProperties));
    std::copy(std::begin(variables), std::end(variables), std::begin(mVariables));

    mMaterialCode = materialCode;
    mMaterialVertexCode = materialVertexCode;
    mMaterialLineOffset = lineOffset;
    mMaterialVertexLineOffset = vertexLineOffset;

    if (mMaterialCode.empty()) {
        mMaterialCode =
            "void material(inout MaterialInputs m) {\n    prepareMaterial(m);\n}\n";
    }
    if (mMaterialVertexCode.empty()) {
        mMaterialVertexCode =
            "void materialVertex(inout MaterialVertexInputs m) {\n}\n";
    }
}

const std::string ShaderGenerator::createVertexProgram(
    ShaderModel sm, MaterialBuilder::TargetApi targetApi,
    MaterialBuilder::TargetLanguage targetLanguage, MaterialInfo const& material,
    uint8_t variantKey, Interpolation interpolation, VertexDomain vertexDomain) const noexcept
{
    CodeGenerator cg;
    const bool lit = material.isLit;
    const Variant variant(variantKey);

    generateProlog(cg, ShaderType::VERTEX, material.hasExternalSamplers);

    generateDefine(cg, "FLIP_UV_ATTRIBUTE", material.flipUV);

    bool litVariants = lit || material.hasShadowMultiplier;
    generateDefine(cg, "HAS_DIRECTIONAL_LIGHTING", litVariants && variant.hasDirectionalLighting());
    generateDefine(cg, "HAS_SHADOWING", litVariants && variant.hasShadowReceiver());
    generateDefine(cg, "HAS_SHADOW_MULTIPLIER", material.hasShadowMultiplier);
    generateDefine(cg, "HAS_SKINNING", variant.hasSkinning());
    generateDefine(cg, getShadingDefine(material.shading), true);
    generateMaterialDefines(cg, mProperties);

    AttributeBitset attributes = material.requiredAttributes;
    if (variant.hasSkinning()) {
        attributes.set(static_cast<int>(VertexAttribute::BONE_INDICES));
        attributes.set(static_cast<int>(VertexAttribute::BONE_WEIGHTS));
    }
    generateShaderInputs(cg, ShaderType::VERTEX, attributes, interpolation);

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        generateVariable(cg, ShaderType::VERTEX, variable, variableIndex++);
    }

    // materials defines
    generateVertexDomain(cg, vertexDomain);

    // uniforms
    generateUniforms(cg, ShaderType::VERTEX,
            BindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    generateUniforms(cg, ShaderType::VERTEX,
            BindingPoints::PER_RENDERABLE, UibGenerator::getPerRenderableUib());
    if (variant.hasSkinning()) {
        generateUniforms(cg, ShaderType::VERTEX,
                BindingPoints::PER_RENDERABLE_BONES,
                UibGenerator::getPerRenderableBonesUib());
    }
    generateUniforms(cg, ShaderType::VERTEX,
            BindingPoints::PER_MATERIAL_INSTANCE, material.uib);
    cg.Line();
    // TODO: should we generate per-view SIB in the vertex shader?
    generateSamplers(cg, material.samplerBindings.getBlockOffset(BindingPoints::PER_MATERIAL_INSTANCE), material.sib);

    // shader code
    generateCommon(cg, ShaderType::VERTEX);
    generateGetters(cg, ShaderType::VERTEX);
    generateCommonMaterial(cg, ShaderType::VERTEX);

    if (variant.isDepthPass() &&
        (material.blendingMode != BlendingMode::MASKED) &&
        !hasCustomDepthShader()) {
        // these variants are special and are treated as DEPTH variants. Filament will never
        // request that variant for the color pass.
        generateDepthShaderMain(cg, ShaderType::VERTEX);
    } else {
        // main entry point
        appendShader(cg, mMaterialVertexCode, mMaterialVertexLineOffset);
        generateShaderMain(cg, ShaderType::VERTEX);
    }

    generateEpilog(cg);

    return cg.ToText();
}

const std::string ShaderGenerator::createFragmentProgram(
    ShaderModel shaderModel, MaterialBuilder::TargetApi targetApi,
    MaterialBuilder::TargetLanguage targetLanguage, MaterialInfo const& material,
    uint8_t variantKey, Interpolation interpolation) const noexcept
{
    CodeGenerator cg;
    const bool lit = material.isLit;
    const Variant variant(variantKey);

    generateProlog(cg, ShaderType::FRAGMENT, material.hasExternalSamplers);

    // this should probably be a code generation option
    generateDefine(cg, "USE_MULTIPLE_SCATTERING_COMPENSATION", true);

    generateDefine(cg, "GEOMETRIC_SPECULAR_AA", material.specularAntiAliasing && lit);

    generateDefine(cg, "CLEAR_COAT_IOR_CHANGE", material.clearCoatIorChange);

    bool specularAO = material.specularAOSet ?
            material.specularAO : !isMobileTarget(shaderModel);
    generateDefine(cg, "SPECULAR_AMBIENT_OCCLUSION", specularAO ? 1u : 0u);

    bool multiBounceAO = material.multiBounceAOSet ?
            material.multiBounceAO : !isMobileTarget(shaderModel);
    generateDefine(cg, "MULTI_BOUNCE_AMBIENT_OCCLUSION", multiBounceAO ? 1u : 0u);

    // lighting variants
    bool litVariants = lit || material.hasShadowMultiplier;
    generateDefine(cg, "HAS_DIRECTIONAL_LIGHTING", litVariants && variant.hasDirectionalLighting());
    generateDefine(cg, "HAS_DYNAMIC_LIGHTING", litVariants && variant.hasDynamicLighting());
    generateDefine(cg, "HAS_SHADOWING", litVariants && variant.hasShadowReceiver());
    generateDefine(cg, "HAS_SHADOW_MULTIPLIER", material.hasShadowMultiplier);

    // material defines
    generateDefine(cg, "MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY", material.hasDoubleSidedCapability);
    switch (material.blendingMode) {
        case BlendingMode::B_OPAQUE:
            generateDefine(cg, "BLEND_MODE_OPAQUE", true);
            break;
        case BlendingMode::B_TRANSPARENT:
            generateDefine(cg, "BLEND_MODE_TRANSPARENT", true);
            break;
        case BlendingMode::ADD:
            generateDefine(cg, "BLEND_MODE_ADD", true);
            break;
        case BlendingMode::MASKED:
            generateDefine(cg, "BLEND_MODE_MASKED", true);
            break;
        case BlendingMode::FADE:
            // Fade is a special case of transparent
            generateDefine(cg, "BLEND_MODE_TRANSPARENT", true);
            generateDefine(cg, "BLEND_MODE_FADE", true);
            break;
        case BlendingMode::MULTIPLY:
            generateDefine(cg, "BLEND_MODE_MULTIPLY", true);
            break;
        case BlendingMode::SCREEN:
            generateDefine(cg, "BLEND_MODE_SCREEN", true);
            break;
    }
    switch (material.postLightingBlendingMode) {
        case BlendingMode::B_OPAQUE:
            generateDefine(cg, "POST_LIGHTING_BLEND_MODE_OPAQUE", true);
            break;
        case BlendingMode::B_TRANSPARENT:
            generateDefine(cg, "POST_LIGHTING_BLEND_MODE_TRANSPARENT", true);
            break;
        case BlendingMode::ADD:
            generateDefine(cg, "POST_LIGHTING_BLEND_MODE_ADD", true);
            break;
        case BlendingMode::MULTIPLY:
            generateDefine(cg, "POST_LIGHTING_BLEND_MODE_MULTIPLY", true);
            break;
        case BlendingMode::SCREEN:
            generateDefine(cg, "POST_LIGHTING_BLEND_MODE_SCREEN", true);
            break;
        default:
            break;
    }
    generateDefine(cg, getShadingDefine(material.shading), true);
    generateMaterialDefines(cg, mProperties);

    generateShaderInputs(cg, ShaderType::FRAGMENT, material.requiredAttributes, interpolation);

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        generateVariable(cg, ShaderType::FRAGMENT, variable, variableIndex++);
    }

    // uniforms and samplers
    generateUniforms(cg, ShaderType::FRAGMENT,
            BindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    generateUniforms(cg, ShaderType::FRAGMENT,
            BindingPoints::LIGHTS, UibGenerator::getLightsUib());
    generateUniforms(cg, ShaderType::FRAGMENT,
            BindingPoints::PER_MATERIAL_INSTANCE, material.uib);
    cg.Line();
    generateSamplers(cg,
            material.samplerBindings.getBlockOffset(BindingPoints::PER_VIEW),
            SibGenerator::getPerViewSib());
    generateSamplers(cg,
            material.samplerBindings.getBlockOffset(BindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    // shading code
    generateCommon(cg, ShaderType::FRAGMENT);
    generateGetters(cg, ShaderType::FRAGMENT);
    generateCommonMaterial(cg, ShaderType::FRAGMENT);
    generateParameters(cg, ShaderType::FRAGMENT);

    // shading model
    if (variant.isDepthPass()) {
        if (material.blendingMode == BlendingMode::MASKED) {
            appendShader(cg, mMaterialCode, mMaterialLineOffset);
        }
        // these variants are special and are treated as DEPTH variants. Filament will never
        // request that variant for the color pass.
        generateDepthShaderMain(cg, ShaderType::FRAGMENT);
    } else {
        appendShader(cg, mMaterialCode, mMaterialLineOffset);
        if (material.isLit) {
            generateShaderLit(cg, ShaderType::FRAGMENT, variant, material.shading);
        } else {
            generateShaderUnlit(cg, ShaderType::FRAGMENT, variant, material.hasShadowMultiplier);
        }
        // entry point
        generateShaderMain(cg, ShaderType::FRAGMENT);
    }

    generateEpilog(cg);

    return cg.ToText();
}

bool ShaderGenerator::hasCustomDepthShader() const noexcept
{
    for (const auto& variable : mVariables) {
        if (!variable.empty()) {
            return true;
        }
    }
    return false;
}

void ShaderGenerator::generateProlog(CodeGenerator& cg, ShaderType type, bool hasExternalSamplers) const
{
    assert(mShaderModel != ShaderModel::UNKNOWN);
    switch (mShaderModel) {
        case ShaderModel::UNKNOWN:
            break;
        case ShaderModel::GL_ES_30:
            // Vulkan requires version 310 or higher
            if (mTargetLanguage == MaterialBuilder::TargetLanguage::SPIRV) {
                // Vulkan requires layout locations on ins and outs, which were not supported
                // in the OpenGL 4.1 GLSL profile.
                cg.Line("#version 310 es\n");
            } else {
                cg.Line("#version 300 es\n");
            }
            if (hasExternalSamplers) {
                cg.Line("#extension GL_OES_EGL_image_external_essl3 : require\n");
            }
            cg.Line("#define TARGET_MOBILE");
            break;
        case ShaderModel::GL_CORE_41:
            if (mTargetLanguage == MaterialBuilder::TargetLanguage::SPIRV) {
                // Vulkan requires binding specifiers on uniforms and samplers, which were not
                // supported in the OpenGL 4.1 GLSL profile.
                cg.Line("#version 450 core\n");
            } else {
                cg.Line("#version 410 core\n");
            }
            break;
    }

    if (mTargetApi == MaterialBuilder::TargetApi::VULKAN) {
        cg.Line("#define TARGET_VULKAN_ENVIRONMENT");
    }
    if (mTargetApi == MaterialBuilder::TargetApi::METAL) {
        cg.Line("#define TARGET_METAL_ENVIRONMENT");
    }
    if (mTargetLanguage == MaterialBuilder::TargetLanguage::SPIRV) {
        cg.Line("#define TARGET_LANGUAGE_SPIRV");
    }

    Precision defaultPrecision = getDefaultPrecision(type);
    const char* precision = getPrecisionQualifier(defaultPrecision, Precision::DEFAULT);
    cg.LineFmt("precision %s float;\n", precision);
    cg.LineFmt("precision %s int;\n", precision);

    // The version of the Metal Shading Language (MSL) we use does not have the invariant qualifier.
    // New versions of MSL (> 2.1) have it, but we want to support older devices.
    if (type == ShaderType::VERTEX && mTargetApi != MaterialBuilder::TargetApi::METAL) {
        cg.Line("");
        cg.Line("invariant gl_Position;");
    }

    cg.Line(SHADERS_COMMON_TYPES_FS_DATA);

    cg.Line("");
}

void ShaderGenerator::generateEpilog(CodeGenerator& cg) const
{
    cg.Line(); // For line compression all shaders finish with a newline character.
}

void ShaderGenerator::generateCommon(CodeGenerator& cg, ShaderType type) const
{
    cg.Line(SHADERS_COMMON_MATH_FS_DATA);
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        cg.Line(SHADERS_COMMON_SHADING_FS_DATA);
        cg.Line(SHADERS_COMMON_GRAPHICS_FS_DATA);
        cg.Line(SHADERS_COMMON_MATERIAL_FS_DATA);
    }
}

void ShaderGenerator::generateCommonMaterial(CodeGenerator& cg, ShaderType type) const
{
    if (type == ShaderType::VERTEX) {
        cg.Line(SHADERS_MATERIAL_INPUTS_VS_DATA);
    } else if (type == ShaderType::FRAGMENT) {
        cg.Line(SHADERS_MATERIAL_INPUTS_FS_DATA);
    }
}

void ShaderGenerator::generateShaderMain(CodeGenerator& cg, ShaderType type) const
{
    if (type == ShaderType::VERTEX) {
        cg.Line(SHADERS_SHADOWING_VS_DATA);
        cg.Line(SHADERS_MAIN_VS_DATA);
    } else if (type == ShaderType::FRAGMENT) {
        cg.Line(SHADERS_MAIN_FS_DATA);
    }
}

void ShaderGenerator::generateShaderLit(CodeGenerator& cg, ShaderType type, Variant variant, Shading shading) const
{
    if (type == ShaderType::VERTEX)
    {
    }
    else if (type == ShaderType::FRAGMENT)
    {
        cg.Line(SHADERS_COMMON_LIGHTING_FS_DATA);
        if (variant.hasShadowReceiver()) {
            cg.Line(SHADERS_SHADOWING_FS_DATA);
        }

        cg.Line(SHADERS_BRDF_FS_DATA);
        switch (shading) {
            case Shading::UNLIT:
                assert("Lit shader generated with unlit shading model");
                break;
            case Shading::SPECULAR_GLOSSINESS:
            case Shading::LIT:
                cg.Line(SHADERS_SHADING_MODEL_STANDARD_FS_DATA);
                break;
            case Shading::SUBSURFACE:
                cg.Line(SHADERS_SHADING_MODEL_SUBSURFACE_FS_DATA);
                break;
            case Shading::CLOTH:
                cg.Line(SHADERS_SHADING_MODEL_CLOTH_FS_DATA);
                break;
        }

        if (shading != Shading::UNLIT) {
            cg.Line(SHADERS_AMBIENT_OCCLUSION_FS_DATA);
            cg.Line(SHADERS_LIGHT_INDIRECT_FS_DATA);
        }
        if (variant.hasDirectionalLighting()) {
            cg.Line(SHADERS_LIGHT_DIRECTIONAL_FS_DATA);
        }
        if (variant.hasDynamicLighting()) {
            cg.Line(SHADERS_LIGHT_PUNCTUAL_FS_DATA);
        }

        cg.Line(SHADERS_SHADING_LIT_FS_DATA);
    }
}

void ShaderGenerator::generateShaderUnlit(CodeGenerator& cg, ShaderType type, Variant variant, bool hasShadowMultiplier) const
{
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        if (hasShadowMultiplier) {
            if (variant.hasShadowReceiver()) {
                cg.Line(SHADERS_SHADOWING_FS_DATA);
            }
        }
        cg.Line(SHADERS_SHADING_UNLIT_FS_DATA);
    }
}

void ShaderGenerator::generateMaterialDefines(CodeGenerator& cg, MaterialBuilder::PropertyList const properties) const
{
    for (size_t i = 0; i < MaterialBuilder::MATERIAL_PROPERTIES_COUNT; i++) {
        if (properties[i]) {
            cg.LineFmt("#define MATERIAL_HAS_%s", getConstantName(static_cast<MaterialBuilder::Property>(i)));
        }
    }
}

void ShaderGenerator::generateShaderInputs(CodeGenerator& cg, ShaderType type, const AttributeBitset& attributes, Interpolation interpolation) const
{
    const char* shading = getInterpolationQualifier(interpolation);
    cg.LineFmt("#define SHADING_INTERPOLATION %s", shading);;

    bool hasTangents = attributes.test(static_cast<int>(VertexAttribute::TANGENTS));
    generateDefine(cg, "HAS_ATTRIBUTE_TANGENTS", hasTangents);

    bool hasColor = attributes.test(static_cast<int>(VertexAttribute::COLOR));
    generateDefine(cg, "HAS_ATTRIBUTE_COLOR", hasColor);

    bool hasUV0 = attributes.test(static_cast<int>(VertexAttribute::UV0));
    generateDefine(cg, "HAS_ATTRIBUTE_UV0", hasUV0);

    bool hasUV1 = attributes.test(static_cast<int>(VertexAttribute::UV1));
    generateDefine(cg, "HAS_ATTRIBUTE_UV1", hasUV1);

    bool hasBoneIndices = attributes.test(static_cast<int>(VertexAttribute::BONE_INDICES));
    generateDefine(cg, "HAS_ATTRIBUTE_BONE_INDICES", hasBoneIndices);

    bool hasBoneWeights = attributes.test(static_cast<int>(VertexAttribute::BONE_WEIGHTS));
    generateDefine(cg, "HAS_ATTRIBUTE_BONE_WEIGHTS", hasBoneWeights);

    if (type == ShaderType::VERTEX)
    {
        cg.Line("");
        generateDefine(cg, "LOCATION_POSITION", uint32_t(VertexAttribute::POSITION));
        if (hasTangents) {
            generateDefine(cg, "LOCATION_TANGENTS", uint32_t(VertexAttribute::TANGENTS));
        }
        if (hasUV0) {
            generateDefine(cg, "LOCATION_UV0", uint32_t(VertexAttribute::UV0));
        }
        if (hasUV1) {
            generateDefine(cg, "LOCATION_UV1", uint32_t(VertexAttribute::UV1));
        }
        if (hasColor) {
            generateDefine(cg, "LOCATION_COLOR", uint32_t(VertexAttribute::COLOR));
        }
        if (hasBoneIndices) {
            generateDefine(cg, "LOCATION_BONE_INDICES", uint32_t(VertexAttribute::BONE_INDICES));
        }
        if (hasBoneWeights) {
            generateDefine(cg, "LOCATION_BONE_WEIGHTS", uint32_t(VertexAttribute::BONE_WEIGHTS));
        }
        cg.Line(SHADERS_INPUTS_VS_DATA);
    } else if (type == ShaderType::FRAGMENT) {
        cg.Line(SHADERS_INPUTS_FS_DATA);
    }
}

void ShaderGenerator::generateVariable(CodeGenerator& cg, ShaderType type, const std::string& name, size_t index) const
{
    if (!name.empty())
    {
        if (type == ShaderType::VERTEX)
        {
            cg.LineFmt("\n#define VARIABLE_CUSTOM%d %s", index, name.c_str());
            cg.LineFmt("\n#define VARIABLE_CUSTOM_AT%d variable_%s", index, name.c_str());
            cg.LineFmt("LAYOUT_LOCATION(%d) out vec4 variable_%s;", index, name.c_str());
        }
        else if (type == ShaderType::FRAGMENT)
        {
            cg.LineFmt("\nLAYOUT_LOCATION(%d) in highp vec4 variable_%s;", index, name.c_str());
        }
    }
}

void ShaderGenerator::generateDepthShaderMain(CodeGenerator& cg, ShaderType type) const
{
    if (type == ShaderType::VERTEX) {
        cg.Line(SHADERS_DEPTH_MAIN_VS_DATA);
    } else if (type == ShaderType::FRAGMENT) {
        cg.Line(SHADERS_DEPTH_MAIN_FS_DATA);
    }
}

void ShaderGenerator::generateUniforms(CodeGenerator& cg, ShaderType type, uint8_t binding, const UniformInterfaceBlock& uib) const
{
    auto const& infos = uib.getUniformInfoList();
    if (infos.empty()) {
        return;
    }

    const std::string& blockName = uib.getName();
    std::string instanceName(uib.getName().c_str());
    std::transform(instanceName.begin(), instanceName.end(), instanceName.begin(), ::tolower);

    Precision uniformPrecision = getDefaultUniformPrecision();
    Precision defaultPrecision = getDefaultPrecision(type);

    std::string str;
    str = "\nlayout(";
    if (mTargetLanguage == MaterialBuilder::TargetLanguage::SPIRV) {
        uint32_t bindingIndex = (uint32_t) binding; // avoid char output
        str += "binding = " + std::to_string(bindingIndex) + ", ";
    }
    str += "std140) uniform " + blockName + " {";
    cg.Line(str);

    for (auto const& info : infos)
    {
        char const* const type = getUniformTypeName(info.type);
        char const* const precision = getUniformPrecisionQualifier(info.type, info.precision,
                uniformPrecision, defaultPrecision);

        std::stringstream ss;
        ss << "    " << precision;
        if (precision[0] != '\0') ss << " ";
        ss << type << " " << info.name.c_str();
        if (info.size > 1) {
            ss << "[" << info.size << "]";
        }
        ss << ";";
        cg.Line(ss.str());
    }
    cg.LineFmt("} %s;", instanceName);
}

void ShaderGenerator::generateSamplers(CodeGenerator& cg, uint8_t firstBinding,
                                       const SamplerInterfaceBlock& sib) const
{
    auto const& infos = sib.getSamplerInfoList();
    if (infos.empty()) {
        return;
    }

    for (auto const& info : infos)
    {
        std::string uniformName = SamplerInterfaceBlock::getUniformName(
            sib.getName().c_str(), info.name.c_str()
        );

        auto type = info.type;
        if (type == SamplerType::SAMPLER_EXTERNAL && mShaderModel != ShaderModel::GL_ES_30) {
            // we're generating the shader for the desktop, where we assume external textures
            // are not supported, in which case we revert to texture2d
            type = SamplerType::SAMPLER_2D;
        }
        char const* const typeName = getSamplerTypeName(type, info.format, info.multisample);
        char const* const precision = getPrecisionQualifier(info.precision, Precision::DEFAULT);
        std::stringstream ss;
        if (mTargetLanguage == MaterialBuilder::TargetLanguage::SPIRV)
        {
            const uint32_t bindingIndex = (uint32_t) firstBinding + info.offset;
            ss << "layout(binding = " << bindingIndex;

            // For Vulkan, we place uniforms in set 0 (the default set) and samplers in set 1. This
            // allows the sampler bindings to live in a separate "namespace" that starts at zero.
            // Note that the set specifier is not covered by the desktop GLSL spec, including
            // recent versions. It is only documented in the GL_KHR_vulkan_glsl extension.
            if (mTargetApi == MaterialBuilder::TargetApi::VULKAN) {
                ss << ", set = 1";
            }

            ss << ") ";
        }
        ss << "uniform " << precision << " " << typeName << " " << uniformName.c_str();
        ss << ";";
        cg.Line(ss.str());
    }
    cg.Line();
}

void ShaderGenerator::generateVertexDomain(CodeGenerator& cg, VertexDomain domain) const noexcept
{
    switch (domain)
    {
    case VertexDomain::OBJECT:
        generateDefine(cg, "VERTEX_DOMAIN_OBJECT", true);
        break;
    case VertexDomain::WORLD:
        generateDefine(cg, "VERTEX_DOMAIN_WORLD", true);
        break;
    case VertexDomain::VIEW:
        generateDefine(cg, "VERTEX_DOMAIN_VIEW", true);
        break;
    case VertexDomain::DEVICE:
        generateDefine(cg, "VERTEX_DOMAIN_DEVICE", true);
        break;
    }
}

void ShaderGenerator::generateDefine(CodeGenerator& cg, const char* name, bool value) const
{
    if (value) {
        cg.LineFmt("#define %s", name);
    }
}

void ShaderGenerator::generateDefine(CodeGenerator& cg, const char* name, float value) const
{
    char buffer[32];
    snprintf(buffer, 32, "%.1f", value);
    cg.LineFmt("#define %s %s", name, buffer);
}

void ShaderGenerator::generateDefine(CodeGenerator& cg, const char* name, uint32_t value) const
{
    cg.LineFmt("#define %s %d", name, value);
}

void ShaderGenerator::generateDefine(CodeGenerator& cg, const char* name, const char* string) const
{
    cg.LineFmt("#define %s %s", name, string);
}

void ShaderGenerator::generateGetters(CodeGenerator& cg, ShaderType type) const
{
    cg.Line(SHADERS_COMMON_GETTERS_FS_DATA);
    if (type == ShaderType::VERTEX) {
        cg.Line(SHADERS_GETTERS_VS_DATA);
    } else if (type == ShaderType::FRAGMENT) {
        cg.Line(SHADERS_GETTERS_FS_DATA);
    }
}

void ShaderGenerator::generateParameters(CodeGenerator& cg, ShaderType type) const
{
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        cg.Line(SHADERS_SHADING_PARAMETERS_FS_DATA);
    }
}

Precision ShaderGenerator::getDefaultPrecision(ShaderType type) const
{
    if (type == ShaderType::VERTEX) {
        return Precision::HIGH;
    }
    else if (type == ShaderType::FRAGMENT) {
        if (mShaderModel < ShaderModel::GL_CORE_41) {
            return Precision::MEDIUM;
        }
        else {
            return Precision::HIGH;
        }
    }
    return Precision::HIGH;
}

Precision ShaderGenerator::getDefaultUniformPrecision() const
{
    if (mShaderModel < ShaderModel::GL_CORE_41) {
        return Precision::MEDIUM;
    }
    else {
        return Precision::HIGH;
    }
}

char const* ShaderGenerator::getSamplerTypeName(SamplerType type, SamplerFormat format, bool multisample) const noexcept
{
    switch (type) {
        case SamplerType::SAMPLER_2D:
            if (!multisample) {
                switch (format) {
                    case SamplerFormat::INT:    return "isampler2D";
                    case SamplerFormat::UINT:   return "usampler2D";
                    case SamplerFormat::FLOAT:  return "sampler2D";
                    case SamplerFormat::SHADOW: return "sampler2DShadow";
                }
            } else {
                assert(format != SamplerFormat::SHADOW);
                switch (format) {
                    case SamplerFormat::INT:    return "ms_isampler2D";
                    case SamplerFormat::UINT:   return "ms_usampler2D";
                    case SamplerFormat::FLOAT:  return "ms_sampler2D";
                    case SamplerFormat::SHADOW: return "sampler2DShadow";   // should not happen
                }
            }
        case SamplerType::SAMPLER_CUBEMAP:
            assert(!multisample);
            switch (format) {
                case SamplerFormat::INT:    return "isamplerCube";
                case SamplerFormat::UINT:   return "usamplerCube";
                case SamplerFormat::FLOAT:  return "samplerCube";
                case SamplerFormat::SHADOW: return "samplerCubeShadow";
            }
        case SamplerType::SAMPLER_EXTERNAL:
            assert(!multisample);
            assert(format != SamplerFormat::SHADOW);
            // Vulkan doesn't have external textures in the sense as GL. Vulkan external textures
            // are created via VK_ANDROID_external_memory_android_hardware_buffer, but they are
            // backed by VkImage just like a normal texture, and sampled from normally.
            return (mTargetLanguage == MaterialBuilder::TargetLanguage::SPIRV) ? "sampler2D" : "samplerExternalOES";
        default: return"";
    }
}

/* static */
char const* ShaderGenerator::getConstantName(MaterialBuilder::Property property) noexcept
{
    using Property = MaterialBuilder::Property;
    switch (property) {
        case Property::BASE_COLOR:           return "BASE_COLOR";
        case Property::ROUGHNESS:            return "ROUGHNESS";
        case Property::METALLIC:             return "METALLIC";
        case Property::REFLECTANCE:          return "REFLECTANCE";
        case Property::AMBIENT_OCCLUSION:    return "AMBIENT_OCCLUSION";
        case Property::CLEAR_COAT:           return "CLEAR_COAT";
        case Property::CLEAR_COAT_ROUGHNESS: return "CLEAR_COAT_ROUGHNESS";
        case Property::CLEAR_COAT_NORMAL:    return "CLEAR_COAT_NORMAL";
        case Property::ANISOTROPY:           return "ANISOTROPY";
        case Property::ANISOTROPY_DIRECTION: return "ANISOTROPY_DIRECTION";
        case Property::THICKNESS:            return "THICKNESS";
        case Property::SUBSURFACE_POWER:     return "SUBSURFACE_POWER";
        case Property::SUBSURFACE_COLOR:     return "SUBSURFACE_COLOR";
        case Property::SHEEN_COLOR:          return "SHEEN_COLOR";
        case Property::GLOSSINESS:           return "GLOSSINESS";
        case Property::SPECULAR_COLOR:       return "SPECULAR_COLOR";
        case Property::EMISSIVE:             return "EMISSIVE";
        case Property::NORMAL:               return "NORMAL";
        case Property::POST_LIGHTING_COLOR:  return "POST_LIGHTING_COLOR";
        default: return"";
    }
}

/* static */
char const* ShaderGenerator::getPrecisionQualifier(Precision precision, Precision defaultPrecision) noexcept
{
    if (precision == defaultPrecision) {
        return "";
    }

    switch (precision) {
        case Precision::LOW:     return "lowp";
        case Precision::MEDIUM:  return "mediump";
        case Precision::HIGH:    return "highp";
        case Precision::DEFAULT: return "ERROR";
    }

    return "ERROR";
}

bool ShaderGenerator::hasPrecision(UniformType type) noexcept
{
    switch (type) {
        case UniformType::BOOL:
        case UniformType::BOOL2:
        case UniformType::BOOL3:
        case UniformType::BOOL4:
            return false;
        default:
            return true;
    }
}

const char* ShaderGenerator::getUniformPrecisionQualifier(UniformType type, Precision precision,
                                                          Precision uniformPrecision, Precision defaultPrecision) const noexcept
{
    if (!hasPrecision(type)) {
        return "";
    }
    if (precision == Precision::DEFAULT) {
        precision = uniformPrecision;
    }
    return getPrecisionQualifier(precision, defaultPrecision);
}

}