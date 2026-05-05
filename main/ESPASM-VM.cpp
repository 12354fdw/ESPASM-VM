#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char* TAG = "ESPASM";

extern "C" void app_main(void) {

	// init non volatile storage
	nvs_flash_init_label:
	esp_err_t nvs_ret = nvs_flash_init();
	if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES ||
		nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		
		ESP_LOGE(TAG, "NVS init error!");
		ESP_ERROR_CHECK(nvs_flash_erase());
		goto nvs_flash_init_label;
	}
	ESP_LOGI(TAG, "initalized NVS!");

	ESP_LOGI(TAG, "Hello, world!");
}