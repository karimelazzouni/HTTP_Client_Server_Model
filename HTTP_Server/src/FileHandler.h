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
#include <vector>
using namespace std;

class FileHandler {
public:
	void static open_file_to_read(ifstream* file_stream, string file_name);
	double static get_file_size(ifstream* file_stream);
	void static read_chunk_in_memory(ifstream* file_stream, char* data_read, int chunk_size);
	void static read_file_in_memory(ifstream* file_stream, char* buffer);
	void static concat_to_existing_file(string out_path, char* buffer,int buf_len);
	int static create_file_from_buf(string out_path, char* buffer, int buf_len,int size_int);
	void static split(const string& s, char c,vector<string>& v);
	void static split_string(const string& s, string del, vector<string>& v);
	void static replaceAll(string* s, string sub_str, string new_sub_str);
	bool static fexists(const std::string& filename);

	virtual ~FileHandler();
};

#endif /* FILEHANDLER_H_ */
