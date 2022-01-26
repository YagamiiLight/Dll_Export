#include<iostream>
#include<fstream>
#include<format>
#include<string>
#include<windows.h>
#include<vector>
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


DWORD rva_convert_foa(DWORD relative_virtual_address, DWORD num_section_headers, PIMAGE_SECTION_HEADER section_header) {

	PIMAGE_SECTION_HEADER next_section_header = PIMAGE_SECTION_HEADER((DWORD64)section_header + (DWORD)0x28);

	for (int num = 0; num < num_section_headers - 1; num++) {

		PIMAGE_SECTION_HEADER next_section_header = PIMAGE_SECTION_HEADER((DWORD64)section_header + (DWORD)0x28);

		if ((section_header->VirtualAddress <= relative_virtual_address) and (relative_virtual_address <= next_section_header->VirtualAddress)) {

			DWORD offset = DWORD(relative_virtual_address - section_header->VirtualAddress);
			return DWORD(section_header->PointerToRawData + offset);

		}

		section_header = next_section_header;

	}

	return NULL;

}

IMAGE_EXPORT_DIRECTORY read_export_directory(fstream& file_stream, DWORD p_export_directory) {
	IMAGE_EXPORT_DIRECTORY export_directory;
	file_stream.seekg(p_export_directory, readmode);
	file_stream.read((char*)&export_directory, sizeof(export_directory));
	return export_directory;

}

vector<string> read_dictory_names(fstream& file_stream, DWORD dictory_names) {

	cout << hex << dictory_names << endl;


	char* name = new char[100];
	vector<string> vec_dictory_names;

	file_stream.seekg(dictory_names, readmode);
	//file_stream.getline(name, 1);
	file_stream.read(name, 100);
	//file_stream.read(reinterpret_cast<char*>(&name), sizeof(name));

	cout << name << endl;
	//dictory_names.emplace_back();

	return vec_dictory_names;
}






DWORD dll_export_dictory_address(fstream& file_stream, char* file_buffer_p) {

	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS nt_header;
	PIMAGE_FILE_HEADER file_header;
	PIMAGE_OPTIONAL_HEADER optional_header;
	PIMAGE_SECTION_HEADER section_header;
	IMAGE_EXPORT_DIRECTORY export_directory;

	size_t size_file = get_file_size(file_stream);

	dos_header = PIMAGE_DOS_HEADER(file_buffer_p);
	nt_header = PIMAGE_NT_HEADERS(file_buffer_p + dos_header->e_lfanew);
	file_header = PIMAGE_FILE_HEADER(&(nt_header->FileHeader));
	optional_header = PIMAGE_OPTIONAL_HEADER(&(nt_header->OptionalHeader));
	section_header = PIMAGE_SECTION_HEADER((DWORD64)optional_header + (file_header->SizeOfOptionalHeader));

	PIMAGE_DATA_DIRECTORY array_data_dictory = optional_header->DataDirectory;
	IMAGE_DATA_DIRECTORY dll_export_dictory = array_data_dictory[0];

	DWORD foa_export_dictory = rva_convert_foa(dll_export_dictory.VirtualAddress, file_header->NumberOfSections, section_header);

	export_directory = read_export_directory(file_stream, foa_export_dictory);

	DWORD foa_export_names_dictory = rva_convert_foa(export_directory.AddressOfNames, file_header->NumberOfSections, section_header);

	read_dictory_names(file_stream, foa_export_names_dictory);



	return foa_export_dictory;
}



void read_export_dll(const string file_name) {

	fstream file_stream;
	file_stream.open(file_name, open_mode);
	size_t size_file = get_file_size(file_stream);

	unique_ptr<char> unique_file_buffer = get_file_buffer(file_stream, size_file);
	char* file_buffer_p = unique_file_buffer.get();
	dll_export_dictory_address(file_stream, file_buffer_p);

	file_stream.close();
}

int main() {

	const string file_path = "C:\\Users\\A\\Desktop\\shit\\kernel32.dll";

	read_export_dll(file_path);

	return 0;
}