static const char* SHADERS_DEPTH_MAIN_FS_DATA = R"(

//------------------------------------------------------------------------------
// Depth
//------------------------------------------------------------------------------

void main() {
#if defined(BLEND_MODE_MASKED)
    MaterialInputs inputs;
    initMaterial(inputs);
    material(inputs);

    float alpha = inputs.baseColor.a;
    if (alpha < getMaskThreshold()) {
        discard;
    }
#endif
}

)";