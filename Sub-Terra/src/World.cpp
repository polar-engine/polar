#include "common.h"
#include "World.h"
#include "FileSystem.h"
#include "endian.h"

void World::Update(DeltaTicks &) {
	auto jobM = engine->GetSystem<JobManager>().lock();
	auto &eng = engine;

	for(auto cameraID : cameras) {
		auto objectPos = engine->GetComponent<PositionComponent>(cameraID);
		auto chunkCoord = objectPos != nullptr ? ChunkCoordForPos(objectPos->position.Get()) : Point3(0);

		for(auto it = pickups.begin(); it != pickups.end();) {
			auto &coordTuple = it->first;
			auto &obj = it->second;

			if(abs(static_cast<int>(std::get<0>(coordTuple)) - chunkCoord.x) > viewDistance ||
			   abs(static_cast<int>(std::get<1>(coordTuple)) - chunkCoord.y) > viewDistance ||
			   abs(static_cast<int>(std::get<2>(coordTuple)) - chunkCoord.z) > viewDistance) {
				pickups.erase(it++);
			} else { ++it; }
		}

		for(auto it = chunks.begin(); it != chunks.end();) {
			auto &coordTuple = it->first;
			if(abs(static_cast<int>(std::get<0>(coordTuple)) - chunkCoord.x) > viewDistance ||
			   abs(static_cast<int>(std::get<1>(coordTuple)) - chunkCoord.y) > viewDistance ||
			   abs(static_cast<int>(std::get<2>(coordTuple)) - chunkCoord.z) > viewDistance) {
				chunks.erase(it++);
			} else { ++it; }
		}

		const float size = 1.0f;
		const ModelComponent::TrianglesType triangles({
			std::make_tuple(Point3(0,  size, 0), Point3(size,  0,     0), Point3(0,     0, -size)),
			std::make_tuple(Point3(0,  size, 0), Point3(0,     0,  size), Point3(size,  0,     0)),
			std::make_tuple(Point3(0,  size, 0), Point3(-size, 0,     0), Point3(0,     0,  size)),
			std::make_tuple(Point3(0,  size, 0), Point3(0,     0, -size), Point3(-size, 0,     0)),
			std::make_tuple(Point3(0, -size, 0), Point3(0,     0, -size), Point3(size,  0,     0)),
			std::make_tuple(Point3(0, -size, 0), Point3(size,  0,     0), Point3(0,     0,  size)),
			std::make_tuple(Point3(0, -size, 0), Point3(0,     0,  size), Point3(-size, 0,     0)),
			std::make_tuple(Point3(0, -size, 0), Point3(-size, 0,     0), Point3(0,     0, -size))
		});

		for(int d = 0; d <= viewDistance; ++d) {
			for(int x = -d; x <= d; ++x) {
				for(int y = -d; y <= d; ++y) {
					for(int z = -d; z <= d; ++z) {
						/* only operate on outer ring so at least one of x y z must have absolute value of d */
						if(glm::abs(x) != d && glm::abs(y) != d && glm::abs(z) != d) { continue; }

						auto coord = chunkCoord + Point3(x, y, z);
						KeyType coordTuple = ChunkKeyForChunkCoord(coord);

						if(abs(std::get<0>(coordTuple)) % 2 == 0 &&
						   abs(std::get<1>(coordTuple)) % 2 == 0 &&
						   abs(std::get<2>(coordTuple)) % 2 == 0) {
							if(pickups.find(coordTuple) == pickups.end()) {
								IDType pickupID;
								auto dtor = engine->AddObject(&pickupID);
								engine->AddComponent<PositionComponent>(pickupID, PosForChunkCoord(coord));
								engine->AddComponent<ModelComponent>(pickupID, triangles);
								engine->AddComponent<BoundingComponent>(pickupID, Point3(-size), Point3(size));
								pickups[coordTuple] = std::make_tuple(dtor, pickupID);
							}
						}

						if(chunks.find(coordTuple) == chunks.end()) {
							CreateChunk(coord, GenerateChunk(coord));
						}
					}
				}
			}
		}
	}
}
