/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfFile.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/07 18:09:07 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/11 14:31:11 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFFILE_HPP
#define CONFFILE_HPP

#include "Utils.hpp"

class ConfFile {
	public:
		ConfFile();
		ConfFile(std::string const path);
		~ConfFile();

		static int	getPathType(std::string const path);
		std::string	readFile(std::string path);
		static int	isFileAccessible(std::string const path, int mode);
		static int	checkFile(std::string const path, std::string const index);

		std::string	getPath();
		int			getSize();

	private:
		std::string		_path;
		size_t			_size;
};

#endif
