#pragma once

#include <atomic>
#include <boost/container/vector.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "FileSystem.h"
#include "System.h"
#include "JobManager.h"
#include "PlayerCameraComponent.h"
#include "PositionComponent.h"
#include "BoundingComponent.h"
#include "Chunk.h"
#include "OpenSimplexNoise.h"

class World : public System {
public:
	typedef std::tuple<int64_t, int64_t, int64_t> KeyType;
	typedef std::tuple<boost::shared_ptr<Destructor>, IDType> ContainerType;
private:
	OpenSimplexNoise noise = OpenSimplexNoise(std::random_device()());
	OpenSimplexNoise noise2 = OpenSimplexNoise(std::random_device()());
	boost::unordered_set<IDType> cameras;
	boost::unordered_map<KeyType, ContainerType> chunks;
	boost::unordered_map<KeyType, ContainerType> pickups;
protected:
	void Update(DeltaTicks &) override final;
public:
	float factor = 1;
	const uint8_t viewDistance = 4;
	const Point3 blockSize;
	const Point3i chunkSize;

	static bool IsSupported() { return true; }

	World(Polar *engine, const Point3 &blockSize, const unsigned char chunkWidth, const unsigned char chunkHeight, const unsigned char chunkDepth)
		: System(engine), blockSize(blockSize), chunkSize(chunkWidth, chunkHeight, chunkDepth) {}

	inline void ComponentAdded(IDType id, const std::type_info *ti, boost::weak_ptr<Component> ptr) override final {
		if(ti != &typeid(PlayerCameraComponent)) { return; }
		cameras.emplace(id);
	}

	inline void ComponentRemoved(IDType id, const std::type_info *ti) override final {
		if(ti != &typeid(PlayerCameraComponent)) { return; }
		cameras.erase(id);
	}

	inline boost::container::vector<Block>::size_type BlockIndexForCoord(const glm::ivec3 &p) const {
		return BlockIndexForCoord(p.x, p.y, p.z);
	}

	inline boost::container::vector<Block>::size_type BlockIndexForCoord(const unsigned char &x, const unsigned char &y, const unsigned char &z) const {
		return z * chunkSize.x * chunkSize.y + x * chunkSize.y + y;
	}

	inline Point3 BlockCoordForPos(const Point3 &pos) const {
		return glm::floor(pos / blockSize);
	}

	inline Point3 ChunkCoordForPos(const Point3 &pos) const {
		return glm::floor(pos / (blockSize * Point3(chunkSize)));
	}

	inline Point3 PosForBlockCoord(const Point3 &coord) const {
		return coord * blockSize;
	}

	inline Point3 PosForChunkCoord(const Point3 &coord) const {
		return coord * blockSize * Point3(chunkSize);
	}

	inline Point3 BlockCoordForChunkCoord(const Point3 &coord) const {
		return coord * Point3(chunkSize);
	}

	inline Point3 ChunkCoordForBlockCoord(const Point3 &coord) const {
		return glm::floor(coord / Point3(chunkSize));
	}

	inline std::pair<Point3, Point3> CoordsForBlockCoord(const Point3 &coord) const {
		auto chunkCoord = ChunkCoordForBlockCoord(coord);
		return std::make_pair(chunkCoord, coord - BlockCoordForChunkCoord(chunkCoord));
	}

	inline KeyType ChunkKeyForChunkCoord(const Point3 &coord) const {
		return std::make_tuple(static_cast<uint64_t>(coord.x),
		                       static_cast<uint64_t>(coord.y),
		                       static_cast<uint64_t>(coord.z));
	}

	inline Block GetBlock(const Point3 &coord) const {
		Point3 chunkCoord, blockCoord;
		std::tie(chunkCoord, blockCoord) = CoordsForBlockCoord(coord);
		auto blocks = GetChunk(chunkCoord)->blocks;
		return blocks[BlockIndexForCoord(glm::ivec3(blockCoord))];
	}

	inline void SetBlock(const Point3 &coord, const Block &block) {
		Point3 chunkCoord, blockCoord;
		std::tie(chunkCoord, blockCoord) = CoordsForBlockCoord(coord);
		auto chunk = *GetChunk(chunkCoord);

		chunk.blocks[BlockIndexForCoord(glm::ivec3(blockCoord))] = block;
		DestroyChunk(chunkCoord);
		CreateChunk(chunkCoord, chunk.blocks);
	}

