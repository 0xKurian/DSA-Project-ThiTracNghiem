CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -g
BUILD_DIR ?= build

ifeq ($(OS),Windows_NT)
	# Cấu hình trên Windows (MinGW-w64 + Raylib Windows)
	CPPFLAGS ?= -I"third_party/raylib-6.2_win64_mingw-w64/include" -Icore -Ibusiness -Iui
	LDFLAGS ?= -L"third_party/raylib-6.2_win64_mingw-w64/lib"
	LDLIBS ?= -lraylibdll -lopengl32 -lgdi32 -lwinmm
	BIN_EXT := .exe
	MKDIR = @if not exist "$(subst /,\,$(@D))" mkdir "$(subst /,\,$(@D))"
	RM_CMD = @if exist "$(subst /,\,$(BUILD_DIR))" rmdir /s /q "$(subst /,\,$(BUILD_DIR))"
	RUN_TESTS = @for %%B in ($(subst /,\,$(TEST_BINS))) do @(echo Running %%B && %%B || exit /b 1)
else
	# Cấu hình trên Linux
	CPPFLAGS ?= -Ithird_party/raylib-6.2_linux_amd64/include -Icore -Ibusiness -Iui
	LDFLAGS ?= -Lthird_party
	LDLIBS ?= third_party/raylib-6.2_linux_amd64/lib/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11
	BIN_EXT :=
	MKDIR = @mkdir -p $(@D)
	RM_CMD = rm -rf -- $(BUILD_DIR)
	RUN_TESTS = @set -e; for test_bin in $(TEST_BINS); do echo "Running $$test_bin"; "$$test_bin"; done
endif

CORE_SRC := $(wildcard core/*.cpp)
CORE_OBJ := $(patsubst core/%.cpp,$(BUILD_DIR)/core/%.o,$(CORE_SRC))

BUSINESS_SRC := $(wildcard business/*.cpp)
BUSINESS_OBJ := $(patsubst business/%.cpp,$(BUILD_DIR)/business/%.o,$(BUSINESS_SRC))

UI_SRC := $(wildcard ui/*.cpp)
UI_OBJ := $(patsubst ui/%.cpp,$(BUILD_DIR)/ui/%.o,$(UI_SRC))

ALL_OBJS := $(CORE_OBJ) $(BUSINESS_OBJ) $(UI_OBJ)

TEST_NAMES := $(basename $(notdir $(wildcard tests/*.cpp)))
TEST_BINS := $(addprefix $(BUILD_DIR)/tests/,$(addsuffix $(BIN_EXT),$(TEST_NAMES)))

APP_BIN := $(BUILD_DIR)/app$(BIN_EXT)
DEMO_BIN := $(BUILD_DIR)/core_demo$(BIN_EXT)

DEPS := $(ALL_OBJS:.o=.d) $(TEST_BINS:=.d) $(BUILD_DIR)/core_demo.d $(BUILD_DIR)/app.d

.PHONY: all app demo test run run-gui run-cli clean
all: $(APP_BIN) $(DEMO_BIN) $(TEST_BINS)
app: $(APP_BIN)
demo: $(DEMO_BIN)

$(BUILD_DIR)/core/%.o: core/%.cpp
	$(MKDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/business/%.o: business/%.cpp
	$(MKDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/ui/%.o: ui/%.cpp
	$(MKDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(APP_BIN): main.cpp $(ALL_OBJS)
	$(MKDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $@.d $< $(ALL_OBJS) $(LDFLAGS) $(LDLIBS) -o $@
ifeq ($(OS),Windows_NT)
	@if not exist "$(subst /,\,$(BUILD_DIR))\raylib.dll" copy /y "third_party\raylib-6.2_win64_mingw-w64\lib\raylib.dll" "$(subst /,\,$(BUILD_DIR))\" >nul
endif

$(DEMO_BIN): examples/core_demo.cpp $(CORE_OBJ)
	$(MKDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $@.d $< $(CORE_OBJ) -o $@

$(BUILD_DIR)/tests/%$(BIN_EXT): tests/%.cpp $(CORE_OBJ) $(BUSINESS_OBJ)
	$(MKDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $@.d $< $(CORE_OBJ) $(BUSINESS_OBJ) -o $@

run: $(APP_BIN)
	$(APP_BIN)

run-gui: $(APP_BIN)
	$(APP_BIN)

run-cli: $(APP_BIN)
	$(APP_BIN) --console

test: $(TEST_BINS)
	$(RUN_TESTS)
	$(BUILD_DIR)/tests/test_core_contracts$(BIN_EXT) null

clean:
	$(RM_CMD)

-include $(DEPS)
