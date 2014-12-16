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

		Add<ModelComponent>();
		auto model = Get<ModelComponent>();
		auto &points = model->points;
		auto &normals = model->normals;
		points.reserve(width * height * depth);
		normals.reserve(width * height * depth);

		for(unsigned char x = 0; x < width; ++x) {
			for(unsigned char y = 0; y < height; ++y) {
				for(unsigned char z = 0; z < depth; ++z) {
					auto current = z * width * height + x * height + y;
					if(blocks.at(current)) {
						Point offset(x, y, -z, 0);
						if(x == 0 || !blocks.at(current - height)) {
							for(auto &triangle : blockLeft) {
								points.emplace_back(std::get<0>(triangle) + offset);
								points.emplace_back(std::get<1>(triangle) + offset);
								points.emplace_back(std::get<2>(triangle) + offset);
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
						if(x == width - 1 || !blocks.at(current + height)) {
							for(auto &triangle : blockRight) {
								points.emplace_back(std::get<0>(triangle) + offset);
								points.emplace_back(std::get<1>(triangle) + offset);
								points.emplace_back(std::get<2>(triangle) + offset);
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
						if(y == 0 || !blocks.at(current - 1)) {
							for(auto &triangle : blockBottom) {
								points.emplace_back(std::get<0>(triangle) + offset);
								points.emplace_back(std::get<1>(triangle) + offset);
								points.emplace_back(std::get<2>(triangle) + offset);
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
						if(y == height - 1 || !blocks.at(current + 1)) {
							for(auto &triangle : blockTop) {
								points.emplace_back(std::get<0>(triangle) + offset);
								points.emplace_back(std::get<1>(triangle) + offset);
								points.emplace_back(std::get<2>(triangle) + offset);
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
						if(z == 0 || !blocks.at(current - width * height)) {
							for(auto &triangle : blockFront) {
								points.emplace_back(std::get<0>(triangle) + offset);
								points.emplace_back(std::get<1>(triangle) + offset);
								points.emplace_back(std::get<2>(triangle) + offset);
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
						if(z == depth - 1 || !blocks.at(current + width * height)) {
							for(auto &triangle : blockBack) {
								points.emplace_back(std::get<0>(triangle) + offset);
								points.emplace_back(std::get<1>(triangle) + offset);
								points.emplace_back(std::get<2>(triangle) + offset);
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
					}
				}
			}
		}
	}
};
