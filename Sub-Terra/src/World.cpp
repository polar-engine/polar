#include "common.h"
#include "World.h"
#include "PositionComponent.h"
#include "PlayerCameraComponent.h"

void World::Init() {
	
}

void World::Update(DeltaTicks &, std::vector<Object *> &) {
	if(cameraObj == nullptr) { return; }
	auto camera = cameraObj->Get<PlayerCameraComponent>();
	if(camera != nullptr) {
		auto pos = cameraObj->Get<PositionComponent>();
		if(pos != nullptr) {
			const unsigned char distance = 4;

			auto chunkSizeF = glm::fvec3(chunkSize);
			auto keyBase = glm::ivec3(glm::floor(pos->position.To<glm::fvec3>()) / chunkSizeF);
			auto jobM = engine->systems.Get<JobManager>();

			/* clean up chunks outside distance */
			chunks.With([this, distance, &keyBase] (ChunksType &chunks) {
				for(auto it = chunks.begin(); it != chunks.end();) {
					auto &keyTuple = it->first;
					auto obj = std::get<1>(it->second);

					if(abs(std::get<0>(keyTuple) -keyBase.x) > distance ||
					   abs(std::get<1>(keyTuple) -keyBase.y) > distance ||
					   abs(std::get<2>(keyTuple) -keyBase.z) > distance) {
						auto &status = std::get<0>(it->second);
						switch(status) {
						case ChunkStatus::Generating:
							status = ChunkStatus::Dying;
							break;
						case ChunkStatus::Alive:
						case ChunkStatus::Dead:
							if(obj != nullptr) {
								engine->RemoveObject(obj, false);
								obj->Pool();
							}
							chunks.erase(it++);
							continue;
						}
					}
					++it;
				}
			});

			for(int d = 0; d <= distance; ++d) {
				for(int x = -d; x <= d; ++x) {
					for(int y = -d; y <= d; ++y) {
						for(int z = -d; z <= d; ++z) {
							auto key = keyBase + glm::ivec3(x, y, z);
							auto keyTuple = std::make_tuple(key.x, key.y, key.z);

							if(chunks.With<bool>([&keyTuple] (ChunksType &chunks) {
								return chunks.find(keyTuple) == chunks.end();
							})) {
								chunks.With([&keyTuple] (ChunksType &chunks) {
									chunks.emplace(keyTuple, std::make_tuple(ChunkStatus::Generating, nullptr));
								});
								jobM->Do([this, jobM, key, keyTuple, chunkSizeF] () {
									auto dead = chunks.With<bool>([&keyTuple] (ChunksType &chunks) {
										auto &status = std::get<0>(chunks.at(keyTuple));
										if(status == ChunkStatus::Dying) {
											status = ChunkStatus::Dead;
											return true;
										} else { return false; }
									});
									if(dead) { return; }

									auto data = Generate(Point(std::get<0>(keyTuple), std::get<1>(keyTuple), std::get<2>(keyTuple), 1));
									auto chunkObj = Chunk::New(chunkSize.x, chunkSize.y, chunkSize.z, std::move(data));
									auto pos = chunkObj->Get<PositionComponent>();
									auto chunkPos = glm::fvec3(key) * chunkSizeF;
									pos->position = Point(chunkPos.x, chunkPos.y, chunkPos.z + chunkSize.z, 1);

									jobM->Do([this, keyTuple, key, chunkSizeF, chunkObj] () {
										engine->AddObject(chunkObj);
										chunks.With([&keyTuple, chunkObj] (ChunksType &chunks) {
											chunks.at(keyTuple) = std::make_tuple(ChunkStatus::Alive, chunkObj);
										});
									}, JobPriority::High, JobThread::Main);
								}, JobPriority::Normal, JobThread::Worker);
							}
						}
					}
				}
			}
		}
	}
}

void World::Destroy() {

}

void World::ObjectAdded(Object *obj) {
	auto camera = obj->Get<PlayerCameraComponent>();
	if(camera != nullptr) {
		cameraObj = obj;
	}
}

std::vector<bool> World::GenerateChunk(const Point &&p) const {
	std::vector<bool> blocks;
	blocks.resize(chunkSize.x * chunkSize.y * chunkSize.z);

	const Point blockSize = Point(1, 1, 1, 1);
	Point actual = p * Point(blockSize.x * chunkSize.x, blockSize.y * chunkSize.y, -blockSize.z * chunkSize.z, 1);

	int i = 0;
	for(float z = 0; z < blockSize.z * chunkSize.z; z += blockSize.z) {
		for(float x = 0; x < blockSize.x * chunkSize.x; x += blockSize.x) {
			for(float y = 0; y < blockSize.y * chunkSize.y; y += blockSize.y) {
				blocks.at(i++) = GenerateBlock(actual + Point(x, y, z, 0));
			}
		}
	}
	return blocks;
}

bool World::GenerateBlock(const Point &&p) const {
	auto fDensity = [] (const double x) {
		return glm::pow(1.0 - glm::abs(glm::sin(x)), 1.0);
	};

	const float scale = 16.0f;
	auto result = noise.eval(p.x / scale, p.y * 1.5f / scale, p.z / scale);
	return (
		result > (fDensity(p.x / scale / 32.0f) - 1) * 1.5f + 0.1f ||
		result > (fDensity(p.y / scale / 10.0f) - 1) * 1.5f + 0.1f ||
		result > (fDensity(p.z / scale / 32.0f) - 1) * 1.5f + 0.1f
		);
}
