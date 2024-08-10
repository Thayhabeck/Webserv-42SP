/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfFile.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/07 18:09:07 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/08 14:50:41 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFFILE_HPP
#define CONFFILE_HPP

#include "Utils.hpp"

class ConfFile {
	
	public:
		
		// Constructors and Destructor
		ConfFile();
		ConfFile(std::string const path);
		~ConfFile();

		// Methods
		static int getPathType(std::string const path);
		std::string	readFile(std::string path);
		static int isFileAccessible(std::string const path, int mode);
		static int checkFile(std::string const path, std::string const index);

		// Getters
		std::string getPath();
		int getSize();

	private:
		std::string		_path;
		size_t			_size;
};

#endif
