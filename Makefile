#-----------------------------------------------#
#                   COMPILATION                 #
#-----------------------------------------------#

NAME		= webserv

FLAGS		= -Wall -Wextra -Werror -std=c++98 -g3
INCLUDE		= -I $(HEAD_DIR) -lXext -lX11 -lm
CC			= c++

#-----------------------------------------------#
#                     SOURCES                   #
#-----------------------------------------------#

SRCS_DIR	= ./srcs/
SRCS_PATH	= $(addprefix $(SRCS_DIR), $(SRCS))

SRCS		= main.cpp \
			  Server.cpp \
			  Client.cpp \
			  Request.cpp \
			  Location.cpp \
			  Utils.cpp \
			  Parser.cpp \

#-----------------------------------------------#
#                     OBJECTS                   #
#-----------------------------------------------#

OBJS		= $(addprefix $(OBJS_DIR), $(SRCS:.cpp=.o))
OBJS_DIR	= ./objs/

#-----------------------------------------------#
#                    INCLUDES                   #
#-----------------------------------------------#

INC_DIR		= ./includes/

INCLUDES	= -I$(INC_DIR)

#-----------------------------------------------#
#                     COLORS                    #
#-----------------------------------------------#

RESET		= \033[0m
BOLD		= \033[1m
UNDERLINE	= \033[4m
BLINK		= \033[5m

BLACK		= \033[30m
RED			= \033[31m
GREEN		= \033[32m
YELLOW		= \033[33m
BLUE		= \033[34m
MAGENTA		= \033[35m
CYAN		= \033[36m
WHITE		= \033[37m

#-----------------------------------------------#
#                     OTHERS                    #
#-----------------------------------------------#

RM			= rm -rf

#-----------------------------------------------#
#                     RULES                     #
#-----------------------------------------------#

all:		$(NAME)

$(NAME):	$(OBJS)
			$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(NAME)

$(OBJS_DIR)%.o:	$(SRCS_DIR)%.cpp
				@mkdir -p $(OBJS_DIR)
				$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
			$(RM) $(OBJS_DIR)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re