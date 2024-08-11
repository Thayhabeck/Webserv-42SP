# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/11/29 13:51:09 by thabeck-          #+#    #+#              #
#    Updated: 2024/08/10 09:14:41 by matcardo         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	webserv

CC		=	c++
CFLAGS	=	-Wall -Wextra -Werror -std=c++98

RM		=	rm -rf

SRC 	=	main.cpp \
			Webserv.cpp \
			Utils.cpp \
			Server.cpp \
			ServerConf.cpp \
			Location.cpp \
			Request.cpp \
			Response.cpp \
			Client.cpp \
			ConfFile.cpp \
			Cgi.cpp

OBJ_PATH	= obj/
OBJ 		= $(addprefix $(OBJ_PATH), $(SRC:.cpp=.o))

CY		= \033[36;1m
GR		= \033[32;1m
RE		= \033[31;1m
RC		= \033[0m

$(OBJ_PATH)%.o:	src/%.cpp
	@mkdir -p $(OBJ_PATH)
	@$(CC) $(CFLAGS) -c $< -o $@

$(NAME): 	$(OBJ) 
	@printf "$(CY)Generating $(NAME)...$(RC)\n"
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJ)
	@printf "$(GR)Done!$(RC)\n"

all:	$(NAME)

re:		fclean all

clean:
	@$(RM) $(OBJ) $(OBJ_PATH)
	@printf "$(RE)$(NAME) objects removed!$(RC)\n"

fclean:		clean
	@$(RM) $(NAME)
	@printf "$(RE)$(NAME) removed!$(RC)\n"

.PHONY:		all clean fclean re
