#include "includes/init.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *ITAG = "INIT";
static const char *PTAG = "POST";

static void initNvs() {
	esp_err_t ret = nvs_flash_init();

	if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
		ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_LOGW(ITAG, "BAD NVS, ERASING");
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}

	ESP_ERROR_CHECK(ret);
	ESP_LOGI(ITAG, "[  OK  ] NVS");
}

static void initNetif() {
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	ESP_LOGI(ITAG, "[  OK  ] netif");
}

static void initWifi() {
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_LOGI(ITAG, "[  OK  ] Wifi");
}

static void initLittleFS() {
	esp_vfs_littlefs_conf_t cfg = {};
	cfg.base_path = "/littlefs";
	cfg.partition_label = "storage";
	cfg.format_if_mount_failed = true;
	cfg.dont_mount = false;

	esp_err_t ret = esp_vfs_littlefs_register(&cfg);

	if (ret == ESP_OK) goto success;

	if (ret == ESP_FAIL) {
		ESP_LOGE(ITAG, "[ FAIL ] LittleFS");
		ESP_LOGE(ITAG, "Failed to mount / format LittleFS");
	} else if (ret == ESP_ERR_NOT_FOUND) {
		ESP_LOGE(ITAG, "[ FAIL ] LittleFS");
		ESP_LOGE(ITAG, "Failed find to LittleFS partition");
	} else {
		ESP_LOGE(ITAG, "[ FAIL ] LittleFS");
		ESP_LOGE(ITAG, "Error: %s", esp_err_to_name(ret));
	}

	esp_restart();


success:
	ESP_LOGI(ITAG, "[  OK  ] LittleFS");

	return;
}

static void postFlash() {
	size_t total, used;
	if (esp_littlefs_info("storage", &total, &used) != ESP_OK) {
		ESP_LOGE(PTAG, "[  FAIL  ] FLASH SELFTEST (Filesystem unreachable!)");
		esp_restart();
	}

	nvs_handle_t nvsHandle;
	nvs_open("storage", NVS_READWRITE, &nvsHandle);

	uint32_t lastTotal = 0;
	nvs_get_u32(nvsHandle, "littleFsTotal", &lastTotal);

	if (lastTotal != 0 && total < lastTotal) {
		ESP_LOGW(
			PTAG,
			"[ WARN ] Flash capacity lower than last time! (prev=%d, now=%d)",
			lastTotal, total);
		goto ret;
	}

	ESP_LOGI(PTAG, "[  OK  ] Flash (prev=%d, now=%d)", lastTotal, total);

ret:
	nvs_set_u32(nvsHandle, "littleFsTotal", total);
	nvs_commit(nvsHandle);
	nvs_close(nvsHandle);
}

void initSystems() {
	initNvs();
	initNetif();
	initWifi();
	initLittleFS();

	ESP_LOGI(ITAG, "ESP32 INITALIZED");

	ESP_LOGI(PTAG, "PERFORMING POST");
	postFlash();

	ESP_LOGI("MEMCHECK", "FREE HEAP: %lu bytes",
			 heap_caps_get_free_size(MALLOC_CAP_8BIT));
	ESP_LOGI("MEMCHECK", "LARGEST BLOCK: %lu bytes",
			 heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}