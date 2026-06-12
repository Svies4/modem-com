TARGET_EXEC := modem-com
BUILD_DIR   := ./build
BIN_DIR     := ./bin
SRC_DIRS    := ./src

LOCAL_INC   := -I./include

SDK_INC     :=

CFLAGS   += -fPIC -fPIE

CPPFLAGS += $(LOCAL_INC) $(SDK_INC)
CPPFLAGS += -I$(STAGING_DIR)/usr/include/cjson
LDFLAGS  += -lcjson

SRCS := $(shell find $(SRC_DIRS) -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

all: $(BIN_DIR)/$(TARGET_EXEC)

$(BIN_DIR)/$(TARGET_EXEC): $(OBJS)
	mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)