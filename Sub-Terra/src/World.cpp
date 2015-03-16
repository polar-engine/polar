#include "common.h"
#include "World.h"
#include "JobManager.h"
#include "PlayerCameraComponent.h"
#include "PositionComponent.h"
#include "Chunk.h"

void World::Init() {
	
}

void World::Update(DeltaTicks &) {
	auto pair = engine->objects.right.equal_range(&typeid(PlayerCameraComponent));
	for(auto it = pair.first; it != pair.second; ++it) {
		auto object = it->get_left();
		auto camera = static_cast<PlayerCameraComponent *>(it->info.get());

		auto pos = engine->GetComponent<PositionComponent>(object);
		if(pos != nullptr) {
			const unsigned char distance = 5;

			auto chunkSizeF = Point3(chunkSize);
			auto keyBase = glm::ivec3(glm::floor(pos->position.Get() / chunkSizeF));
			auto jobM = engine->systems.Get<JobManager>();

			/* clean up chunks outside distance */
			chunks.With([this, jobM, distance, &keyBase] (ChunksType &chunks) {
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
							jobM->Do([this, obj] () { engine->RemoveObject(obj); }, JobPriority::Low, JobThread::Main);
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
							/* only operate on outer ring so at least one of x y z must have absolute value of d */
							if(glm::abs(x) != d && glm::abs(y) != d && glm::abs(z) != d) { continue; }

							auto key = keyBase + glm::ivec3(x, y, z);
							auto keyTuple = std::make_tuple(key.x, key.y, key.z);

							if(chunks.With<bool>([&keyTuple] (ChunksType &chunks) {
								return chunks.find(keyTuple) == chunks.end();
							})) {
								chunks.With([&keyTuple] (ChunksType &chunks) {
									chunks.emplace(keyTuple, std::make_tuple(ChunkStatus::Generating, 0));
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

									auto pos = new PositionComponent(Point3(key) * chunkSizeF);
									auto data = GenerateChunk(Point3(std::get<0>(keyTuple), std::get<1>(keyTuple), std::get<2>(keyTuple)));
									auto chunk = new Chunk(chunkSize.x, chunkSize.y, chunkSize.z, std::move(data));

									jobM->Do([this, keyTuple, chunk, pos] () {
										auto id = engine->AddObject();
										engine->InsertComponent<PositionComponent>(id, pos);
										engine->InsertComponent<ModelComponent>(id, chunk);
										chunks.With([&keyTuple, id] (ChunksType &chunks) {
											chunks.at(keyTuple) = std::make_tuple(ChunkStatus::Alive, id);
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

std::vector<bool> World::GenerateChunk(const Point3 &&p) const {
	std::vector<bool> blocks;
	blocks.resize(chunkSize.x * chunkSize.y * chunkSize.z);

	const Point3 blockSize(1, 1, 1);
	Point3 actual = p * Point3(blockSize.x * chunkSize.x, blockSize.y * chunkSize.y, blockSize.z * chunkSize.z);

	int i = 0;
	for(float z = 0; z < blockSize.z * chunkSize.z; z += blockSize.z) {
		for(float x = 0; x < blockSize.x * chunkSize.x; x += blockSize.x) {
			for(float y = 0; y < blockSize.y * chunkSize.y; y += blockSize.y) {
				blocks.at(i++) = GenerateBlock(actual + Point3(x, y, z));
			}
		}
	}
	return blocks;
}

bool World::GenerateBlock(const Point3 &&p) const {
	/* periodic 'normal distribution' function */
	auto fDensity = [] (const double x) {
		return 1.0 - glm::abs(glm::sin(x));
	};

	const float scale = 17.0f;
	auto result = noise.eval(p.x / scale, p.y * 1.6f / scale, p.z / scale);
	return (
		result > (fDensity(p.x / scale / 32.0f) - 1) * 1.5f + 0.1f ||
		result > (fDensity(p.y / scale / 10.0f) - 1) * 1.5f + 0.1f ||
		result > (fDensity(p.z / scale / 32.0f) - 1) * 1.5f + 0.1f
		);
}
