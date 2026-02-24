.PHONY: all debug clean

CXX = g++
CXXFLAGS = -I${SYSTEMC_HOME}/include -I./src -DSC_DEFAULT_WRITER_POLICY=SC_MANY_WRITERS -MMD
LDFLAGS = -L${SYSTEMC_HOME}/lib-linux64
LDLIBS = -lstdc++ -lsystemc -lm

SRC_DIR = src
BUILD_DIR = build

SOURCES = $(shell find $(SRC_DIR) -name "*.cpp")
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)
TARGET = $(BUILD_DIR)/main.out

all: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	gdb -q -ex run ./$(TARGET)

$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)