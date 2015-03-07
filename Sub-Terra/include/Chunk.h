#pragma once

#include "ModelComponent.h"
#include "OpenSimplexNoise.h"

class Chunk : public ModelComponent {
public:
	Chunk(unsigned char width, unsigned char height, unsigned char depth, const std::vector<bool> &blocks) {
		type = GeometryType::Triangles;
		const std::vector<Triangle> blockFront = {
			std::make_tuple(Point3(-0.5, -0.5,  0.5), Point3( 0.5, -0.5,  0.5), Point3(-0.5,  0.5,  0.5)),
			std::make_tuple(Point3( 0.5, -0.5,  0.5), Point3( 0.5,  0.5,  0.5), Point3(-0.5,  0.5,  0.5))
		};
		const std::vector<Triangle> blockTop = {
			std::make_tuple(Point3(-0.5,  0.5,  0.5), Point3( 0.5,  0.5,  0.5), Point3(-0.5,  0.5, -0.5)),
			std::make_tuple(Point3( 0.5,  0.5,  0.5), Point3( 0.5,  0.5, -0.5), Point3(-0.5,  0.5, -0.5))
		};
		const std::vector<Triangle> blockBack = {
			std::make_tuple(Point3(-0.5,  0.5, -0.5), Point3( 0.5,  0.5, -0.5), Point3(-0.5, -0.5, -0.5)),
			std::make_tuple(Point3( 0.5,  0.5, -0.5), Point3( 0.5, -0.5, -0.5), Point3(-0.5, -0.5, -0.5))
		};
		const std::vector<Triangle> blockBottom = {
			std::make_tuple(Point3(-0.5, -0.5, -0.5), Point3( 0.5, -0.5, -0.5), Point3(-0.5, -0.5,  0.5)),
			std::make_tuple(Point3( 0.5, -0.5, -0.5), Point3( 0.5, -0.5,  0.5), Point3(-0.5, -0.5,  0.5))
		};
		const std::vector<Triangle> blockRight = {
			std::make_tuple(Point3( 0.5, -0.5,  0.5), Point3( 0.5, -0.5, -0.5), Point3( 0.5,  0.5,  0.5)),
			std::make_tuple(Point3( 0.5, -0.5, -0.5), Point3( 0.5,  0.5, -0.5), Point3( 0.5,  0.5,  0.5))
		};
		const std::vector<Triangle> blockLeft = {
			std::make_tuple(Point3(-0.5, -0.5, -0.5), Point3(-0.5, -0.5,  0.5), Point3(-0.5,  0.5, -0.5)),
			std::make_tuple(Point3(-0.5, -0.5,  0.5), Point3(-0.5,  0.5,  0.5), Point3(-0.5,  0.5, -0.5))
		};

		points.reserve(width * height * depth * 3);

		for(unsigned char x = 0; x < width; ++x) {
			for(unsigned char y = 0; y < height; ++y) {
				for(unsigned char z = 0; z < depth; ++z) {
					auto current = z * width * height + x * height + y;
					if(blocks.at(current)) {
						Point3 offset(x, y, -z);
						if(x == 0 || !blocks.at(current - height)) {
							for(auto &triangle : blockLeft) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
							}
						}
						if(x == width - 1 || !blocks.at(current + height)) {
							for(auto &triangle : blockRight) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
							}
						}
						if(y == 0 || !blocks.at(current - 1)) {
							for(auto &triangle : blockBottom) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
							}
						}
						if(y == height - 1 || !blocks.at(current + 1)) {
							for(auto &triangle : blockTop) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
							}
						}
						if(z == 0 || !blocks.at(current - width * height)) {
							for(auto &triangle : blockFront) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
							}
						}
						if(z == depth - 1 || !blocks.at(current + width * height)) {
							for(auto &triangle : blockBack) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
							}
						}
					}
				}
			}
		}
	}
};
