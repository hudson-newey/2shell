CC = "gcc"

BUILD_DIR = "build"
SOURCE_DIR = "src"

build:
	mkdir -p $(BUILD_DIR)
	$(CC) $(SOURCE_DIR)/main.c -o $(BUILD_DIR)/2sh

build-prod:
	mkdir -p $(BUILD_DIR)
	$(CC) $(SOURCE_DIR)/main.c -o $(BUILD_DIR)/2sh -O3

install:
	scripts/install-artifacts.sh $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean build build-prod install
