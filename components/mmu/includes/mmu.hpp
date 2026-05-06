#pragma once

#include <stdint.h>

// 32 kilobytes worth of usable memory
#define PAGE_SIZE_BYTES 		256
#define MAXIUM_PAGES_IN_MEMORY 	128

struct Page {
	uint8_t data[PAGE_SIZE_BYTES];
	uint32_t vaddr;
	bool inMemory;
	bool dirty;
	bool locked;
	uint32_t lastAccess;
};