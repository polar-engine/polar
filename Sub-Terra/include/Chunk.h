#pragma once

#include <deque>
#include "Object.h"
#include "PositionComponent.h"
#include "ModelComponent.h"
#include "OpenSimplexNoise.h"

class Chunk : public Object {
public:
	typedef std::deque<Chunk *> pool_type;
private:
	static Atomic<pool_type> pool;

	Chunk() {
		Add<PositionComponent>();
		Add<ModelComponent>();
	}
public:
	template<typename ...Ts> static inline Chunk * New(Ts && ...args) {
		auto obj = pool.With<Chunk *>([] (pool_type &pool) {
			if(pool.empty()) {
				return new Chunk();
			} else {
				auto obj = pool.front();
				pool.pop_front();
				return obj;
			}
		});
		obj->Construct(std::forward<Ts>(args)...);
		return obj;
	}

	inline void Pool() {
		pool.With([this] (pool_type &pool) {
			pool.push_back(this);
		});
	}

	inline void Construct(unsigned char width, unsigned char height, unsigned char depth, const std::vector<bool> &blocks) {
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

		auto model = Get<ModelComponent>();
		auto &points = model->points;
		auto &normals = model->normals;

		points.clear();
		normals.clear();
		points.reserve(width * height * depth * 3);
		normals.reserve(width * height * depth * 3);

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
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
						if(x == width - 1 || !blocks.at(current + height)) {
							for(auto &triangle : blockRight) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
						if(y == 0 || !blocks.at(current - 1)) {
							for(auto &triangle : blockBottom) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
						if(y == height - 1 || !blocks.at(current + 1)) {
							for(auto &triangle : blockTop) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
						if(z == 0 || !blocks.at(current - width * height)) {
							for(auto &triangle : blockFront) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
								auto normal = ModelComponent::CalculateNormal(triangle);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
								normals.emplace_back(normal);
							}
						}
						if(z == depth - 1 || !blocks.at(current + width * height)) {
							for(auto &triangle : blockBack) {
								points.emplace_back(std::get<0>(triangle) +offset);
								points.emplace_back(std::get<1>(triangle) +offset);
								points.emplace_back(std::get<2>(triangle) +offset);
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
