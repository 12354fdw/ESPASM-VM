#include "esp_log.h"
#include "global.hpp"
#include "../components/init_systems/includes/init.hpp"

extern "C" void app_main(void) {
	init_systems();

	ESP_LOGI(TAG, "Hello, world!");
}