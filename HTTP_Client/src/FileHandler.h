/*
 * TextFileHandler.h
 *
 *  Created on: Oct 23, 2015
 *      Author: karim
 */

#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cmath>
using namespace std;

class FileHandler {
public:
	void static open_file_to_read(ifstream* file_stream, string file_name);
	double static get_file_size(ifstream* file_stream);
	void static read_chunk_in_memory(ifstream* file_stream, char* data_read, int chunk_size);
	void static read_file_in_memory(ifstream* file_stream, char* buffer);
	void static concat_to_existing_file(string out_path, char* buffer,int buf_len);
	void static create_file_from_buf(string out_path, char* buffer, int buf_len);
	virtual ~FileHandler();
};

#endif /* FILEHANDLER_H_ */