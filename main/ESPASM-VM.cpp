#include "esp_event.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "global.hpp"

#include "../components/init_systems/includes/init.hpp"
#include "../components/mmu//includes//mmu.hpp"

extern "C" void app_main(void) {
	initSystems();

	ESP_LOGI(TAG, "Hello, world!");

	MMU *mmu = new MMU();

	ESP_LOGI("MEMCHECK", "FREE HEAP: %lu bytes",
			 heap_caps_get_free_size(MALLOC_CAP_8BIT));
	ESP_LOGI("MEMCHECK", "LARGEST BLOCK: %lu bytes",
			 heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

	ESP_LOGI("MEMCHECK", "PSRAM: %lu bytes",
			 heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
}