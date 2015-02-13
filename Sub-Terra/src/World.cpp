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
			const unsigned char distance = 5;

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

									auto data = Generate(keyTuple);
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

std::vector<bool> World::Generate(const ChunkKeyType &keyTuple) const {
	const float scale = 1.0f;
	const float scaleX = scale, scaleY = scale, scaleZ = scale;

	std::vector<bool> blocks;
	blocks.resize(chunkSize.x * chunkSize.y * chunkSize.z);

	float xIncr = 1.0f / chunkSize.x, yIncr = 1.0f / chunkSize.y, zIncr = 1.0f / chunkSize.z;
	int i = 0;
	for(float z = 0; z < 1; z += zIncr) {
		for(float x = 0; x < 1; x += xIncr) {
			for(float y = 0; y < 1; y += yIncr) {
				blocks.at(i++) = GenerateBlock(
					( std::get<0>(keyTuple) + x) * scaleX,
					( std::get<1>(keyTuple) + y) * scaleY,
					(-std::get<2>(keyTuple) + z) * scaleZ
				);
			}
		}
	}
	return blocks;
}

bool World::GenerateBlock(const float x, const float y, const float z) const {
	auto fDensity = [] (const double x) {
		return glm::pow(1.0 - glm::abs(glm::sin(x)), 1.0);
	};

	const float scale = 1.2f;
	auto result = noise.eval(x / scale, y * 1.4f / scale, z / scale);
	return (
		result > (fDensity(x / 32.0f) - 1) * 1.5f + 0.1f ||
		result > (fDensity(y / 10.0f) - 1) * 1.5f + 0.1f ||
		result > (fDensity(z / 32.0f) - 1) * 1.5f + 0.1f
		);
}
