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

		std::vector<Triangle> triangles;
		triangles.reserve(width * height * depth);
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
