#include "common.h"
#include "World.h"
#include "FileSystem.h"
#include "endian.h"

Atomic<bool> World::exists;

void World::Init() {
	uint32_t seed = std::random_device()();
	noise = OpenSimplexNoise(seed);
}

void World::Update(DeltaTicks &) {
	auto pair = engine->objects.right.equal_range(&typeid(PlayerCameraComponent));
	for(auto it = pair.first; it != pair.second; ++it) {
		auto &object = it->get_left();

		auto pos = engine->GetComponent<PositionComponent>(object);
		if(pos != nullptr) {
			auto chunkCoord = ChunkCoordForPos(pos->position.Get());
			auto jobM = engine->GetSystem<JobManager>().lock();

			/* clean up chunks outside distance */
			auto &eng = engine;
			auto &viewDist = viewDistance;
			chunks.With([&eng, &viewDist, &jobM, &chunkCoord] (ChunksType &chunks) {
				for(auto it = chunks.begin(); it != chunks.end();) {
					auto &coordTuple = it->first;
					auto &obj = std::get<1>(it->second);

					if(abs(static_cast<int>(std::get<0>(coordTuple)) - chunkCoord.x) > viewDist ||
					   abs(static_cast<int>(std::get<1>(coordTuple)) - chunkCoord.y) > viewDist ||
					   abs(static_cast<int>(std::get<2>(coordTuple)) - chunkCoord.z) > viewDist) {
						auto &status = std::get<0>(it->second);
						switch(status) {
						case ChunkStatus::Generating:
							status = ChunkStatus::Dying;
							break;
						case ChunkStatus::Alive:
						case ChunkStatus::Dead:
							jobM->Do([&eng, obj] () { eng->RemoveObject(obj); }, JobPriority::Low, JobThread::Main);
							chunks.erase(it++);
							continue;
						case ChunkStatus::Dying:
							break;
						}
					}
					++it;
				}
			});

			for(int d = 0; d <= viewDistance; ++d) {
				for(int x = -d; x <= d; ++x) {
					for(int y = -d; y <= d; ++y) {
						for(int z = -d; z <= d; ++z) {
							/* only operate on outer ring so at least one of x y z must have absolute value of d */
							if(glm::abs(x) != d && glm::abs(y) != d && glm::abs(z) != d) { continue; }

							auto coord = chunkCoord + Point3(x, y, z);
							ChunkKeyType coordTuple = std::make_tuple(static_cast<uint64_t>(coord.x),
							                                          static_cast<uint64_t>(coord.y),
							                                          static_cast<uint64_t>(coord.z));

							if(chunks.With<bool>([&coordTuple] (ChunksType &chunks) {
								return chunks.find(coordTuple) == chunks.end();
							})) {
								chunks.With([&coordTuple] (ChunksType &chunks) {
									chunks.emplace(coordTuple, std::make_tuple(ChunkStatus::Generating, 0));
								});
								jobM->Do([this, jobM, coord] () {
									auto e = exists.With<bool>([] (bool &exists) { return exists; });
									if(e == false) { return; }

									/* create tuple again inside of copying by binding to lambda
									 * tuples are packed at compile-time anyway or something
									 */
									ChunkKeyType coordTuple = std::make_tuple(static_cast<uint64_t>(coord.x),
																			  static_cast<uint64_t>(coord.y),
																			  static_cast<uint64_t>(coord.z));

									auto dead = chunks.With<bool>([&coordTuple] (ChunksType &chunks) {
										if(chunks.find(coordTuple) != chunks.end()) {
											auto &status = std::get<0>(chunks.at(coordTuple));
											if(status == ChunkStatus::Dying) {
												status = ChunkStatus::Dead;
												return true;
											} else { return false; }
										} else { return false; }
									});
									if(dead) { return; }

									auto blocks = GenerateChunk(coord);
									CreateChunk(coord, blocks, true);
								}, JobPriority::Normal, JobThread::Worker);
							}
						}
					}
				}
			}
		}
	}
}

World::~World() {
	exists.With([this] (bool &exists) {
		exists = false;
	});
}
