/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 18:49:20 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/13 22:49:54 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Utils.hpp"
# include "Request.hpp"
# include "Server.hpp"
# include "Cgi.hpp"

class Response
{
	public:
		Response();
		Response(Request&);
		~Response();

		void		buildResponse();
		void		clearResponse();
		void		breakResponse(size_t);
		std::string	removeUploadBoundary(std::string &body, std::string &boundary);

		std::string	getResponse();
		size_t		getResponseLen() const;
		int			getCode() const;
		int			getCgiState();
		std::string	getMimeType(std::string extension);
		void		setRequest(Request &);
		void		setServer(Server &);
		void		setCgiState(int);
		void		setResponseError(short code);

		Cgi			_cgi_obj;
		Request		request;
		std::string	_response_content;

	private:
		void	fillMimeTypes();
		void	headerContentType();
		void	headerContentLength();
		void	headerConnection();
		void	headerServer();
		void	headerLocation();
		void	headerDate();
		void	setHeaders();
		void	setStatusLine();
		int		buildResponseBody();
		void	setServerDefaultErrorPages();
		int		readFile();
		int		handleTarget();
		void	buildErrorBody();
		bool	requestError();
		int		handleCgi(std::string &);
		int		handleCgiTemp(std::string &);

		Server								_server;
		std::string							_target_file;
		std::vector<uint8_t>				_body;
		size_t								_body_length;
		std::string							_response_body;
		std::string							_location;
		short								_code;
		char								*_res;
		int									_cgi;
		size_t								_cgi_response_length;
		bool								_auto_index;
		std::map<std::string, std::string>	_mime;
};

#endif
