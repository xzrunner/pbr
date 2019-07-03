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

#include "pbr/MaterialBuilder.h"
#include "pbr/MaterialEnums.h"
#include "pbr/Variant.h"

namespace pbr
{

class CodeGenerator;
class UniformInterfaceBlock;
class SamplerInterfaceBlock;

class ShaderGenerator
{
public:
    ShaderGenerator(
        MaterialBuilder::PropertyList const& properties,
        MaterialBuilder::VariableList const& variables,
        const std::string& materialCode,
        size_t lineOffset,
        const std::string& materialVertexCode,
        size_t vertexLineOffset) noexcept;

    const std::string createVertexProgram(ShaderModel sm, MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage, MaterialInfo const& material, uint8_t variantKey,
        Interpolation interpolation, VertexDomain vertexDomain) const noexcept;

    const std::string createFragmentProgram(ShaderModel sm, MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage, MaterialInfo const& material, uint8_t variantKey,
        Interpolation interpolation) const noexcept;

    bool hasCustomDepthShader() const noexcept;

private:
    // generate prolog for the given shader
    void generateProlog(CodeGenerator& cg, ShaderType type, bool hasExternalSamplers) const;

    void generateEpilog(CodeGenerator& cg) const;

    // generate common functions for the given shader
    void generateCommon(CodeGenerator& cg, ShaderType type) const;
    void generateCommonMaterial(CodeGenerator& cg, ShaderType type) const;

    // generate the shader's main()
    void generateShaderMain(CodeGenerator& cg, ShaderType type) const;

    // generate the shader's code for the lit shading model
    void generateShaderLit(CodeGenerator& cg, ShaderType type, Variant variant, Shading shading) const;

    // generate the shader's code for the unlit shading model
    void generateShaderUnlit(CodeGenerator& cg, ShaderType type, Variant variant, bool hasShadowMultiplier) const;

    // generate material properties getters
    void generateMaterialDefines(CodeGenerator& cg, MaterialBuilder::PropertyList const properties) const;

    // generate declarations for non-custom "in" variables
    void generateShaderInputs(CodeGenerator& cg, ShaderType type, const AttributeBitset& attributes, Interpolation interpolation) const;

    // generate declarations for custom interpolants
    void generateVariable(CodeGenerator& cg, ShaderType type, const std::string& name, size_t index) const;

    // generate no-op shader for depth prepass
    void generateDepthShaderMain(CodeGenerator& cg, ShaderType type) const;

    // generate uniforms
    void generateUniforms(CodeGenerator& cg, ShaderType type, uint8_t binding,
        const UniformInterfaceBlock& uib) const;

    // generate samplers
    void generateSamplers(CodeGenerator& cg, uint8_t firstBinding, const SamplerInterfaceBlock& sib) const;

    void generateVertexDomain(CodeGenerator& cg, VertexDomain domain) const noexcept;

    void generateDefine(CodeGenerator& cg, const char* name, bool value) const;
    void generateDefine(CodeGenerator& cg, const char* name, float value) const;
    void generateDefine(CodeGenerator& cg, const char* name, uint32_t value) const;
    void generateDefine(CodeGenerator& cg, const char* name, const char* string) const;

    void generateGetters(CodeGenerator& cg, ShaderType type) const;
    void generateParameters(CodeGenerator& cg, ShaderType type) const;

private:
    Precision getDefaultPrecision(ShaderType type) const;
    Precision getDefaultUniformPrecision() const;

    // return type name of sampler  (e.g.: "sampler2D")
    char const* getSamplerTypeName(SamplerType type, SamplerFormat format, bool multisample) const noexcept;

    // return name of the material property (e.g.: "ROUGHNESS")
    static char const* getConstantName(MaterialBuilder::Property property) noexcept;

    static char const* getPrecisionQualifier(Precision precision, Precision defaultPrecision) noexcept;

    static bool hasPrecision(UniformType type) noexcept;

    const char* getUniformPrecisionQualifier(UniformType type, Precision precision,
        Precision uniformPrecision, Precision defaultPrecision) const noexcept;

private:
    ShaderModel                     mShaderModel;
    MaterialBuilder::TargetApi      mTargetApi;
    MaterialBuilder::TargetLanguage mTargetLanguage;

    MaterialBuilder::PropertyList mProperties;
    MaterialBuilder::VariableList mVariables;
    std::string mMaterialCode;
    std::string mMaterialVertexCode;
    size_t mMaterialLineOffset;
    size_t mMaterialVertexLineOffset;

}; // ShaderGenerator

}