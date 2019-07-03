#pragma once

#include "pbr/MaterialEnums.h"

#include <vector>
#include <memory>

namespace pbr
{

class Context
{
public:
    struct AttributeCfg
    {
        bool color    : 1;
        bool uv0      : 1;
        bool uv1      : 1;
        bool tangents : 1;
    };

    enum class TargetApi : uint8_t {
        OPENGL = 0x01,
        VULKAN = 0x02,
        METAL = 0x04,
        ALL = OPENGL | VULKAN | METAL
    };

    enum class TargetLanguage {
        GLSL,
        SPIRV
    };

public:
    auto& GetArrtCfg() const { return m_attr_cfg; }

private:
    AttributeCfg m_attr_cfg;

}; // Context

}