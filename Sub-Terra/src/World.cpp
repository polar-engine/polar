#include "common.h"
#include "World.h"
#include "FileSystem.h"
#include "endian.h"

void World::Init() {
	uint32_t seed = std::random_device()();
	uint32_t seed2 = std::random_device()();
	noise = OpenSimplexNoise(seed);
	noise2 = OpenSimplexNoise(seed2);
}

void World::Update(DeltaTicks &) {
	auto self = std::shared_ptr<World>(this);
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
			for(auto itPickup = pickups.begin(); itPickup != pickups.end();) {
				auto &coordTuple = itPickup->first;
				auto &obj = itPickup->second;

				if(abs(static_cast<int>(std::get<0>(coordTuple)) - chunkCoord.x) > viewDistance ||
				   abs(static_cast<int>(std::get<1>(coordTuple)) - chunkCoord.y) > viewDistance ||
				   abs(static_cast<int>(std::get<2>(coordTuple)) - chunkCoord.z) > viewDistance) {
					INFOS(std::get<0>(coordTuple) << std::get<1>(coordTuple) << std::get<2>(coordTuple));
					jobM->Do([self, obj] () { self->engine->RemoveObject(obj); }, JobPriority::Low, JobThread::Main);
					pickups.erase(itPickup++);
					continue;
				}
				++itPickup;
			}
			chunks.With([self, &jobM, &chunkCoord] (ChunksType &chunks) {
				for(auto it = chunks.begin(); it != chunks.end();) {
					auto &coordTuple = it->first;
					auto &obj = std::get<1>(it->second);

					if(abs(static_cast<int>(std::get<0>(coordTuple)) - chunkCoord.x) > self->viewDistance ||
					   abs(static_cast<int>(std::get<1>(coordTuple)) - chunkCoord.y) > self->viewDistance ||
					   abs(static_cast<int>(std::get<2>(coordTuple)) - chunkCoord.z) > self->viewDistance) {
						auto &status = std::get<0>(it->second);
						switch(status) {
						case ChunkStatus::Generating:
							status = ChunkStatus::Dying;
							break;
						case ChunkStatus::Alive:
						case ChunkStatus::Dead:
							jobM->Do([self, obj] () { self->engine->RemoveObject(obj); }, JobPriority::Low, JobThread::Main);
							chunks.erase(it++);
							continue;
						}
					}
					++it;
				}
			});


			const float size = 1.0f;
			ModelComponent::TrianglesType triangles({
				std::make_tuple(Point3(    0,  size,     0), Point3( size,     0,     0), Point3(    0,     0, -size)),
				std::make_tuple(Point3(    0,  size,     0), Point3(    0,     0,  size), Point3( size,     0,     0)),
				std::make_tuple(Point3(    0,  size,     0), Point3(-size,     0,     0), Point3(    0,     0,  size)),
				std::make_tuple(Point3(    0,  size,     0), Point3(    0,     0, -size), Point3(-size,     0,     0)),
				std::make_tuple(Point3(    0, -size,     0), Point3(    0,     0, -size), Point3( size,     0,     0)),
				std::make_tuple(Point3(    0, -size,     0), Point3( size,     0,     0), Point3(    0,     0,  size)),
				std::make_tuple(Point3(    0, -size,     0), Point3(    0,     0,  size), Point3(-size,     0,     0)),
				std::make_tuple(Point3(    0, -size,     0), Point3(-size,     0,     0), Point3(    0,     0, -size))
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

							if(abs(std::get<0>(coordTuple)) % 4 == 0 &&
							   abs(std::get<1>(coordTuple)) % 4 == 0 &&
							   abs(std::get<2>(coordTuple)) % 4 == 0) {
								if(pickups.find(coordTuple) == pickups.end()) {
									IDType pickupID;
									dtors.emplace_back(engine->AddObject(&pickupID));
									engine->AddComponent<PositionComponent>(pickupID, PosForChunkCoord(coord));
									engine->AddComponent<ModelComponent>(pickupID, triangles);
									//engine->AddComponent<BoundingComponent>(pickupID, Point3(-size), Point3(size));
									pickups.emplace(coordTuple, pickupID);
								}
							}

							if(chunks.With<bool>([&coordTuple] (ChunksType &chunks) {
								return chunks.find(coordTuple) == chunks.end();
							})) {
								chunks.With([&coordTuple] (ChunksType &chunks) {
									chunks.emplace(coordTuple, std::make_tuple(ChunkStatus::Generating, 0));
								});
								jobM->Do([self, jobM, coord] () {
									if(!self->exists) { return; }

									/* create tuple again inside of copying by binding to lambda
									 * tuples are packed at compile-time anyway or something
									 */
									ChunkKeyType coordTuple = std::make_tuple(static_cast<uint64_t>(coord.x),
																			  static_cast<uint64_t>(coord.y),
																			  static_cast<uint64_t>(coord.z));

									auto dead = self->chunks.With<bool>([&coordTuple] (ChunksType &chunks) {
										if(chunks.find(coordTuple) != chunks.end()) {
											auto &status = std::get<0>(chunks.at(coordTuple));
											if(status == ChunkStatus::Dying) {
												status = ChunkStatus::Dead;
												return true;
											} else { return false; }
										} else { return false; }
									});
									if(dead) { return; }

									auto blocks = self->GenerateChunk(coord);
									self->CreateChunk(coord, blocks, true);
								}, JobPriority::Normal, JobThread::Worker);
							}
						}
					}
				}
			}
		}
	}
}
