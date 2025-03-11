CC = "gcc"

BUILD_DIR = "./build"
SOURCE_DIR = "src"

build:
	export CC=$(CC)
	cmake -DCMAKE_BUILD_TYPE=Release -B $(BUILD_DIR)
	cmake --build ./build

# do a prod build first then run the install scripts
install:
	export CC=$(CC)
	cmake -DCMAKE_BUILD_TYPE=Release -B $(BUILD_DIR)
	cmake --build ./build
	scripts/install-artifacts.sh $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean build build-prod install
