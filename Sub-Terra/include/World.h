#pragma once

#include <boost/container/vector.hpp>
#include <boost/unordered_map.hpp>
#include "System.h"
#include "OpenSimplexNoise.h"

enum class ChunkStatus {
	Generating,
	Alive,
	Dying,
	Dead
};

typedef std::tuple<int64_t, int64_t, int64_t> ChunkKeyType;
typedef std::tuple<ChunkStatus, IDType> ChunkContainerType;
typedef boost::unordered_map<ChunkKeyType, ChunkContainerType> ChunksType;

class World : public System {
private:
	const uint8_t viewDistance = 3;
	const Point3 blockSize = Point3(1.0f);
	const glm::ivec3 chunkSize;
	OpenSimplexNoise noise = OpenSimplexNoise(std::random_device()());
	Atomic<ChunksType> chunks;
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
public:
	static bool IsSupported() { return true; }
	World(Polar *engine, const unsigned char chunkWidth, const unsigned char chunkHeight, const unsigned char chunkDepth)
		: System(engine), chunkSize(chunkWidth, chunkHeight, chunkDepth) {}
	~World();

	inline Point3 BlockCoordForPos(Point3 &pos) const {
		return glm::floor(pos / blockSize);
	}
	inline Point3 ChunkCoordForPos(Point3 &pos) const {
		return glm::floor(pos / (blockSize * Point3(chunkSize)));
	}
	inline boost::container::vector<bool> GenerateChunk(const Point3 &p) const {
		boost::container::vector<bool> blocks;
		blocks.resize(chunkSize.x * chunkSize.y * chunkSize.z);

		Point3 actual = p * Point3(blockSize.x * chunkSize.x, blockSize.y * chunkSize.y, blockSize.z * chunkSize.z);

		int i = 0;
		for(float z = 0; z < blockSize.z * chunkSize.z; z += blockSize.z) {
			for(float x = 0; x < blockSize.x * chunkSize.x; x += blockSize.x) {
				for(float y = 0; y < blockSize.y * chunkSize.y; y += blockSize.y) {
					blocks.at(i++) = GenerateBlock(actual + Point3(x, y, z));
				}
			}
		}
		return blocks;
	}

	inline bool GenerateBlock(const Point3 &&p) const {
		/* periodic 'normal distribution' function */
		auto fDensity = [] (const double x) {
			return 1.0 - glm::abs(glm::sin(x));
		};

		const float scale1 = 17.0f;
		const float scale2 = scale1 / 17.0f * 8.0f;
		auto eval1 = noise.eval(p.x / scale1, p.y * 1.6f / scale1, p.z / scale1);
		auto eval2 = noise.eval(p.x / scale2, p.y / scale2 / 3.0f, p.z / scale2 / 5.0f);

		/* cave systems */
		auto result1 = (
			eval1 > (fDensity(p.x / scale1 / 32.0f) - 1) * 1.5f + 0.1f ||
			eval1 > (fDensity(p.y / scale1 / 10.0f) - 1) * 1.5f + 0.1f ||
			eval1 > (fDensity(p.z / scale1 / 32.0f) - 1) * 1.5f + 0.1f
			);
		/* cave system crevices */
		auto result2 = (
			eval1 > (fDensity(p.x / scale1 / 32.0f) - 1) * 1.5f + 0.1f ||
			eval1 > (fDensity(p.y / scale1 / 10.0f + noise.eval(p.x / scale1 / 60.0f, p.z / scale1 / 60.0f) * 100.0f) - 1) * 1.5f + 0.1f ||
			eval1 > (fDensity(p.z / scale1 / 32.0f) - 1) * 1.5f + 0.1f
			);
		/* crevices */
		auto result3 = eval2 > -0.75;

		/* AND results so there is only a block if no results dictate no block */
		return result1 && result2 && result3;
	}
};
