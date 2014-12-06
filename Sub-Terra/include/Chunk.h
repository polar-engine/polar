#pragma once

#include "Object.h"
#include "ModelComponent.h"
#include "OpenSimplexNoise.h"

class Chunk : public Object {
public:
	Chunk(unsigned char width, unsigned char height, unsigned char depth, const std::vector<bool> &blocks) {
		const std::vector<Triangle> blockFront = {
			std::make_tuple(Point(-0.5, -0.5,  0.5, 1), Point( 0.5, -0.5,  0.5, 1), Point(-0.5,  0.5,  0.5, 1)),
			std::make_tuple(Point( 0.5, -0.5,  0.5, 1), Point( 0.5,  0.5,  0.5, 1), Point(-0.5,  0.5,  0.5, 1))
		};
		const std::vector<Triangle> blockTop = {
			std::make_tuple(Point(-0.5,  0.5,  0.5, 1), Point( 0.5,  0.5,  0.5, 1), Point(-0.5,  0.5, -0.5, 1)),
			std::make_tuple(Point( 0.5,  0.5,  0.5, 1), Point( 0.5,  0.5, -0.5, 1), Point(-0.5,  0.5, -0.5, 1))
		};
		const std::vector<Triangle> blockBack = {
			std::make_tuple(Point(-0.5,  0.5, -0.5, 1), Point( 0.5,  0.5, -0.5, 1), Point(-0.5, -0.5, -0.5, 1)),
			std::make_tuple(Point( 0.5,  0.5, -0.5, 1), Point( 0.5, -0.5, -0.5, 1), Point(-0.5, -0.5, -0.5, 1))
		};
		const std::vector<Triangle> blockBottom = {
			std::make_tuple(Point(-0.5, -0.5, -0.5, 1), Point( 0.5, -0.5, -0.5, 1), Point(-0.5, -0.5,  0.5, 1)),
			std::make_tuple(Point( 0.5, -0.5, -0.5, 1), Point( 0.5, -0.5,  0.5, 1), Point(-0.5, -0.5,  0.5, 1))
		};
		const std::vector<Triangle> blockRight = {
			std::make_tuple(Point( 0.5, -0.5,  0.5, 1), Point( 0.5, -0.5, -0.5, 1), Point( 0.5,  0.5,  0.5, 1)),
			std::make_tuple(Point( 0.5, -0.5, -0.5, 1), Point( 0.5,  0.5, -0.5, 1), Point( 0.5,  0.5,  0.5, 1))
		};
		const std::vector<Triangle> blockLeft = {
			std::make_tuple(Point(-0.5, -0.5, -0.5, 1), Point(-0.5, -0.5,  0.5, 1), Point(-0.5,  0.5, -0.5, 1)),
			std::make_tuple(Point(-0.5, -0.5,  0.5, 1), Point(-0.5,  0.5,  0.5, 1), Point(-0.5,  0.5, -0.5, 1))
		};

		/*std::vector<unsigned char> heights;
		heights.resize(width * depth);

		std::random_device rDevice;
		auto seed = rDevice();
		OpenSimplexNoise noise(seed);

		for(unsigned char x = 0; x < width; ++x) {
			for(unsigned char z = 0; z < depth; ++z) {
				heights[x * depth + z] = static_cast<unsigned char>(noise.eval(x / 100.0f, z / 100.0f) * height);
			}
		}*/

		/*std::default_random_engine rGen(seed);
		std::negative_binomial_distribution<> rDist(height, 0.95);
		auto rHeight = std::bind(rDist, rGen);
		for(unsigned char x = 0; x < width; ++x) {
			for(unsigned char z = 0; z < depth; ++z) {
				heights[x * depth + z] = rHeight();
			}
		}*/

		/*std::vector<bool> blocks;
		blocks.resize(width * height * depth);
		for(unsigned char x = 0; x < width; ++x) {
			for(unsigned char z = 0; z < depth; ++z) {
				for(unsigned char y = 0; y < height; ++y) {
					auto current = z * width * height + x * height + y;
					blocks.at(current) = y <= heights[x * depth + z]
						//&& (x % 4 | y % 4 | z % 4) != 0
						;
				}
			}
		}*/

		std::vector<Triangle> triangles;
		for(unsigned char x = 0; x < width; ++x) {
			for(unsigned char y = 0; y < height; ++y) {
				for(unsigned char z = 0; z < depth; ++z) {
					auto current = z * width * height + x * height + y;
					if(blocks.at(current)) {
						Point offset(x, y, -z, 0);
						if(x == 0 || !blocks.at(current - height)) {
							for(auto &triangle : blockLeft) {
								triangles.emplace_back(std::get<0>(triangle) +offset, std::get<1>(triangle) +offset, std::get<2>(triangle) +offset);
							}
						}
						if(x == width - 1 || !blocks.at(current + height)) {
							for(auto &triangle : blockRight) {
								triangles.emplace_back(std::get<0>(triangle) +offset, std::get<1>(triangle) +offset, std::get<2>(triangle) +offset);
							}
						}
						if(y == 0 || !blocks.at(current - 1)) {
							for(auto &triangle : blockBottom) {
								triangles.emplace_back(std::get<0>(triangle) +offset, std::get<1>(triangle) +offset, std::get<2>(triangle) +offset);
							}
						}
						if(y == height - 1 || !blocks.at(current + 1)) {
							for(auto &triangle : blockTop) {
								triangles.emplace_back(std::get<0>(triangle) +offset, std::get<1>(triangle) +offset, std::get<2>(triangle) +offset);
							}
						}
						if(z == 0 || !blocks.at(current - width * height)) {
							for(auto &triangle : blockFront) {
								triangles.emplace_back(std::get<0>(triangle) +offset, std::get<1>(triangle) +offset, std::get<2>(triangle) +offset);
							}
						}
						if(z == depth - 1 || !blocks.at(current + width * height)) {
							for(auto &triangle : blockBack) {
								triangles.emplace_back(std::get<0>(triangle) +offset, std::get<1>(triangle) +offset, std::get<2>(triangle) +offset);
							}
						}
					}
				}
			}
		}
		Add<ModelComponent>(triangles);
	}
};