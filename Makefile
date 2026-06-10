NAME = tiramisu

# Project Structure
SRCDIR = src
SRC = Lexer.cpp CLI.cpp main.cpp Parser.cpp SshHandler.cpp \
      commands/Host.cpp commands/Build.cpp commands/Setup.cpp \
      commands/Install.cpp commands/Init.cpp \
      commands/Create.cpp Project.cpp commands/Local.cpp

SRCS = $(addprefix $(SRCDIR)/, $(SRC))
OBJ = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d) # Track header dependencies automatically

# Compiler Settings
CC = c++
CFLAGS = -Wall -Werror -Wextra -O3 --std=c++20 -Iinclude -MMD -MP
LDFLAGS = -lssh

# Installation Paths (Default to standard Linux /usr/local)
PREFIX ?= /usr/local

# --- Core Rules ---

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(NAME)

# Compile source files and generate dependency trees
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# --- Debug Build Configuration ---
debug: CFLAGS = -Wall -Werror -Wextra -g -DDEBUG --std=c++20 -Iinclude -MMD -MP
debug: re

# --- Installation Rules ---
# Run with: sudo make install
install: $(NAME)
	@echo "Installing $(NAME) to $(PREFIX)/bin..."
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp $(NAME) $(DESTDIR)$(PREFIX)/bin/$(NAME)
	@chmod +x $(DESTDIR)$(PREFIX)/bin/$(NAME)
	@echo "=========================================================="
	@echo "$(NAME) binary successfully installed to $(PREFIX)/bin!"
	@echo " Note: Local cluster configs will initialize automatically"
	@echo "       in ~/.tiramisu/ when you run the app as a normal user."
	@echo "=========================================================="

uninstall:
	@echo "Removing $(NAME) from $(PREFIX)/bin..."
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(NAME)
	@echo "$(NAME) removed."

# --- Cleanup Rules ---
clean:
	rm -f $(OBJ) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

# Include compiler-generated dependency files (silently if they don't exist yet)
-include $(DEPS)

.PHONY: all debug install uninstall clean fclean re