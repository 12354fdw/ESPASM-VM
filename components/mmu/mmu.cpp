#include "mmu.hpp"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"
#include <cstdint>
#include <cstring>
#include <unordered_map>

static const char *TAG = "MMU";

MMU::MMU() {
	// neat memory saving trick
	uint32_t i = 0;

	for (; i < MAX_FRAMES; i++) {
		void *addr = heap_caps_calloc(1, PAGE_SIZE_BYTES, MALLOC_CAP_8BIT);

		if (!addr)
			continue;

		frames[i].data = static_cast<uint8_t *>(addr);
		frames[i].inUse = false;
		frames[i].dirty = false;
		frames[i].locked = false;
		frames[i].lastAccess = 0;
	}

	ESP_LOGI(TAG, "allocated %d/%d frames (%d bytes)", i, MAX_FRAMES,
			 i * PAGE_SIZE_BYTES);

	
	// SWAP allocation
	uint32_t swapSize = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
	swapFrameCount = swapSize / PAGE_SIZE_BYTES;

	ESP_LOGI(TAG, "now allocating swap on psram, size=%lu", swapSize);

	// saving memory on SRAM
	swapFrames = (SwapFrame *)heap_caps_calloc(1, swapFrameCount * sizeof(SwapFrame), MALLOC_CAP_SPIRAM);

	i = 0;
	for (; i < swapFrameCount; i++) {
		void *addr = heap_caps_calloc(1, PAGE_SIZE_BYTES, MALLOC_CAP_SPIRAM);
		swapFrames[i].data = (uint8_t *)addr;
		swapFrames[i].inUse = false;
	}

	ESP_LOGI(TAG, "allocated %d frames on PSRAM (%d bytes)", i, i * PAGE_SIZE_BYTES);
}

MMU::~MMU() {
	for (int i = 0; i < MAX_FRAMES; i++) {
		// should not be nullptr
		if (frames[i].data) {
			free(frames[i].data);
			frames[i].data = nullptr;
		}
	}
}

template <typename T> T MMU::read(uint32_t vaddr, uint32_t processId) {
	Page *page = resolvePage(vaddr, processId);
	uint32_t offset = vaddr % PAGE_SIZE_BYTES;

	frames[page->frameIdx].locked = true;

	T result;
	memcpy(&result, frames[page->frameIdx].data + offset, sizeof(T));
	frames[page->frameIdx].locked = false;

	return result;
}

template <typename T>
void MMU::write(uint32_t vaddr, T val, uint32_t processId) {
	Page *page = resolvePage(vaddr, processId);
	uint32_t offset = vaddr % PAGE_SIZE_BYTES;

	frames[page->frameIdx].locked = true;

	// pray that we didn't write over PAGE_SIZE_BYTES
	memcpy(frames[page->frameIdx].data + offset, &val, sizeof(T));
	frames[page->frameIdx].dirty = true;

	frames[page->frameIdx].locked = false;
}

Page *MMU::resolvePage(uint32_t vaddr, uint32_t processId) {
	// make a reference or memory hell
	std::unordered_map<uint32_t, Page> &pages = pageTable[processId];

	auto it = pages.find(getPageIdx(vaddr));

	// it exists!
	if (it != pages.end()) {
		loadPage(&it->second);
		return &it->second;
	}

	// page doesn't exists! creating one
	// actually lets panic
	esp_restart();
}

Frame *MMU::loadPage(Page *page) {

	// already in memory
	if (page->present) {
		return &frames[page->frameIdx];
	}

	// probably swapped
	esp_restart();
	// TODO: load from PSRAM
}
