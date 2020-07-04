.PHONY: all check clean
TARGET = radar01
GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS) $(TARGET) $(C_SOURCES)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

include common.mk

CFLAGS = -I./ -I./vender -I./ringbuffer
CFLAGS += -Os
CFLAGS += -std=gnu99 -Wall -W
CFLAGS += -DUNUSED="__attribute__((unused))"
CFLAGS += -DNDEBUG
LDFLAGS += -lpthread


##### C Source #####
C_SOURCES = \
        ./radar01_main.c \
        ./radar01_io.c \
		./radar01_utils.c \
		./radar01_tlv.c \
		./radar01_http.c \
		./ringbuffer/ringbuffer.c
####################
OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES))

deps += $(OBJECTS:%.o=%.o.d)

%.o: %.c
	$(VECHO) "  CC\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF $@.d $<

$(TARGET): $(OBJECTS)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) -o $@ $^ $(LDFLAGS)

check: all
	@scripts/test.sh

clean:
	$(VECHO) "  Cleaning...\n"
	$(Q)$(RM) $(TARGET) $(OBJECTS) $(deps)

-include $(deps)