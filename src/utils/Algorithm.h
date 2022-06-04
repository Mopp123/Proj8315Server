#pragma once

#include <vector>


float lerp(float v1, float v2, float amount);

std::vector<float> generate_perlin(const float* seedArray, const int seedArrLength, int octaves, float scalingMod);
std::vector<float> generate_perlin2D(const float* seedArray, const int width, int octaves, float scalingMod, float* out_min = nullptr);

