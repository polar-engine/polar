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
			auto chunkSizeF = glm::fvec3(chunkSize);
			auto keyBase = glm::ivec3(glm::floor(glm::fvec3(pos->position) / chunkSizeF));
			const unsigned char distance = 2;
			for(int x = -distance; x <= distance; ++x) {
				for(int y = -distance; y <= distance; ++y) {
					for(int z = -distance; z <= distance; ++z) {
						auto key = keyBase + glm::ivec3(x, y, z);
						auto keyTuple = std::make_tuple(key.x, key.y, key.z);
						auto chunk = chunks.find(keyTuple);
						if(chunk == chunks.end()) {
							auto chunkObj = new Chunk(chunkSize.x, chunkSize.y, chunkSize.z, Generate(keyTuple));
							auto chunkPos = glm::fvec3(key) * chunkSizeF;
							chunkObj->Add<PositionComponent>(Point(chunkPos.x, chunkPos.y, chunkPos.z + chunkSize.z, 1));
							chunks.emplace(keyTuple, chunkObj);
							engine->AddObject(chunkObj);
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

std::vector<bool> World::Generate(const ChunkKey &keyTuple) {
	auto chunkSizeF = glm::fvec3(chunkSize);
	std::vector<bool> blocks;
	blocks.resize(chunkSize.x * chunkSize.y * chunkSize.z);
	for(unsigned char x = 0; x < chunkSize.x; ++x) {
		for(unsigned char z = 0; z < chunkSize.z; ++z) {
			for(unsigned char y = 0; y < chunkSize.y; ++y) {
				auto current = z * chunkSize.x * chunkSize.y + x * chunkSize.y + y;
				float scaleX = 0.5f, scaleY = 0.9f, scaleZ = 0.5f;;
				float random = noise.eval(
					( std::get<0>(keyTuple) + x / chunkSizeF.x) * scaleX,
					( std::get<1>(keyTuple) + y / chunkSizeF.y) * scaleY,
					(-std::get<2>(keyTuple) + z / chunkSizeF.z) * scaleZ
					);
				blocks.at(current) = random > -0.4;
			}
		}
	}
	return blocks;
}
