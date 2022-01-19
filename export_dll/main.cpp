#include<iostream>
#include<fstream>
#include<windows.h>
#include"dll_export.hpp"

using namespace std;

unique_ptr<char> get_file_buffer(fstream& file_stream) {
	file_stream.seekg(0, file_stream.end);
	size_t size_file = file_stream.tellg();

	unique_ptr<char> file_buffer_p(new char(size_file));
	file_stream.read(file_buffer_p.get(), size_file);

	return file_buffer_p;
}


void read_export_dll(const string file_name) {
	char* file_buffer_p;
	fstream file_stream;

	file_stream.open(file_name);
	PIMAGE_DOS_HEADER dos_header = NULL;
	PIMAGE_FILE_HEADER file_header = NULL;
	PIMAGE_OPTIONAL_HEADER32 optional_header = NULL;

	unique_ptr<char> unique_file_buffer = get_file_buffer(file_stream);
	file_buffer_p = unique_file_buffer.get();

	PIMAGE_DATA_DIRECTORY data_dictory = optional_header->DataDirectory;
	IMAGE_DATA_DIRECTORY data_dictory_export = data_dictory[0];

	file_stream.close();
}

int main() {

	const string file_path = "C:\\Users\\chris\\Desktop\\chaos\\kernel32.dll";

	read_export_dll(file_path);

	return 0;
}