# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/11/29 13:51:09 by thabeck-          #+#    #+#              #
#    Updated: 2024/08/13 22:25:07 by thabeck-         ###   ########.fr        #
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

all:	update_hosts $(NAME)

update_hosts:
	@if ! grep -q "127.0.0.1 catsarmy" /etc/hosts; then \
		echo "127.0.0.1 catsarmy" | sudo tee -a /etc/hosts; \
		echo "$(CY)Server name catsarmy added to /etc/hosts$(RC)"; \
	else \
		echo "$(CY)Server name catsarmy already exists in /etc/hosts$(RC)"; \
	fi
	@if ! grep -q "127.0.0.1 cucumbersarmy" /etc/hosts; then \
		echo "127.0.0.1 cucumbersarmy" | sudo tee -a /etc/hosts; \
		echo "$(CY)Server name cucumbersarmy added to /etc/hosts$(RC)"; \
	else \
		echo "$(CY)Server name cucumberarmy already exists in /etc/hosts$(RC)"; \
	fi


re:		fclean all

clean:
	@$(RM) $(OBJ) $(OBJ_PATH)
	@printf "$(RE)$(NAME) objects removed!$(RC)\n"

fclean:		clean
	@$(RM) $(NAME)
	@printf "$(RE)$(NAME) removed!$(RC)\n"

.PHONY:		all clean fclean re update_hosts
