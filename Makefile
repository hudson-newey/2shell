BUILD_DIR = "build"
SOURCE_DIR = "src"

build:
	mkdir -p $(BUILD_DIR)
	gcc $(SOURCE_DIR)/main.c -o $(BUILD_DIR)/main

build-prod:
	mkdir -p $(BUILD_DIR)
	gcc $(SOURCE_DIR)/main.c -o $(BUILD_DIR)/main -O3

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean build build-prod
