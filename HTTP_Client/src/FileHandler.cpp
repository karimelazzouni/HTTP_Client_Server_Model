/*
 * FileHandler.cpp
 *
 *  Created on: Oct 23, 2015
 *      Author: karim
 */

#include "FileHandler.h"

void FileHandler::open_file_to_read(ifstream* file_stream, string file_name) {
	file_stream->open(file_name.c_str(), ifstream::in | ifstream::binary);
}

//ofstream FileHandler::open_file_to_write(string file_name) {
//	ofstream
//}

double FileHandler::get_file_size(ifstream* file_stream) { // file stream must be open, not validated
	file_stream->seekg(0, file_stream->end);
	double file_size = file_stream->tellg();
	file_stream->seekg(0, file_stream->beg);
	return file_size;
}

void FileHandler::read_chunk_in_memory(ifstream* file_stream, char* data_read, int chunk_size) {
	data_read = new char[chunk_size];
	file_stream->read(data_read,chunk_size);

}

void FileHandler::read_file_in_memory(ifstream* file_stream, char* buffer) {
	double file_size = get_file_size(file_stream);
	buffer = new char[(int)ceil(file_size)]();
	file_stream->read(buffer,file_size);

}

void FileHandler::concat_to_existing_file(string out_path, char* buffer, int buf_len) {
	ofstream new_file (out_path.c_str(),ofstream::binary | ofstream::app);
	new_file.write(buffer, buf_len);
	new_file.close();
}

void FileHandler::create_file_from_buf(string out_path, char* buffer, int buf_len) {
	ofstream new_file (out_path.c_str(),ofstream::binary);
	new_file.write(buffer, buf_len);
	new_file.close();
}

FileHandler::~FileHandler() {
	// TODO Auto-generated destructor stub
//	delete[] buffer;
}

