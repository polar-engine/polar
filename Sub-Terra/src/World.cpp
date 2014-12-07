#include "common.h"
#include "World.h"
#include "PositionComponent.h"
#include "PlayerCameraComponent.h"

void World::Init() {
	
}

void World::Update(DeltaTicks &, std::vector<Object *> &) {
	auto camera = cameraObj->Get<PlayerCameraComponent>();
	if(camera != nullptr) {
		auto pos = cameraObj->Get<PositionComponent>();
		if(pos != nullptr) {
			const unsigned char distance = 3;
			auto chunkSizeF = glm::fvec3(chunkSize);
			auto keyBase = glm::ivec3(glm::floor(glm::fvec3(pos->position) / chunkSizeF));
			auto jobM = engine->systems.Get<JobManager>();

			/* clean up chunks outside distance */
			for(auto it = chunks.begin(); it != chunks.end();) {
				auto &keyTuple = it->first;
				if(it->second != nullptr &&
				   abs(std::get<0>(keyTuple) - keyBase.x) > distance ||
				   abs(std::get<1>(keyTuple) - keyBase.y) > distance ||
				   abs(std::get<2>(keyTuple) - keyBase.z) > distance) {
					engine->RemoveObject(it->second);
					chunks.erase(it++);
				} else { ++it; }
			}

			/* dispatch chunk generation jobs in a cube around the player of size (distance * 2 + 1) */
			for(int d = 0; d <= distance; ++d) {
				for(int x = -d; x <= d; ++x) {
					for(int y = -d; y <= d; ++y) {
						for(int z = -d; z <= d; ++z) {
							JobPriority priority = JobPriority::Low;
							switch(d) {
							case 0:
							case 1:
								priority = JobPriority::High;
								break;
							case 2:
								priority = JobPriority::Normal;
								break;
							}
							auto key = keyBase + glm::ivec3(x, y, z);
							auto keyTuple = std::make_tuple(key.x, key.y, key.z);
							auto chunk = chunks.find(keyTuple);
							if(chunk == chunks.end()) {
								chunks.emplace(keyTuple, nullptr);
								jobM->Do([this, jobM, key, keyTuple, chunkSizeF, x, y, z] () {
									auto chunkObj = new Chunk(chunkSize.x, chunkSize.y, chunkSize.z, Generate(keyTuple));
									auto chunkPos = glm::fvec3(key) * chunkSizeF;
									chunkObj->Add<PositionComponent>(Point(chunkPos.x, chunkPos.y, chunkPos.z + chunkSize.z, 1));
									jobM->Do([this, keyTuple, chunkObj] () {
										auto chunk = chunks.find(keyTuple);
										if(chunk != chunks.end()) {
											engine->AddObject(chunkObj);
											chunks.at(keyTuple) = chunkObj;
										}
									}, JobPriority::High, JobThread::Main);
								}, priority, JobThread::Main);
								/* TODO: make thread-safe */
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

std::vector<bool> World::Generate(const ChunkKey &keyTuple) const {
	auto chunkSizeF = glm::fvec3(chunkSize);
	std::vector<bool> blocks;
	blocks.resize(chunkSize.x * chunkSize.y * chunkSize.z);
	for(unsigned char x = 0; x < chunkSize.x; ++x) {
		for(unsigned char z = 0; z < chunkSize.z; ++z) {
			for(unsigned char y = 0; y < chunkSize.y; ++y) {
				auto current = z * chunkSize.x * chunkSize.y + x * chunkSize.y + y;
				float scale = 0.5f;
				float scaleX = scale, scaleY = scale * 2, scaleZ = scale;
				double random = noise.eval(
					( std::get<0>(keyTuple) + x / chunkSizeF.x) * scaleX,
					( std::get<1>(keyTuple) + y / chunkSizeF.y) * scaleY,
					(-std::get<2>(keyTuple) + z / chunkSizeF.z) * scaleZ
				);
				blocks.at(current) = random > -0.25;
			}
		}
	}
	return blocks;
}
