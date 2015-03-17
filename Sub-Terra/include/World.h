#pragma once

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
	const Point3 blockSize = Point3(1.0f);
	const glm::ivec3 chunkSize;
	OpenSimplexNoise noise = OpenSimplexNoise(std::random_device()());
	Atomic<ChunksType> chunks;
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
	void Destroy() override final;
public:
	static bool IsSupported() { return true; }
	World(Polar *engine, const unsigned char chunkWidth, const unsigned char chunkHeight, const unsigned char chunkDepth)
		: System(engine), chunkSize(chunkWidth, chunkHeight, chunkDepth) {}
	std::vector<bool> GenerateChunk(const Point3 &&) const;
	bool GenerateBlock(const Point3 &&) const;
};
