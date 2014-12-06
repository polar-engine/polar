#pragma once

#include "System.h"
#include "Chunk.h"

typedef std::tuple<int64_t, int64_t, int64_t> ChunkKey;

struct ChunkKeyHash : public std::unary_function<ChunkKey, std::size_t> {
	std::size_t operator()(const ChunkKey &k) const {
		return static_cast<size_t>(std::get<0>(k) * 32 * 32 + std::get<1>(k) * 32 + std::get<2>(k));
	}
};

struct ChunkKeyEqual : public std::binary_function<ChunkKey, ChunkKey, bool> {
	bool operator()(const ChunkKey &v0, const ChunkKey &v1) const {
		return (
			std::get<0>(v0) == std::get<0>(v1) &&
			std::get<1>(v0) == std::get<1>(v1) &&
			std::get<2>(v0) == std::get<2>(v1)
		);
	}
};

class World : public System {
private:
	const glm::ivec3 chunkSize;

	OpenSimplexNoise noise = OpenSimplexNoise(std::random_device()());
	std::unordered_map<ChunkKey, Chunk *, ChunkKeyHash, ChunkKeyEqual> chunks;
	Object *cameraObj = nullptr;
protected:
	void Init() override final;
	void Update(DeltaTicks &, std::vector<Object *> &) override final;
	void Destroy() override final;
	void ObjectAdded(Object *) override final;
public:
	static bool IsSupported() { return true; }
	World(Polar *engine, const unsigned char chunkWidth, const unsigned char chunkHeight, const unsigned char chunkDepth)
		: System(engine), chunkSize(chunkWidth, chunkHeight, chunkDepth) {}
	std::vector<bool> Generate(const ChunkKey &);
};
