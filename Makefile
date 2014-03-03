PROGRAM		=	redis_executor
CC		=	gcc
CFLAGS		=	-std=gnu99 -lpcre -lhiredis -lrt -Wall --pedantic
SOURCE_DIR  	=	./source
OUT_DIR     	=	./out
vpath 		%.c $(SOURCE_DIR)

DEBUG ?= 0

ifeq ($(DEBUG),1)
	CFLAGS := $(CFLAGS) -g3
else
	CFLAGS := $(CFLAGS) -O2
endif

all: $(addprefix $(OUT_DIR)/,$(PROGRAM))

$(OUT_DIR)/%.o: %.c | $(OUT_DIR)/
	$(CC) $(CFLAGS) -o $@ -c $^

$(OUT_DIR)/$(PROGRAM): $(addprefix $(OUT_DIR)/, main.o redis.o redis_command.o argument.o d_array.o output.o ) |  $(OUT_DIR)/
	$(CC) $(CFLAGS) -o $@ $^

$(OUT_DIR)/:
	mkdir -p $@

run: $(addprefix $(OUT_DIR)/,$(PROGRAM))
	$(addprefix $(OUT_DIR)/,$(PROGRAM)) -s "127.0.0.1:26379" "LPUSH lista888 \"Here is some text\";LPUSH lista888 \"Here is other text\""


.PHONY: clean
clean:
	@if test -d $(OUT_DIR); then\
		rm -r $(OUT_DIR); \
	fi
	@echo "cleaned up"
