#pragma once

#include "System.h"
#include "Chunk.h"

typedef std::tuple<int64_t, int64_t, int64_t> ChunkKeyType;

struct ChunkKeyHash : public std::unary_function<ChunkKeyType, std::size_t> {
	std::size_t operator()(const ChunkKeyType &k) const {
		return static_cast<size_t>((std::get<0>(k) << 8) + (std::get<1>(k) << 4) + std::get<2>(k));
	}
};

struct ChunkKeyEqual : public std::binary_function<ChunkKeyType, ChunkKeyType, bool> {
	bool operator()(const ChunkKeyType &v0, const ChunkKeyType &v1) const {
		return (
			std::get<0>(v0) == std::get<0>(v1) &&
			std::get<1>(v0) == std::get<1>(v1) &&
			std::get<2>(v0) == std::get<2>(v1)
		);
	}
};

enum class ChunkStatus {
	Generating,
	Alive,
	Dying,
	Dead
};

typedef std::tuple<ChunkStatus, Chunk *> ChunkContainerType;
typedef std::unordered_map<ChunkKeyType, ChunkContainerType, ChunkKeyHash, ChunkKeyEqual> ChunksType;

class World : public System {
private:
	const glm::ivec3 chunkSize;
	OpenSimplexNoise noise = OpenSimplexNoise(std::random_device()());
	Atomic<ChunksType> chunks;
	Object *cameraObj = nullptr;
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
	void Destroy() override final;
	void ObjectAdded(Object *) override final;
public:
	static bool IsSupported() { return true; }
	World(Polar *engine, const unsigned char chunkWidth, const unsigned char chunkHeight, const unsigned char chunkDepth)
		: System(engine), chunkSize(chunkWidth, chunkHeight, chunkDepth) {}
	std::vector<bool> GenerateChunk(const Point3 &&) const;
	bool GenerateBlock(const Point3 &&) const;
};
