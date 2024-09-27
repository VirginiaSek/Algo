# good guide: https://www.lucavall.in/blog/crafting-clean-maintainable-understandable-makefile-for-c-project
TARGET_EXEC ?= benchmarkSC

BUILD_DIR ?= build
SRC_DIRS ?= src

# Trova tutti i file sorgente .cpp, .c, .s usando il wildcard di make
SRCS := $(wildcard $(SRC_DIRS)/**/*.cpp $(SRC_DIRS)/**/*.c $(SRC_DIRS)/**/*.s $(SRC_DIRS)/*.c)

# Crea i file oggetto relativi
OBJS := $(SRCS:$(SRC_DIRS)/%.cpp=$(BUILD_DIR)/%.cpp.o)
OBJS := $(OBJS:$(SRC_DIRS)/%.c=$(BUILD_DIR)/%.c.o)
OBJS := $(OBJS:$(SRC_DIRS)/%.s=$(BUILD_DIR)/%.s.o)

# Aggiungi manualmente le directory di include (esempio: src/bangle_simple, src/dummy, ecc.)
INC_DIRS := src/bangle_simple src/dummy src/espruino src/oxford src/panTompkins
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
LDFLAGS ?= -static -mconsole  # Rimuovi -municode e aggiungi -mconsole

# Regola principale
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Regole per i file oggetto .s
$(BUILD_DIR)/%.s.o: $(SRC_DIRS)/%.s
	@powershell -Command "if (!(Test-Path '$(dir $@)')) { New-Item -ItemType Directory -Path '$(dir $@)' }"
	$(AS) $(ASFLAGS) -c $< -o $@

# Regole per i file oggetto .c
$(BUILD_DIR)/%.c.o: $(SRC_DIRS)/%.c
	@powershell -Command "if (!(Test-Path '$(dir $@)')) { New-Item -ItemType Directory -Path '$(dir $@)' }"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Regole per i file oggetto .cpp
$(BUILD_DIR)/%.cpp.o: $(SRC_DIRS)/%.cpp
	@powershell -Command "if (!(Test-Path '$(dir $@)')) { New-Item -ItemType Directory -Path '$(dir $@)' }"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
