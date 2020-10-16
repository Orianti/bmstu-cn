EXEC = server.out

BUILD_DIR = build

CC = gcc
CFLAGS = -Wall -Wextra -MMD
LDFLAGS =

SRCS = server.c itoa/itoa.c

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)
-include $(DEPS)
       
$(BUILD_DIR)/$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
