idf_component_register(
	SRCS "init.cpp"
	INCLUDE_DIRS "includes"
	REQUIRES nvs_flash esp_wifi littlefs
)