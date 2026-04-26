# This is the brain of the whole operation.
# Let's get this ship going!

# ──────────────────────────────────────────────────────────────────────────────
# COLOURS
# ──────────────────────────────────────────────────────────────────────────────

RED          = \033[0;31m
GREEN        = \033[0;32m
BLUE         = \033[0;34m
MAGENTA      = \033[0;35m
CYAN         = \033[0;36m

BOLD_RED     = \033[1;31m
BOLD_GREEN   = \033[1;32m
BOLD_BLUE    = \033[1;34m
BOLD_MAGENTA = \033[1;35m
BOLD_CYAN    = \033[1;36m

RESET        = \033[0m

# ──────────────────────────────────────────────────────────────────────────────
# COMPILER SETTINGS
# ──────────────────────────────────────────────────────────────────────────────

CC			= c++

CFLAGS      = -Wall -Wextra -Werror -g -std=c++98

CFLAGS_MAIN = $(CFLAGS) -I $(INCLUDES_DIR)

RM			= rm -rf

# ──────────────────────────────────────────────────────────────────────────────
# DIRECTORIES
# ──────────────────────────────────────────────────────────────────────────────

SERVER_DIR		= src

COMMAND_DIR		= src/cmds

UTILS_DIR		= utils

INCLUDES_DIR	= includes

OBJ_DIR			= obj

# ──────────────────────────────────────────────────────────────────────────────
# FILE LISTS
# ──────────────────────────────────────────────────────────────────────────────

NAME			= ircserv

INCLUDES_SRCS	= Server.hpp Client.hpp Channel.hpp utils.hpp

SERVER_SRCS		= Server.cpp Client.cpp Parsing.cpp ProcessCmd.cpp Channel.cpp

COMMAND_SRCS	= Cap.cpp Invite.cpp Join.cpp Kick.cpp Mode.cpp Nick.cpp Part.cpp \
				  Pass.cpp Ping.cpp Privmsg.cpp Quit.cpp Topic.cpp User.cpp

UTILS_SRCS		= utils.cpp

HEADER_FILE		= $(addprefix $(INCLUDES_DIR)/, $(INCLUDES_SRCS))

SRCS			= main.cpp \
				  $(addprefix $(SERVER_DIR)/, $(SERVER_SRCS)) \
				  $(addprefix $(COMMAND_DIR)/, $(COMMAND_SRCS)) \
				  $(addprefix $(UTILS_DIR)/, $(UTILS_SRCS))

OBJECTS			= $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

# ──────────────────────────────────────────────────────────────────────────────
# BONUS
# ──────────────────────────────────────────────────────────────────────────────

CFLAGS_BONUS			= $(CFLAGS) -I $(BONUS_DIR)/$(INCLUDES_DIR)

BONUS_DIR				= bonus

NAME_BONUS				= ircserv_bonus

INCLUDES_SRCS_BONUS		= server_bonus.hpp

SERVER_SRCS_BONUS		= server_bonus.cpp

CLIENT_SRCS_BONUS		= client_bonus.cpp

UTILS_SRCS_BONUS		= 

HEADER_FILE_BONUS		= $(addprefix $(BONUS_DIR)/$(INCLUDES_DIR)/, $(INCLUDES_SRCS_BONUS))

SRCS_BONUS				= $(BONUS_DIR)/main_bonus.cpp \
						  $(addprefix $(BONUS_DIR)/$(SERVER_DIR)/, $(SERVER_SRCS_BONUS)) \
						  $(addprefix $(BONUS_DIR)/$(CLIENT_DIR)/, $(CLIENT_SRCS_BONUS)) \
						  $(addprefix $(BONUS_DIR)/$(UTILS_DIR)/, $(UTILS_SRCS_BONUS))

OBJECTS_BONUS			= $(addprefix $(OBJ_DIR)/, $(SRCS_BONUS:.cpp=.o))

# ──────────────────────────────────────────────────────────────────────────────
# TARGETS
# ──────────────────────────────────────────────────────────────────────────────

.PHONY: all clean fclean re bonus

all: $(NAME)

bonus: $(NAME_BONUS)

$(NAME): $(OBJECTS)
	@$(CC) $(CFLAGS_MAIN) $(OBJECTS) -o $(NAME)
	@echo "$(BOLD_GREEN)Executable ready!$(RESET)"

$(NAME_BONUS): $(OBJECTS_BONUS)
	@$(CC) $(CFLAGS_BONUS) $(OBJECTS_BONUS) -o $(NAME_BONUS)
	@echo "$(BOLD_GREEN)Bonus executable ready!$(RESET)"

$(OBJ_DIR)/%.o: %.cpp $(HEADER_FILE)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS_MAIN) -c $< -o $@
	@echo "$(CYAN)Compiling $<$(RESET)"

$(OBJ_DIR)/$(BONUS_DIR)/%.o: $(BONUS_DIR)/%.cpp $(HEADER_FILE_BONUS)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS_BONUS) -c $< -o $@
	@echo "$(CYAN)Compiling $<$(RESET)"

clean:
	@echo "$(BOLD_RED)Cleaning object files…$(RESET)"
	@$(RM) $(OBJ_DIR)

fclean: clean
	@echo "$(BOLD_RED)Cleaning executables…$(RESET)"
	@$(RM) $(NAME) $(NAME_BONUS)

re: fclean all