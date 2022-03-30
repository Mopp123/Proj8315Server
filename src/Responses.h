#pragma once

#include "ByteBuffer.h"
#include <vector>
#include <string>

class Response
{
private:

	char* _pData = nullptr;
	size_t _contentSize = 0;
	size_t _totalSize = 0;

	const std::string _defaultHeader = 
				"HTTP/1.1 200 OK\n"
  				"Content-Type: application/octet-stream\n"
  				"Access-Control-Allow-Origin: *\n";

	
public:

	Response(char* data, size_t size);
	~Response();

	inline size_t getSize() const { return _totalSize; }
	inline const char* getData() const { return _pData; } 

private:
	std::string createHeader(size_t contentLength) const;
};
