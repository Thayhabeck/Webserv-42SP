# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/11/29 13:51:09 by thabeck-          #+#    #+#              #
#    Updated: 2024/08/12 23:59:23 by thabeck-         ###   ########.fr        #
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

# Nova regra para adicionar o server_name ao arquivo hosts
update_hosts:
	@if ! grep -q "127.0.0.1 catarmy" /etc/hosts; then \
		echo "127.0.0.1 catarmy" | sudo tee -a /etc/hosts; \
		echo "$(CY)Server name catsarmy added to /etc/hosts$(RC)"; \
	else \
		echo "$(CY)Server name catsarmy already exists in /etc/hosts$(RC)"; \
	fi
	@if ! grep -q "127.0.0.1 cucumberarmy" /etc/hosts; then \
		echo "127.0.0.1 cucumberarmy" | sudo tee -a /etc/hosts; \
		echo "$(CY)Server name cucumberarmy added to /etc/hosts$(RC)"; \
	else \
		echo "$(CY)Server name cucumberarmy already exists in /etc/hosts$(RC)"; \
	fi

# Altere a regra all para depender de update_hosts
all:	update_hosts $(NAME)

re:		fclean all

clean:
	@$(RM) $(OBJ) $(OBJ_PATH)
	@printf "$(RE)$(NAME) objects removed!$(RC)\n"

fclean:		clean
	@$(RM) $(NAME)
	@printf "$(RE)$(NAME) removed!$(RC)\n"

.PHONY:		all clean fclean re update_hosts
