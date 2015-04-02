#include "common.h"
#include "World.h"
#include "JobManager.h"
#include "PlayerCameraComponent.h"
#include "PositionComponent.h"
#include "BoundingComponent.h"
#include "Chunk.h"

void World::Init() {
	
}

void World::Update(DeltaTicks &) {
	auto pair = engine->objects.right.equal_range(&typeid(PlayerCameraComponent));
	for(auto it = pair.first; it != pair.second; ++it) {
		auto object = it->get_left();

		auto pos = engine->GetComponent<PositionComponent>(object);
		if(pos != nullptr) {
			auto chunkCoord = ChunkCoordForPos(pos->position.Get());
			auto jobM = engine->systems.Get<JobManager>().lock();

			/* clean up chunks outside distance */
			chunks.With([this, jobM, &chunkCoord] (ChunksType &chunks) {
				for(auto it = chunks.begin(); it != chunks.end();) {
					auto &keyTuple = it->first;
					auto obj = std::get<1>(it->second);

					if(abs(static_cast<int>(std::get<0>(keyTuple)) - chunkCoord.x) > viewDistance ||
					   abs(static_cast<int>(std::get<1>(keyTuple)) - chunkCoord.y) > viewDistance ||
					   abs(static_cast<int>(std::get<2>(keyTuple)) - chunkCoord.z) > viewDistance) {
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
									/* create tuple again inside of copying by binding to lambda
									 * tuples are packed at compile-time anyway or something
									 */
									ChunkKeyType coordTuple = std::make_tuple(static_cast<uint64_t>(coord.x),
																			  static_cast<uint64_t>(coord.y),
																			  static_cast<uint64_t>(coord.z));

									auto dead = chunks.With<bool>([&coordTuple] (ChunksType &chunks) {
										auto &status = std::get<0>(chunks.at(coordTuple));
										if(status == ChunkStatus::Dying) {
											status = ChunkStatus::Dead;
											return true;
										} else { return false; }
									});
									if(dead) { return; }

									auto chunkSizeF = Point3(chunkSize);
									auto data = GenerateChunk(coord);
									auto pos = new PositionComponent(coord * chunkSizeF);
									auto chunk = new Chunk(chunkSize.x, chunkSize.y, chunkSize.z, data);
									auto bounds = new BoundingComponent(Point3(0.0f), blockSize * chunkSizeF);

									/* add all block bounding boxes in chunk as children */
									for(unsigned char x = 0; x < chunkSize.x; ++x) {
										for(unsigned char y = 0; y < chunkSize.y; ++y) {
											for(unsigned char z = 0; z < chunkSize.z; ++z) {
												if(data.at(z * chunkSize.x * chunkSize.y + x * chunkSize.y + y) == true) {
													bounds->box.children.emplace_back(Point3(x * blockSize.x, y * blockSize.y, z * blockSize.z), blockSize);
												}
											}
										}
									}

									jobM->Do([this, coordTuple, pos, chunk, bounds] () {
										auto id = engine->AddObject();
										engine->InsertComponent<PositionComponent>(id, pos);
										engine->InsertComponent<ModelComponent>(id, chunk);
										engine->InsertComponent<BoundingComponent>(id, bounds);
										chunks.With([&coordTuple, id] (ChunksType &chunks) {
											chunks.at(coordTuple) = std::make_tuple(ChunkStatus::Alive, id);
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

World::~World() {
	chunks.With([this] (ChunksType &chunks) {
		for(auto it = chunks.begin(); it != chunks.end(); ++it) {
			engine->RemoveObject(std::get<1>(it->second));
		}
	});
}
