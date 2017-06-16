#pragma once

#include <vector>
#include "ModelComponent.h"
#include "Serializer.h"

struct Block {
	bool state = false;
	float hardness = 1.0f;
	float health = 1.0f;

	Block(const bool state = false) : state(state) {}
};

inline Serializer & operator<<(Serializer &s, const Block &block) {
	return s << block.state << block.hardness << block.health;
}

inline Deserializer & operator>>(Deserializer &s, Block &block) {
	return s >> block.state >> block.hardness >> block.health;
}

class Chunk : public ModelComponent {
public:
	unsigned char width;
	unsigned char height;
	unsigned char depth;
	std::vector<Block> blocks;

	Chunk(const unsigned char &width, const unsigned char &height, const unsigned char &depth) : width(width), height(height), depth(depth) {}

	void Generate(const Point3 &blockSize) {
		type = GeometryType::Triangles;
		const std::vector<Triangle> blockLeft = {
			std::make_tuple(Point3(-0.5, -0.5, -0.5), Point3(-0.5, -0.5,  0.5), Point3(-0.5,  0.5, -0.5)),
			std::make_tuple(Point3(-0.5, -0.5,  0.5), Point3(-0.5,  0.5,  0.5), Point3(-0.5,  0.5, -0.5))
		};
		const std::vector<Triangle> blockRight = {
			std::make_tuple(Point3( 0.5, -0.5,  0.5), Point3( 0.5, -0.5, -0.5), Point3( 0.5,  0.5,  0.5)),
			std::make_tuple(Point3( 0.5, -0.5, -0.5), Point3( 0.5,  0.5, -0.5), Point3( 0.5,  0.5,  0.5))
		};
		const std::vector<Triangle> blockBottom = {
			std::make_tuple(Point3(-0.5, -0.5, -0.5), Point3( 0.5, -0.5, -0.5), Point3(-0.5, -0.5,  0.5)),
			std::make_tuple(Point3( 0.5, -0.5, -0.5), Point3( 0.5, -0.5,  0.5), Point3(-0.5, -0.5,  0.5))
		};
		const std::vector<Triangle> blockTop = {
			std::make_tuple(Point3(-0.5,  0.5,  0.5), Point3( 0.5,  0.5,  0.5), Point3(-0.5,  0.5, -0.5)),
			std::make_tuple(Point3( 0.5,  0.5,  0.5), Point3( 0.5,  0.5, -0.5), Point3(-0.5,  0.5, -0.5))
		};
		const std::vector<Triangle> blockBack = {
			std::make_tuple(Point3(-0.5,  0.5, -0.5), Point3( 0.5,  0.5, -0.5), Point3(-0.5, -0.5, -0.5)),
			std::make_tuple(Point3( 0.5,  0.5, -0.5), Point3( 0.5, -0.5, -0.5), Point3(-0.5, -0.5, -0.5))
		};
		const std::vector<Triangle> blockFront = {
			std::make_tuple(Point3(-0.5, -0.5,  0.5), Point3( 0.5, -0.5,  0.5), Point3(-0.5,  0.5,  0.5)),
			std::make_tuple(Point3( 0.5, -0.5,  0.5), Point3( 0.5,  0.5,  0.5), Point3(-0.5,  0.5,  0.5))
		};

		points.reserve(width * height * depth * 3);

		for(unsigned char x = 0; x < width; ++x) {
			for(unsigned char y = 0; y < height; ++y) {
				for(unsigned char z = 0; z < depth; ++z) {
					auto current = z * width * height + x * height + y;
					if(blocks.at(current).state) {
						Point3 offset(x + 0.5f, y + 0.5f, z + 0.5f);

						bool left   = x > 0          && blocks.at(current - height).state;
						bool right  = x < width - 1  && blocks.at(current + height).state;
						bool bottom = y > 0          && blocks.at(current - 1).state;
						bool top    = y < height - 1 && blocks.at(current + 1).state;
						bool back   = z > 0          && blocks.at(current - width * height).state;
						bool front  = z < depth - 1  && blocks.at(current + width * height).state;

						/*if(!left && right) {
							if(!bottom && top) {
								if(!back && front) {
									points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
								} else if(!front && back) {
									points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
								} else {
									points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
								}
								continue;
							} else if(!top && bottom) {
								if(!back && front) {
									points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
								} else if(!front && back) {
									points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
								} else {
									points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
								}
								continue;
							} else if(!back && front) {
								points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
								continue;
							} else if(!front && back) {
								points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
								continue;
							}
						} else if(!right && left) {
							if(!bottom && top) {
								if(!back && front) {
									points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
								} else if(!front && back) {
									points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
								} else {
									points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
								}
								continue;
							} else if(!top && bottom) {
								if(!back && front) {
									points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
								} else if(!front && back) {
									points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
								} else {
									points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
									points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
									points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
								}
								continue;
							} else if(!back && front) {
								points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
								continue;
							} else if(!front && back) {
								points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
								continue;
							}
						} else if(!bottom && top) {
							if(!back && front) {
								points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
								continue;
							} else if(!front && back) {
								points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
								continue;
							}
						} else if(!top && bottom) {
							if(!back && front) {
								points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5,  0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5, -0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5, -0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5,  0.5,  0.5) + offset) * blockSize);
								continue;
							} else if(!front && back) {
								points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5,  0.5, -0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3(-0.5, -0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5, -0.5,  0.5) + offset) * blockSize);
								points.emplace_back((Point3( 0.5,  0.5, -0.5) + offset) * blockSize);
								continue;
							}
						}*/

						if(!left) {
							for(auto &triangle : blockLeft) {
								points.emplace_back((std::get<0>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<1>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<2>(triangle) +offset) * blockSize);
							}
						}
						if(!right) {
							for(auto &triangle : blockRight) {
								points.emplace_back((std::get<0>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<1>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<2>(triangle) +offset) * blockSize);
							}
						}
						if(!bottom) {
							for(auto &triangle : blockBottom) {
								points.emplace_back((std::get<0>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<1>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<2>(triangle) +offset) * blockSize);
							}
						}
						if(!top) {
							for(auto &triangle : blockTop) {
								points.emplace_back((std::get<0>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<1>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<2>(triangle) +offset) * blockSize);
							}
						}
						if(!back) {
							for(auto &triangle : blockBack) {
								points.emplace_back((std::get<0>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<1>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<2>(triangle) +offset) * blockSize);
							}
						}
						if(!front) {
							for(auto &triangle : blockFront) {
								points.emplace_back((std::get<0>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<1>(triangle) +offset) * blockSize);
								points.emplace_back((std::get<2>(triangle) +offset) * blockSize);
							}
						}
					}
				}
			}
		}
	}
};

inline Serializer & operator<<(Serializer &s, const Chunk &chunk) {
	s << chunk.width << chunk.height << chunk.depth;
	for(auto &block : chunk.blocks) {
		s << block;
	}
	return s;
}

inline Deserializer & operator>>(Deserializer &s, Chunk &chunk) {
	s >> chunk.width >> chunk.height >> chunk.depth;
	chunk.blocks.resize(chunk.width * chunk.height * chunk.depth);

	for(int i = 0; i < chunk.width * chunk.height * chunk.depth; ++i) {
		s >> chunk.blocks[i];
	}
	return s;
}
