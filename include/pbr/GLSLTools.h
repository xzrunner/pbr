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

#include <list>

namespace pbr
{

// Used for symbol tracking during static code analysis.
struct Access {
    enum Type {Swizzling, DirectIndexForStruct, FunctionCall};
    Type type;
    std::string string;
    size_t parameterIdx = 0; // Only used when type == FunctionCall;
};

// Record of symbol interactions in a statment involving a symbol. Can track a sequence of up to
// (and in this order):
// Function call: foo(material)
// DirectIndexForStruct e.g: material.baseColor
// Swizzling e.g: material.baseColor.xyz
// Combinations are possible. e.g: foo(material.baseColor.xyz)
class Symbol {
public:
    Symbol() {}
    Symbol(const std::string& name) {
        mName = name;
    }

    std::string& getName() {
        return mName;
    }

    std::list<Access>& getAccesses() {
        return mAccesses;
    };

    void setName(const std::string& name) {
        mName = name;
    }

    void add(const Access& access) {
        mAccesses.push_front(access);
    }

    std::string toString() const {
        std::string str(mName);
        for (Access access: mAccesses) {
            str += ".";
            str += access.string;
        }
        return str;
    }

    bool hasDirectIndexForStruct() const noexcept {
        for (Access access : mAccesses) {
            if (access.type == Access::Type::DirectIndexForStruct) {
                return true;
            }
        }
        return false;
    }

    const std::string getDirectIndexStructName() const noexcept {
        for (Access access : mAccesses) {
            if (access.type == Access::Type::DirectIndexForStruct) {
                return access.string;
            }
        }
        return "";
    }

private:
    std::list<Access> mAccesses;
    std::string mName;

}; // Symbol

class GLSLTools
{
public:
    // Return true if:
    // The shader is syntactically and semantically valid AND
    // The shader features a material() function AND
    // The shader features a prepareMaterial() function AND
    // prepareMaterial() is called at some point in material() call chain.
    bool AnalyzeFragmentShader(const std::string& shaderCode, ShaderModel model,
        MaterialBuilder::TargetApi targetApi) const noexcept;
    bool AnalyzeVertexShader(const std::string& shaderCode, ShaderModel model,
        MaterialBuilder::TargetApi targetApi) const noexcept;

}; // GLSLTools

}