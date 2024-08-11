/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/07 21:44:38 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/11 14:32:25 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Utils.hpp"
#include "ConfFile.hpp"
#include "Server.hpp"

class Location
{
	public:
		Location();
		Location(const Location &other);
		Location &operator=(const Location &rhs);
		~Location();

		int			 isValidLocation(std::string &serverRoot);
		std::string	printMethods() const;

		void setPath(std::string param);
		void setRootLocation(std::string param);
		void setMethods(std::vector<std::string> methods);
		void setAutoindex(std::string param);
		void setIndexLocation(std::string param);
		void setReturn(std::string param);
		void setAlias(std::string param);
		void setCgiPath(std::vector<std::string> path);
		void setCgiExtension(std::vector<std::string> extension);
		void setMaxBodySize(std::string param);
		void setMaxBodySize(unsigned long param);
		const std::string &getPath() const;
		const std::string &getRootLocation() const;
		const std::vector<short> &getMethods() const;
		const bool &getAutoindex() const;
		const std::string &getIndexLocation() const;
		const std::string &getReturn() const;
		const std::string &getAlias() const;
		const std::vector<std::string> &getCgiPath() const;
		const std::vector<std::string> &getCgiExtension() const;
		const std::map<std::string, std::string> &getExtensionPath() const;
		const unsigned long &getMaxBodySize() const;

		std::map<std::string, std::string> _ext_path;

	private:
		std::string					_path;
		std::string					_root;
		bool						_autoindex;
		std::string					_index;
		std::vector<short>			_allowed_methods;
		std::string					_return;
		std::string					_alias;
		std::vector<std::string>	_cgi_path;
		std::vector<std::string>	_cgi_ext;
		unsigned long				_client_max_body_size;
};

#endif
