/*
 * FileHandler.cpp
 *
 *  Created on: Oct 23, 2015
 *      Author: karim
 */

#include "FileHandler.h"
#include <vector>

void FileHandler::open_file_to_read(ifstream* file_stream, string file_name) {
	file_stream->open(file_name.c_str(), ifstream::in | ifstream::binary);
}

double FileHandler::get_file_size(ifstream* file_stream) { // file stream must be open, not validated
	file_stream->seekg(0, file_stream->end);
	double file_size = file_stream->tellg();
	file_stream->seekg(0, file_stream->beg);
	return file_size;
}

void FileHandler::read_chunk_in_memory(ifstream* file_stream, char* data_read, int chunk_size) {
	file_stream->read(data_read,chunk_size);

}

void FileHandler::read_file_in_memory(ifstream* file_stream, char* buffer) {
	double file_size = get_file_size(file_stream);
	buffer = new char[(int)ceil(file_size)]();
	file_stream->read(buffer,file_size);

}

void FileHandler::concat_to_existing_file(string out_path, char* buffer, int buf_len) {
	cout<<"ABO: "<<out_path<<endl;
	ofstream new_file (out_path.c_str(),ofstream::binary | ofstream::app);
	new_file.write(buffer, buf_len);
	new_file.close();
}

int FileHandler::create_file_from_buf(string out_path, char* buffer, int buf_len,int size_int) {
	ofstream new_file (out_path.c_str(),ofstream::binary);
	new_file.write(buffer, min(buf_len,size_int));
	new_file.close();
	return min(buf_len,size_int);
}

FileHandler::~FileHandler() {
	// TODO Auto-generated destructor stub
//	delete[] buffer;
}



void FileHandler::split(const string& s, char c, vector<string>& v) {
	string::size_type i = 0;
	string::size_type j = s.find(c);

	while (j != string::npos) {
		v.push_back(s.substr(i, j - i));
		i = ++j;
		j = s.find(c, j);

		if (j == string::npos)
			v.push_back(s.substr(i, s.length()));
	}
}

void FileHandler::split_string(const string& s, string del, vector<string>& v) {
	if (del.compare("") == 0) {
		v.push_back(s);
		return;
	}
	string::size_type i = 0;
	string::size_type j = s.find(del);

	while (j != string::npos) {
		v.push_back(s.substr(i, j - i));
		j += del.length();
		i = j;
		j = s.find(del, j);

		if (j == string::npos)
			v.push_back(s.substr(i, s.length()));
	}
}

void FileHandler::replaceAll(string* s, string sub_str, string new_sub_str) {
	int index_to_start_search_at = 0;
	while(1) {
		/* Locate the substring to replace. */
		 index_to_start_search_at = s->find(sub_str, index_to_start_search_at);
		 if (index_to_start_search_at == std::string::npos) break;

		 /* Make the replacement. */
		 s->replace(index_to_start_search_at, strlen(sub_str.c_str()), new_sub_str);

		 /* Advance index forward so the next iteration doesn't pick it up as well. */
		 index_to_start_search_at += strlen(new_sub_str.c_str());
	}
}

bool FileHandler::fexists(const std::string& filename) {
	ifstream ifile(filename.c_str());
	return ifile;
}