	inline bool DamageBlock(const Point3 &coord, const float &damage) {
		Point3 chunkCoord, blockCoord;
		std::tie(chunkCoord, blockCoord) = CoordsForBlockCoord(coord);
		auto chunkTuple = ChunkKeyForChunkCoord(chunkCoord);
		auto &container = chunks.at(chunkTuple);
		auto chunk = static_cast<Chunk *>(engine->GetComponent<ModelComponent>(std::get<1>(container)));

		auto index = BlockIndexForCoord(glm::ivec3(blockCoord));
		auto destroy = (chunk->blocks.at(index).health -= damage) < 0.0f;

		if(destroy) { SetBlock(coord, Block()); }
		return destroy;
	}

	inline const ContainerType GetChunkContainer(const Point3 &coord) const {
		return chunks.at(ChunkKeyForChunkCoord(coord));
	}

	inline const Chunk * GetChunk(const Point3 &coord) const {
		auto container = GetChunkContainer(coord);
		return static_cast<Chunk *>(engine->GetComponent<ModelComponent>(std::get<1>(container)));
	}

	inline void CreateChunk(const Point3 &coord, const boost::container::vector<Block> &blocks) {
		auto chunk = boost::make_shared<Chunk>(chunkSize.x, chunkSize.y, chunkSize.z);
		chunk->blocks = blocks;
		chunk->Generate(blockSize);

		/* add all block bounding boxes in chunk as children */
		auto bounds = boost::make_shared<BoundingComponent>(Point3(0.0f), Point3(chunkSize), true);
		for(unsigned char x = 0; x < chunkSize.x; ++x) {
			for(unsigned char y = 0; y < chunkSize.y; ++y) {
				for(unsigned char z = 0; z < chunkSize.z; ++z) {
					if(blocks.at(z * chunkSize.x * chunkSize.y + x * chunkSize.y + y).state == true) {
						bounds->box.children.emplace_back(PosForBlockCoord(Point3(x, y, z)), blockSize);
					}
				}
			}
		}

		IDType id;
		auto dtor = engine->AddObject(&id);
		engine->AddComponent<TagComponent<Chunk>>(id);
		engine->AddComponent<PositionComponent>(id, PosForChunkCoord(coord));
		engine->InsertComponent<ModelComponent>(id, chunk);
		engine->InsertComponent<BoundingComponent>(id, bounds);
		chunks[ChunkKeyForChunkCoord(coord)] = std::make_tuple(dtor, id);
	}

	inline void DestroyChunk(const Point3 &coord) {
		auto chunkTuple = ChunkKeyForChunkCoord(coord);
		auto container = chunks.at(chunkTuple);
		chunks.erase(chunkTuple);
	}

	inline void SetBlockAtPos(const Point3 &pos, const bool &value) {
		SetBlock(BlockCoordForPos(pos), value);
	}

	inline boost::container::vector<Block> GenerateChunk(const Point3 &p) const {
		boost::container::vector<Block> blocks;
		blocks.resize(chunkSize.x * chunkSize.y * chunkSize.z);

		Point3 actual = p * Point3(blockSize.x * chunkSize.x, blockSize.y * chunkSize.y, blockSize.z * chunkSize.z);

		int i = 0;
		for(float z = 0; z < blockSize.z * chunkSize.z; z += blockSize.z) {
			for(float x = 0; x < blockSize.x * chunkSize.x; x += blockSize.x) {
				for(float y = 0; y < blockSize.y * chunkSize.y; y += blockSize.y) {
					blocks.at(i++).state = GenerateBlock(actual + Point3(x, y, z));
				}
			}
		}
		return blocks;
	}

	/* logo generation matching */
	inline bool Match(const Point3 &p) const {
		if(p.z == -25) {
			return true;
		} else if(p.z == -6) {
			if((p.y == -4 && p.x >=  1 && p.x <= 6) ||
			   (p.y == -5 && p.x ==  2) ||
			   (p.y == -3 && p.x ==  5)) { return true; } else { return false; }
		} else { return false; }
	}

	inline bool GenerateBlock(const Point3 &&p) const {
		const float scale1 = 17.0f;
		auto scaled = p / scale1;
		return noise.eval(scaled) > 0.25 || noise2.eval(scaled) > 0.25 + 0.75 * factor;
	}
};
