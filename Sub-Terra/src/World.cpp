#include "common.h"
#include "World.h"
#include "PositionComponent.h"
#include "PlayerCameraComponent.h"

void World::Init() {
	
}

void World::Update(DeltaTicks &, std::vector<Object *> &) {
	chunks.With([] (ChunksType &chunks) {
		INFO(chunks.size());
	});
	auto camera = cameraObj->Get<PlayerCameraComponent>();
	if(camera != nullptr) {
		auto pos = cameraObj->Get<PositionComponent>();
		if(pos != nullptr) {
			const unsigned char distance = 3;
			auto chunkSizeF = glm::fvec3(chunkSize);
			auto keyBase = glm::ivec3(glm::floor(glm::fvec3(pos->position) / chunkSizeF));
			auto jobM = engine->systems.Get<JobManager>();

			/* clean up chunks outside distance */
			chunks.With([this, distance, &keyBase] (ChunksType &chunks) {
				for(auto it = chunks.begin(); it != chunks.end();) {
					auto &keyTuple = it->first;
					if(it->second != nullptr) {
						if(abs(std::get<0>(keyTuple) -keyBase.x) > distance ||
						   abs(std::get<1>(keyTuple) -keyBase.y) > distance ||
						   abs(std::get<2>(keyTuple) -keyBase.z) > distance) {
							engine->RemoveObject(it->second);
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
								chunks.With([&keyTuple] (ChunksType &chunks) { chunks.emplace(keyTuple, nullptr); });
								jobM->Do([this, jobM, key, keyTuple, chunkSizeF] () {
									auto data = Generate(keyTuple);
									auto chunkObj = new Chunk(chunkSize.x, chunkSize.y, chunkSize.z, data);
									jobM->Do([this, keyTuple, key, chunkSizeF, chunkObj] () {
										auto chunkPos = glm::fvec3(key) * chunkSizeF;
										chunkObj->Add<PositionComponent>(Point(chunkPos.x, chunkPos.y, chunkPos.z + chunkSize.z, 1));
										engine->AddObject(chunkObj);
										chunks.With([&keyTuple, chunkObj] (ChunksType &chunks) { chunks.at(keyTuple) = chunkObj; });
									}, JobPriority::High, JobThread::Main);
								}, JobPriority::Normal, JobThread::Any);
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
	const float start = -0.5, end = -0;
	static double factor = 1.0f;
	factor /= 1.0001f;
	auto r = factor * (start - end) + end;

	auto chunkSizeF = glm::fvec3(chunkSize);
	std::vector<bool> blocks;
	blocks.reserve(chunkSize.x * chunkSize.y * chunkSize.z);
	for(unsigned char z = 0; z < chunkSize.z; ++z) {
		for(unsigned char x = 0; x < chunkSize.x; ++x) {
			for(unsigned char y = 0; y < chunkSize.y; ++y) {
				auto current = z * chunkSize.x * chunkSize.y + x * chunkSize.y + y;
				float scale = 0.5f;
				float scaleX = scale, scaleY = scale * 2, scaleZ = scale;
				double random = noise.eval(
					( std::get<0>(keyTuple) + x / chunkSizeF.x) * scaleX,
					( std::get<1>(keyTuple) + y / chunkSizeF.y) * scaleY,
					(-std::get<2>(keyTuple) + z / chunkSizeF.z) * scaleZ
				);
				static double sine = 0;
				sine += 0.000001;
				//blocks.push_back(random > (0.35f + glm::sin(sine) * 0.0625) || random < r);
				blocks.push_back(random > 0.45f);
			}
		}
	}
	return blocks;
}
