.PHONY: all clean install debug

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
LDFLAGS ?=

BUILD_DIR ?= build
TARGET = $(BUILD_DIR)/searchup
SRC = src/searchup.cpp

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

debug: CXXFLAGS = -std=c++17 -Wall -Wextra -g -O0 -fsanitize=address
debug: $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/searchup

clean:
	rm -rf $(BUILD_DIR)