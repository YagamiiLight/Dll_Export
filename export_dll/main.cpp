#include<iostream>
#include<fstream>
#include<format>
#include<string>
#include<windows.h>
#include"dll_export.hpp"

using namespace std;

unique_ptr<char> get_file_buffer(fstream& file_stream, size_t size_file) {

	unique_ptr<char> file_buffer_p(new char[size_file]);
	file_stream.seekp(0, readmode);
	file_stream.read(file_buffer_p.get(), size_file);

	return file_buffer_p;
}


size_t get_file_size(fstream& file_stream) {

	file_stream.seekg(0, file_stream.end);
	size_t size_file = file_stream.tellg();
	return size_file;
}

DWORD rva_convert_foa() {
	
	return 1;

}


IMAGE_DATA_DIRECTORY get_dll_export_dictory(fstream& file_stream, char* file_buffer_p) {

	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS nt_header;
	PIMAGE_FILE_HEADER file_header;
	PIMAGE_OPTIONAL_HEADER optional_header;
	PIMAGE_SECTION_HEADER section_header;

	size_t size_file = get_file_size(file_stream);

	dos_header = PIMAGE_DOS_HEADER(file_buffer_p);
	nt_header = PIMAGE_NT_HEADERS(file_buffer_p + dos_header->e_lfanew);
	file_header = PIMAGE_FILE_HEADER(&(nt_header->FileHeader));

	optional_header = PIMAGE_OPTIONAL_HEADER(&(nt_header->OptionalHeader));

	section_header = PIMAGE_SECTION_HEADER((DWORD64)optional_header + (file_header->SizeOfOptionalHeader));

	PIMAGE_DATA_DIRECTORY array_data_dictory = optional_header->DataDirectory;
	IMAGE_DATA_DIRECTORY dll_export_dictory = array_data_dictory[0];

	return dll_export_dictory;
}



void read_export_dll(const string file_name) {

	fstream file_stream;
	file_stream.open(file_name, open_mode);
	size_t size_file = get_file_size(file_stream);

	unique_ptr<char> unique_file_buffer = get_file_buffer(file_stream, size_file);
	char* file_buffer_p = unique_file_buffer.get();
	get_dll_export_dictory(file_stream, file_buffer_p);

	file_stream.close();
}

int main() {

	const string file_path = "C:\\Users\\A\\Desktop\\shit\\kernel32.dll";

	read_export_dll(file_path);

	return 0;
}