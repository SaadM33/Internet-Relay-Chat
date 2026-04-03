
CC = c++
CFLAGS = -g  -std=c++98
NAME = ircserv
SRCS = *.cpp
OBJS = $(SRCS:.cpp=.o)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

fresh: all clean
	clear -x

all: $(NAME)

run: fresh
	./$(NAME) input.txt


$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

