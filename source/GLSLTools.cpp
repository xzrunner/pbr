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

#include "pbr/GLSLTools.h"
#include "pbr/ASTHelpers.h"

// glslang
#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/PoolAlloc.h>
#include <glslang/Include/intermediate.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/MachineIndependent/localintermediate.h>

#include "pbr/builtinResource.h"

#include <iostream>

namespace
{

class GLSLangCleaner {
public:
    GLSLangCleaner() {
        mAllocator = &glslang::GetThreadPoolAllocator();
    }
    ~GLSLangCleaner() {
        glslang::GetThreadPoolAllocator().pop();
        glslang::SetThreadPoolAllocator(mAllocator);
    }

private:
    glslang::TPoolAllocator* mAllocator;
};

int glslangVersionFromShaderModel(pbr::ShaderModel model)
{
    int version = 110;
    switch (model) {
    case pbr::ShaderModel::UNKNOWN:
        break;
    case pbr::ShaderModel::GL_ES_30:
        version = 100;
        break;
    case pbr::ShaderModel::GL_CORE_41:
        break;
    }
    return version;
}

EShMessages glslangFlagsFromTargetApi(pbr::MaterialBuilder::TargetApi targetApi)
{
    EShMessages msg = EShMessages::EShMsgDefault;
    if (targetApi == pbr::MaterialBuilder::TargetApi::VULKAN) {
        msg = (EShMessages)(EShMessages::EShMsgVulkanRules | EShMessages::EShMsgSpvRules);
    }
    return msg;
}

}

namespace pbr
{

bool GLSLTools::AnalyzeFragmentShader(const std::string& shaderCode, ShaderModel model,
                                      MaterialBuilder::TargetApi targetApi) const noexcept
{
    ShInitialize();

    // Parse to check syntax and semantic.
    const char* shaderCString = shaderCode.c_str();

    glslang::TShader tShader(EShLanguage::EShLangFragment);
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner cleaner;
    int version = glslangVersionFromShaderModel(model);
    EShMessages msg = glslangFlagsFromTargetApi(targetApi);
    bool ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!ok) {
        //std::cerr << "ERROR: Unable to parse fragment shader:" << std::endl;
        //std::cerr << tShader.getInfoLog() << utils::io::flush;
        return false;
    }

    TIntermNode* root = tShader.getIntermediate()->getTreeRoot();
    // Check there is a material function definition in this shader.
    TIntermNode* materialFctNode = ASTUtils::getFunctionByNameOnly("material", *root);
    if (materialFctNode == nullptr) {
        std::cerr << "ERROR: Invalid fragment shader:" << std::endl;
        std::cerr << "ERROR: Unable to find material() function" << std::endl;
        return false;
    }

    // Check there is a prepareMaterial function defintion in this shader.
    glslang::TIntermAggregate* prepareMaterialNode = ASTUtils::getFunctionByNameOnly("prepareMaterial",
            *root);
    if (prepareMaterialNode == nullptr) {
        std::cerr << "ERROR: Invalid fragment shader:" << std::endl;
        std::cerr << "ERROR: Unable to find prepareMaterial() function" << std::endl;
        return false;
    }

    std::string prepareMaterialSignature = prepareMaterialNode->getName().c_str();
    bool prepareMaterialCalled = ASTUtils::isFunctionCalled(prepareMaterialSignature,
            *materialFctNode, *root);
    if (!prepareMaterialCalled) {
        std::cerr << "ERROR: Invalid fragment shader:" << std::endl;
        std::cerr << "ERROR: prepareMaterial() is not called" << std::endl;
        return false;
    }

    return true;
}

bool GLSLTools::AnalyzeVertexShader(const std::string& shaderCode, ShaderModel model,
                                    MaterialBuilder::TargetApi targetApi) const noexcept
{
    ShInitialize();

    // Parse to check syntax and semantic.
    const char* shaderCString = shaderCode.c_str();

    glslang::TShader tShader(EShLanguage::EShLangVertex);
    tShader.setStrings(&shaderCString, 1);

    GLSLangCleaner cleaner;
    int version = glslangVersionFromShaderModel(model);
    EShMessages msg = glslangFlagsFromTargetApi(targetApi);
    bool ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!ok) {
        std::cerr << "ERROR: Unable to parse vertex shader" << std::endl;
        std::cerr << tShader.getInfoLog() << std::flush;
        return false;
    }

    TIntermNode* root = tShader.getIntermediate()->getTreeRoot();
    // Check there is a material function definition in this shader.
    TIntermNode* materialFctNode = ASTUtils::getFunctionByNameOnly("materialVertex", *root);
    if (materialFctNode == nullptr) {
        std::cerr << "ERROR: Invalid vertex shader" << std::endl;
        std::cerr << "ERROR: Unable to find materialVertex() function" << std::endl;
        return false;
    }

    return true;
}

}