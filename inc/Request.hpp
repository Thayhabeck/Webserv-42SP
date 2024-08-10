/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 21:03:06 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/10 01:11:57 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

# include "Utils.hpp"

class Request
{
    public:
        // Constructor & Destructor
        Request();
        ~Request();

        void        feedRequest(char *data, size_t size);
        bool        isParsed();
        void        printRequest();
        void        clearRequest();
        bool        keepConnected();
        void        breakRequestBody(int bytes);
        
        HttpMethod                                  &getMethod();
        std::string                                 &getPath();
        std::string                                 &getQuery();
        std::string                                 &getFragment();
        std::string                                 getHeader(std::string const &);
		const std::map<std::string, std::string>    &getHeaders() const;
		std::string                                 getMethodStr();
        std::string                                 &getBody();
        std::string                                 getServerName();
        std::string                                 &getUploadBoundary();
        short                                       getErrorCode();
        bool                                        getUploadFlag();
        void                                        setMethod(HttpMethod &);
        void                                        setHeader(std::string &, std::string &);
        void                                        setMaxBodySize(size_t);
        void                                        setBody(std::string name);


    private:
        std::string                         _path;
        std::string                         _query;
        std::string                         _fragment;
        std::map<std::string, std::string>  _request_headers;
        std::vector<u_int8_t>               _body;
        std::string                         _upload_boundary;
        HttpMethod                          _method;
        std::map<u_int8_t, std::string>     _method_str;
        RequestParsingState                 _state;
        size_t                              _max_body_size;
        size_t                              _body_length;
        short                               _error_code;
        size_t                              _chunk_length;
        std::string                         _storage;
        std::string                         _key_storage;
        short                               _method_index;
        u_int8_t                            _ver_major;
        u_int8_t                            _ver_minor;
        std::string                         _server_name;
        std::string                         _body_str;
        /* flags */
        bool                                _fields_done_flag;
        bool                                _body_flag;
        bool                                _body_done_flag;
        bool                                _complete_flag;
        bool                                _chunked_flag;
        bool                                _upload_flag;

        void            _handle_header_fields();

};

#endif