NAME=tiramisu
SRC=Lexer.cpp CLI.cpp main.cpp Parser.cpp SshHandler.cpp \
	commands/Host.cpp commands/Build.cpp commands/Setup.cpp commands/Webserver.cpp commands/Install.cpp
SRCDIR=src
SRCS=$(addprefix $(SRCDIR)/, $(SRC))
CONFDIR=~/.tiramisu/config/
CC=c++
CFLAGS=-Wall -Werror -Wextra -O3 --std=c++20
OBJ=$(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) -g $(OBJ) -lssh -o $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all
