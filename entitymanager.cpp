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
			LogError("Error! No free scene entity buckets!\n");
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

	LogError("Warning! Tried to free scene entities that the entity manager doesn't own!\n");
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
	// Not too sure what to do here.
	// We could just do nothing and this wouldn't become a problem until we start
	//	reusing massive numbers of free entities
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
		LogError("Error! Entity ID mismatch in GetEntity (%u != %u)\n", ent->id, id);
		return nullptr;
	}
	return ent;
}

Entity* FindEntity(const char* name) {
	if(!name) return nullptr;

	auto& buckets = entityManager.entityBuckets;
	auto sbucket = &entityManager.sceneEntityBuckets[0];
	u32 nameLength = strlen(name);

	for(u32 i = 0; i < buckets.size(); i++) {
		auto bucket = &buckets[i];
		if(!bucket->entities || !bucket->capacity) continue;
		for(u32 e = 0; e < bucket->used; e++) {
			auto ent = &bucket->entities[e];
			if(ent->nameLength != nameLength) continue;
			if(!std::strncmp(name, ent->name, ent->nameLength)) {
				return ent;
			}
		}
	}

	if(!sbucket->entities || !sbucket->capacity) return nullptr;
	for(u32 e = 0; e < sbucket->used; e++) {
		auto ent = &sbucket->entities[e];
		if(ent->nameLength != nameLength) continue;
		if(!std::strncmp(name, ent->name, ent->nameLength)) {
			return ent;
		}
	}

	return nullptr;
}

void UpdateAllEntities(f32 dt) {
	auto& buckets = entityManager.entityBuckets;
	auto sbucket = &entityManager.sceneEntityBuckets[0];

	// Update free entities
	for(u32 i = 0; i < buckets.size(); i++) {
		auto bucket = &buckets[i];
		if(!bucket->entities || !bucket->capacity) continue;
		for(u32 e = 0; e < bucket->used; e++) {
			UpdateEntity(&bucket->entities[e], dt);
		}
	}

	// Update scene entities
	if(!sbucket->entities || !sbucket->capacity) return;
	for(u32 e = 0; e < sbucket->used; e++) {
		UpdateEntity(&sbucket->entities[e], dt);
	}
}

EntityIterator GetEntityIterator() {
	return EntityIterator::begin();
}

EntityIterator EntityIterator::begin() { return EntityIterator{0, 0}; }
EntityIterator EntityIterator::end() { return EntityIterator{-1, -1}; }

EntityManager::Bucket* GetBucket(s32 bucketID){
	if(bucketID < 0) {
		return &entityManager.sceneEntityBuckets[0];
	}else{
		assert(bucketID < (s32)entityManager.entityBuckets.size());
		return &entityManager.entityBuckets[bucketID];
	}
}

EntityIterator& EntityIterator::operator++() {
	if(entityOffset == -1) return *this;

	auto bucket = GetBucket(bucketID);
	if(++entityOffset >= bucket->used) {
		if(bucketID < 0) {
			entityOffset = -1;
			return *this;
		}

		entityOffset = 0;
		if(++bucketID >= (s32)entityManager.entityBuckets.size())
			bucketID = -1;

		bucket = GetBucket(bucketID);
		if(bucket->used == 0) {
			entityOffset = -1;
			return *this;
		}
	}

	return *this;
}

EntityIterator EntityIterator::operator++(int) {
	auto cpy = *this;
	++*this;
	return cpy;
}

bool EntityIterator::operator!=(const EntityIterator& o) {
	return !(*this == o);
}
bool EntityIterator::operator==(const EntityIterator& o) {
	return (bucketID == o.bucketID) && (entityOffset == o.entityOffset);
}

Entity* EntityIterator::operator*() {
	assert(entityOffset > -1);

	auto bucket = GetBucket(bucketID);
	assert(entityOffset < bucket->used);

	return &bucket->entities[entityOffset];
}

Entity* EntityIterator::operator->() {
	return **this;
}
