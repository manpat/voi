#include "voi.h"

namespace {
	EntityManager entityManager;
}

using Bucket = EntityManager::Bucket;

bool InitEntityManager() {
	std::memset(entityManager.sceneEntityBuckets, 0, sizeof(Bucket)*2);
	entityManager.entityBuckets.push_back({
		new Entity[EntityManager::FreeEntityBucketSize],
		EntityManager::FreeEntityBucketSize, 0, 0
	});

	return true;
}

Entity* AllocateSceneEntities(u16 count) {
	auto seb = &entityManager.sceneEntityBuckets[0];
	if(seb->entities) {
		seb = &entityManager.sceneEntityBuckets[1];
		if(seb->entities) {
			puts("Error! No free scene entity buckets!");
			return nullptr;
		}
	}

	seb->entities = new Entity[count];
	seb->capacity = count;
	seb->used = count;
	return seb->entities;
}

void FreeSceneEntities(Entity* se) {
	if(!se) return;

	for(u32 i = 0; i < 2; i++) {
		auto seb = &entityManager.sceneEntityBuckets[i];
		if(se == seb->entities) {
			delete[] se;
			seb->entities = nullptr;
			seb->capacity = 0;
			return;
		}
	}

	puts("Warning! Tried to free scene entities that the entity manager doesn't own!");
}

Entity* AllocateEntity() {
	auto& buckets = entityManager.entityBuckets;

	Bucket* bucket = nullptr;
	for(u32 i = 0; i < buckets.size(); i++) {
		auto b = &buckets[i];
		if(b->entities && (b->used < b->capacity)) {
			bucket = b;
			break;
		}
	}

	// All buckets are full
	if(!bucket) {
		u8 bucketID = buckets.size();
		buckets.push_back(Bucket {
			new Entity[EntityManager::FreeEntityBucketSize], 
			EntityManager::FreeEntityBucketSize, 0, bucketID
		});

		bucket = &buckets.back();
	}

	u16 entId = EntityManager::FreeEntityIDOffset + 
		EntityManager::FreeEntityBucketSize*bucket->id +
		bucket->used;

	auto ent = &bucket->entities[bucket->used++];
	std::memset(ent, 0, sizeof(Entity));
	ent->id = entId;

	return ent;
}

void FreeEntity(Entity* e) {
	auto& buckets = entityManager.entityBuckets;

	if(!e) return;
	if(e->id < EntityManager::FreeEntityIDOffset) return;

	u16 id = e->id - EntityManager::FreeEntityIDOffset;
	u16 bucketID = id / EntityManager::FreeEntityBucketSize;
	u16 idx = id % EntityManager::FreeEntityBucketSize;

	if(bucketID >= buckets.size()) {
		// TODO: Error
		return;
	}

	auto bucket = &buckets[bucketID];
	auto bEnt = &bucket->entities[idx];

	assert(e == bEnt);
	// TODO: Free entity stuff
}

Entity* GetEntity(u16 id) {
	if(!id) return nullptr;

	// Scene entity
	if(id < EntityManager::FreeEntityIDOffset) {
		auto b = &entityManager.sceneEntityBuckets[0];
		if(id > b->used) {
			return nullptr;
		}

		return &b->entities[id-1];
	}

	// Free entity
	u16 id2 = id-EntityManager::FreeEntityIDOffset;
	u16 bucketID = id2 / EntityManager::FreeEntityBucketSize;
	u16 idx = id2 % EntityManager::FreeEntityBucketSize;

	auto& buckets = entityManager.entityBuckets;
	if(bucketID >= buckets.size()) {
		return nullptr;
	}

	auto bucket = &buckets[bucketID];
	auto ent = &bucket->entities[idx];
	if(ent->id != id) {
		printf("Error! Entity ID mismatch in GetEntity (%u != %u)\n", ent->id, id);
		return nullptr;
	}
	return ent;
}