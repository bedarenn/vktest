##################################### NAME #####################################

NAME := exec
MAKE := make --no-print-directory -C

################################### COMPILER ###################################

CC := clang++

################################# DIRECTORIES ##################################

DIR_SRCS := srcs/
DIR_HDRS := hdrs/
DIR_OBJS := objs/
DIR_LIBS := libs/

#################################### FLAGS #####################################

CFLAGS := -Wall -Wextra -std=c++20 -Werror
IFLAGS := -I$(DIR_HDRS) -I$(DIR_WCL_)$(DIR_HDRS)
LFLAGS := -L$(DIR_LIBS) -lvulkan -lglfw

#################################### FILES #####################################

include sources.mk

################################### OBJECTS ####################################

OBJS = $(addprefix $(DIR_OBJS), $(SRCS:%.cpp=%.o))

#################################### RULES #####################################

debug:
all:

$(NAME): $(OBJS)
	$(CC) $^ $(LFLAGS) -o $@

$(DIR_OBJS)%.o: $(DIR_SRCS)%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

clean:
	rm -rf $(DIR_OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean debug

all:	CFLAGS += -DNDEBUG=1
all:	$(NAME)
debug:	CFLAGS += -g
debug:	$(NAME)

run: all launch
test: debug launch

launch:
	./$(NAME)

#################################### PHONY #####################################
.PHONY: all clean fclean re

#################################### COLORS ####################################
RED = \033[0;31m
GREEN = \033[0;32m
BROWN = \033[1;33m
PURPLE = \033[0;35m
NC = \033[0m
