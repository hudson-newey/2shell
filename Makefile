BUILD_DIR = "build"
SOURCE_DIR = "src"

build:
	mkdir -p $(BUILD_DIR)
	gcc $(SOURCE_DIR)/main.c -o $(BUILD_DIR)/main

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean
