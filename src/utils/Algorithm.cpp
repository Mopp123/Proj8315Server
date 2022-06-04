
#include "Algorithm.h"
#include <cstring>
#include "Debug.h"
#include <string>
#include <algorithm>
#include <limits>

float lerp(float v1, float v2, float amount)
{
	return ((1.0f - amount) * v1) + (amount * v2);
}


std::vector<float> generate_perlin(const float* seedArray, const int seedArrLength, int octaves, float scalingMod)
{
	std::vector<float> result(seedArrLength);

	for(int x = 0; x < seedArrLength; ++x)
	{
		float noiseVal = 0.0f;
		float scaleFactor = 1.0f;
		int count = 0;
		for(int o = 0; o < octaves; ++o)
		{
			int interval = seedArrLength >> o;
			int samplePos1 = (x / interval) * interval;
			int samplePos2 = (samplePos1 + interval) % seedArrLength;

			float lerpAmount = (float)(x - samplePos1) / (float)(interval);
			noiseVal += lerp(seedArray[samplePos1], seedArray[samplePos2], lerpAmount) * scaleFactor;

			scaleFactor = scaleFactor / scalingMod;
			count += scaleFactor;
		}
		// * Div by count, since we want the vals to be between 0 and 1
		result[x] = noiseVal / count;
	}
	return result;
}


std::vector<float> generate_perlin2D(const float* seedArray, const int width, int octaves, float scalingMod, float* out_min)
{
	std::vector<float> result(width * width);

	float minVal = std::numeric_limits<float>::max();

	for(int x = 0; x < width; ++x)
	{
		for(int y = 0; y < width; ++y)
		{
			float noiseVal = 0.0f;
			float scaleFactor = 1.0f;
			int count = 1;
			for(int o = 0; o < octaves; ++o)
			{
				int interval = width >> o;
				int samplePosX1 = (x / interval) * interval;
				int samplePosY1 = (y / interval) * interval;
			
				int samplePosX2 = (samplePosX1 + interval) % width;
				int samplePosY2 = (samplePosY1 + interval) % width;
				
				float lerpAmountX = (float)(x - samplePosX1) / (float)(interval);
				float lerpAmountY = (float)(y - samplePosY1) / (float)(interval);
				
				float topVal = (1.0f - lerpAmountX) * seedArray[samplePosX1 + samplePosY1 * width] + lerpAmountX * seedArray[samplePosX2 + samplePosY1 * width];
				float bottomVal = (1.0f - lerpAmountX) * seedArray[samplePosX1 + samplePosY2 * width] + lerpAmountX * seedArray[samplePosX2 + samplePosY2 * width];
				
				noiseVal += lerp(topVal, bottomVal, lerpAmountY) * scaleFactor;
				//noiseVal += (lerpAmountY * (bottomVal - topVal) + topVal) * scaleFactor;

				count += scaleFactor;
				scaleFactor = scaleFactor / scalingMod;
			}
			// * Div by count, since we want the vals to be between 0 and 1
			float finalVal =  noiseVal / count;
			minVal = std::min(minVal, finalVal);
			result[x + y * width] = finalVal;
		}
	}
	if(out_min)
		*out_min = minVal;

	return result;
}

