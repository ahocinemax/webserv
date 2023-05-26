#-----------------------------------------------#
#                   COMPILATION                 #
#-----------------------------------------------#

NAME		= webserv
CFLAGS		= -std=c++98 -g3
CC			= c++

#-----------------------------------------------#
#                     SOURCES                   #
#-----------------------------------------------#

SRCS_DIR	= ./srcs/
SRCS_PATH	= $(addprefix $(SRCS_DIR), $(SRCS))
SRCS		= Client.cpp \
			  CgiHandler.cpp \
			  Epoll.cpp \
			  Location.cpp \
			  main.cpp \
			  Parser.cpp \
			  Request.cpp \
			  Response.cpp \
			  Server.cpp \
			  Utils.cpp \
			  Utils_Cgi.cpp \
			  Webserv.cpp

#-----------------------------------------------#
#                     OBJECTS                   #
#-----------------------------------------------#

OBJS		= $(addprefix $(OBJS_DIR), $(SRCS:.cpp=.o))
OBJS_DIR	= ./objs/

#-----------------------------------------------#
#                    INCLUDES                   #
#-----------------------------------------------#

INCLUDES	= -I$(INC_DIR)
INC_DIR		= ./includes/

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
			@echo "${BOLD}${GREEN}\n----------- Compilation done ! -----------${RESET}"
			@echo "${GREEN}Run ${BOLD}${BLUE}./webserv <path_to_config_file>${RESET}${GREEN} to run the server.${RESET}"

$(NAME):	$(OBJS)
			@echo "${YELLOW}${UNDERLINE}\nLinking...${RESET}${YELLOW}"
			$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(NAME)
			@echo -n "${RESET}"

$(OBJS_DIR)%.o:	$(SRCS_DIR)%.cpp
				@mkdir -p $(OBJS_DIR)
				@echo -n "${CYAN}${UNDERLINE}Compiling $<:${RESET}    ${CYAN}"
				$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
				@echo -n "${RESET}"

clean:
			@echo "${RED}${UNDERLINE}Cleaning objects...${RESET}${RED}"
			$(RM) $(OBJS_DIR)
			@echo -n "${RESET}"

fclean:	clean
			@echo "${RED}${UNDERLINE}Removing executable...${RESET}${RED}"
			$(RM) $(NAME)
			@echo "${RESET}"

re:			fclean all

.PHONY:		all clean fclean re