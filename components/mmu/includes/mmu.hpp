#pragma once

#include <stdint.h>
#include <unordered_map>

#define PAGE_SIZE_BYTES 256
#define MAX_FRAMES 512

struct Page {
	uint32_t vAddr;
	uint32_t processId;

	int frameIdx;
	bool present;
};

struct Frame {
	uint8_t *data;
	// uint32_t phyAddr; // theres no need? huh?

	bool inUse; // used by a page?
	bool dirty;
	bool locked; // is it being written or read from
	uint32_t lastAccess;
};

struct SwapFrame {
	uint8_t *data;

	bool inUse;
};

inline uint32_t getPageIdx(uint32_t vaddr) { return vaddr / PAGE_SIZE_BYTES; }

class MMU {
  public:
	MMU();
	~MMU();

	template <typename T> T read(uint32_t vaddr, uint32_t processId);
	template <typename T> void write(uint32_t vaddr, T val, uint32_t processId);

  private:
	Frame frames[MAX_FRAMES];

	uint32_t swapFrameCount;
	SwapFrame* swapFrames;

	// max of 10 processes
	std::unordered_map<uint32_t, Page> pageTable[10]; // pageTable[PID][pageIdx]

	Page *resolvePage(uint32_t vaddr, uint32_t processId);
	Frame *loadPage(Page *page);
	Frame *allocFrame();
	Frame *evict();
};