#include <string.h>

#include <creed.h>

#define INIT_SIZ 256

void crHashMapInit(struct CrHashMap *hm) {
	hm->cells = calloc(sizeof(struct CrHashMapCell), INIT_SIZ);
	hm->size = INIT_SIZ;
}

static
void grow(struct CrHashMap *hm) {
	struct CrHashMapCell *a = calloc(hm->size * 2, sizeof(struct CrHashMapCell));
	const size_t newSize = hm->size * 2;

	for (int i=0; i < hm->size; ++i)
		if (hm->cells[i].p)
			a[hm->cells[i].hash % newSize] = hm->cells[i];

	hm->size = newSize;
	hm->cells = a;
}

void *crHashMapGet(struct CrHashMap *hm, uint32_t hash) {
	return hm->cells[hash % hm->size].p;
}

void *crHashMapGetStr(struct CrHashMap *hm, char *str) {
	return crHashMapGet(hm, crHash((CrSlice(char)){ .p = str, .s = strlen(str) }));
}

void crHashMapSet(struct CrHashMap *hm, uint32_t hash, void *ptr) {
	if (hm->cells[hash % hm->size].p && hm->cells[hash % hm->size].hash != hash) {
		grow(hm);
		crHashMapSet(hm, hash, ptr);
		return;
	}

	hm->cells[hash % hm->size].p = ptr;
	hm->cells[hash % hm->size].hash = hash;
}

void crHashMapSetStr(struct CrHashMap *hm, char *str, void *ptr) {
	crHashMapSet(hm, crHash((CrSlice(char)){ .p = str, .s = strlen(str) }), ptr);
}
