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

#include "pbr/MaterialEnums.h"
#include "pbr/UniformInterfaceBlock.h"
#include "pbr/SamplerInterfaceBlock.h"
#include "pbr/SamplerBindingMap.h"

namespace pbr
{

struct MaterialInfo 
{
    bool isLit;
    bool hasDoubleSidedCapability;
    bool hasExternalSamplers;
    bool hasShadowMultiplier;
    bool specularAntiAliasing;
    bool clearCoatIorChange;
    bool flipUV;
    bool multiBounceAO;
    bool multiBounceAOSet;
    bool specularAO;
    bool specularAOSet;
    AttributeBitset requiredAttributes;
    BlendingMode    blendingMode;
    BlendingMode    postLightingBlendingMode;
    Shading         shading;
    UniformInterfaceBlock uib;
    SamplerInterfaceBlock sib;
    SamplerBindingMap     samplerBindings;
};

}