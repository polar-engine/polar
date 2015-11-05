#pragma once

#include <atomic>
#include <boost/container/vector.hpp>
#include <boost/unordered_map.hpp>
#include "FileSystem.h"
#include "System.h"
#include "JobManager.h"
#include "PlayerCameraComponent.h"
#include "PositionComponent.h"
#include "BoundingComponent.h"
#include "Chunk.h"
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
	std::atomic_bool exists;
	OpenSimplexNoise noise, noise2;
	Atomic<ChunksType> chunks;
	boost::unordered_map<ChunkKeyType, IDType> pickups;
protected:
	void Init() override final;
	void Update(DeltaTicks &) override final;
public:
	float factor = 1;
	const uint8_t viewDistance = 2;
	const Point3 blockSize;
	const glm::ivec3 chunkSize;

	static bool IsSupported() { return true; }

	World(Polar *engine, const Point3 &blockSize, const unsigned char chunkWidth, const unsigned char chunkHeight, const unsigned char chunkDepth)
		: System(engine), blockSize(blockSize), chunkSize(chunkWidth, chunkHeight, chunkDepth) {
		exists = true;
	}

	~World() {
		exists = false;
	}

	inline boost::container::vector<Block>::size_type BlockIndexForCoord(const glm::ivec3 &p) {
		return BlockIndexForCoord(p.x, p.y, p.z);
	}

	inline boost::container::vector<Block>::size_type BlockIndexForCoord(const unsigned char &x, const unsigned char &y, const unsigned char &z) {
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

	inline ChunkKeyType ChunkKeyForChunkCoord(const Point3 &coord) const {
		return std::make_tuple(static_cast<uint64_t>(coord.x),
		                       static_cast<uint64_t>(coord.y),
		                       static_cast<uint64_t>(coord.z));
	}

	inline Block GetBlock(const Point3 &coord) {
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
		auto self = std::shared_ptr<World>(this);

		auto destroy = chunks.With<bool>([self, &coord, &damage] (ChunksType &chunks) {
			Point3 chunkCoord, blockCoord;
			std::tie(chunkCoord, blockCoord) = self->CoordsForBlockCoord(coord);
			auto chunkTuple = self->ChunkKeyForChunkCoord(chunkCoord);
			auto &container = chunks.at(chunkTuple);
			auto chunk = static_cast<Chunk *>(self->engine->GetComponent<ModelComponent>(std::get<1>(container)));

			auto index = self->BlockIndexForCoord(glm::ivec3(blockCoord));
			return (chunk->blocks.at(index).health -= damage) < 0.0f;
		});

		if(destroy) { SetBlock(coord, Block()); }
		return destroy;
	}

	inline const ChunkContainerType GetChunkContainer(const Point3 &coord) {
		auto chunkTuple = ChunkKeyForChunkCoord(coord);
		return chunks.With<ChunkContainerType>([&chunkTuple] (ChunksType &chunks) {
			return chunks.at(chunkTuple);
		});
	}

	inline const Chunk * GetChunk(const Point3 &coord) {
		auto container = GetChunkContainer(coord);
		return static_cast<Chunk *>(engine->GetComponent<ModelComponent>(std::get<1>(container)));
	}

	inline Chunk * CreateChunk(const Point3 &coord, const boost::container::vector<Block> &blocks, const bool &deferredToMain = false) {
		auto self = std::shared_ptr<World>(this);

		auto pos = new PositionComponent(PosForChunkCoord(coord));
		auto chunk = new Chunk(chunkSize.x, chunkSize.y, chunkSize.z);
		chunk->blocks = blocks;
		chunk->Generate(blockSize);
		auto bounds = new BoundingComponent(Point3(0.0f), Point3(chunkSize), true);

		/* add all block bounding boxes in chunk as children */
		for(unsigned char x = 0; x < chunkSize.x; ++x) {
			for(unsigned char y = 0; y < chunkSize.y; ++y) {
				for(unsigned char z = 0; z < chunkSize.z; ++z) {
					if(blocks.at(z * chunkSize.x * chunkSize.y + x * chunkSize.y + y).state == true) {
						bounds->box.children.emplace_back(PosForBlockCoord(Point3(x, y, z)), blockSize);
					}
				}
			}
		}

		auto chunkTuple = ChunkKeyForChunkCoord(coord);
		if(deferredToMain) {
			if(!exists) { return chunk; }

			auto weak = engine->GetSystem<JobManager>();
			if(weak.expired()) { return chunk; }

			auto jobM = weak.lock();
			jobM->Do([self, chunkTuple, pos, chunk, bounds] () {
				if(!self->exists) { return; }

				IDType id;
				self->dtors.emplace_back(self->engine->AddObject(&id));
				self->engine->AddComponent<TagComponent<Chunk>>(id);
				self->engine->InsertComponent<PositionComponent>(id, pos);
				self->engine->InsertComponent<ModelComponent>(id, chunk);
				self->engine->InsertComponent<BoundingComponent>(id, bounds);
				self->chunks.With([&chunkTuple, id] (ChunksType &chunks) {
					if(chunks.find(chunkTuple) != chunks.end()) {
						chunks.at(chunkTuple) = std::make_tuple(ChunkStatus::Alive, id);
					}
				});
			}, JobPriority::High, JobThread::Main);
		} else {
			IDType id;
			dtors.emplace_back(engine->AddObject(&id));
			engine->InsertComponent<PositionComponent>(id, pos);
			engine->InsertComponent<ModelComponent>(id, chunk);
			engine->InsertComponent<BoundingComponent>(id, bounds);
			chunks.With([&chunkTuple, id] (ChunksType &chunks) {
				chunks[chunkTuple] = std::make_tuple(ChunkStatus::Alive, id);
			});
		}

		return chunk;
	}

	inline void DestroyChunk(const Point3 &coord, const bool &deferredToMain = false) { 
		auto self = std::shared_ptr<World>(this);

		auto chunkTuple = ChunkKeyForChunkCoord(coord);
		ChunkContainerType container = chunks.With<ChunkContainerType>([&chunkTuple] (ChunksType &chunks) {
			auto container = chunks.at(chunkTuple);
			chunks.erase(chunkTuple);
			return container;
		});
		if(deferredToMain) {
			auto jobM = engine->GetSystem<JobManager>().lock();
			jobM->Do([self, container] () { self->engine->RemoveObject(std::get<1>(container)); }, JobPriority::Low, JobThread::Main);
		} else {
			engine->RemoveObject(std::get<1>(container));
		}
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
		const float scale2 = scale1 / 17.0f * 8.0f;
		auto eval1 = noise.eval(p.x / scale1, p.y / scale1, p.z / scale1);
		auto eval2 = noise2.eval(p.x / scale1, p.y / scale1, p.z / scale1);

		return eval1 > 0.25 || eval2 > 0.25 + 0.75 * factor;
	}
};
