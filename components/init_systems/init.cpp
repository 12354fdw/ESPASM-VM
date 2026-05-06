#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "includes/init.hpp"
#include "nvs_flash.h"

static const char *TAG = "INIT";

static void initNvs() {
	esp_err_t ret = nvs_flash_init();

	if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
		ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_LOGW(TAG, "BAD NVS, ERASING");
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}

	ESP_ERROR_CHECK(ret);
	ESP_LOGI(TAG, "[  OK  ] NVS");
}

static void initNetif() {
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	ESP_LOGI(TAG, "[  OK  ] netif");
}

void initSystems() {
	initNvs();
	initNetif();

	ESP_LOGI(TAG, "ESP32 INITALIZED");

	ESP_LOGI("MEMCHECK", "FREE HEAP: %lu bytes",
			 heap_caps_get_free_size(MALLOC_CAP_8BIT));
	ESP_LOGI("MEMCHECK", "LARGEST BLOCK: %lu bytes",
			 heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}