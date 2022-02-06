#include<iostream>
#include<fstream>
#include<string>
#include<windows.h>
#include<any>
#include<sstream>
#include"dll_export.hpp"

using namespace std;

unique_ptr<char> get_file_buffer(fstream& file_stream, size_t size_file)
{
	unique_ptr<char> file_buffer_p(new char[size_file]);
	file_stream.seekp(0, readmode);
	file_stream.read(file_buffer_p.get(), size_file);

	return file_buffer_p;
}


size_t get_file_size(fstream& file_stream)
{
	file_stream.seekg(0, file_stream.end);
	size_t size_file = file_stream.tellg();
	return size_file;
}


DWORD rva_convert_foa(DWORD relative_virtual_address, DWORD num_section_headers, PIMAGE_SECTION_HEADER section_header)
{
	PIMAGE_SECTION_HEADER next_section_header = PIMAGE_SECTION_HEADER((DWORD64)section_header + (DWORD)0x28);

	for (int num = 0; num < num_section_headers - 1; num++)
	{
		PIMAGE_SECTION_HEADER next_section_header = PIMAGE_SECTION_HEADER((DWORD64)section_header + (DWORD)0x28);

		if ((section_header->VirtualAddress <= relative_virtual_address) and (relative_virtual_address <=
			next_section_header->VirtualAddress))
		{
			DWORD offset = DWORD(relative_virtual_address - section_header->VirtualAddress);
			return DWORD(section_header->PointerToRawData + offset);
		}

		section_header = next_section_header;
	}

	return NULL;
}

IMAGE_EXPORT_DIRECTORY read_export_directory(fstream& file_stream, DWORD p_export_directory)
{
	IMAGE_EXPORT_DIRECTORY export_directory;
	file_stream.seekg(p_export_directory, readmode);
	file_stream.read((char*)&export_directory, sizeof(export_directory));
	return export_directory;
}

DWORD read_foa_dictory(fstream& file_stream, DWORD rva_dictory)
{
	DWORD foa_dictory;
	file_stream.seekg(rva_dictory, readmode);
	file_stream.read((char*)(&foa_dictory), sizeof(foa_dictory));
	return foa_dictory;
}

WORD read_name_ordinal(fstream& file_stream, DWORD rva_dictory)
{
	WORD name_ordinal;
	file_stream.seekg(rva_dictory, readmode);
	file_stream.read((char*)(&name_ordinal), sizeof(name_ordinal));
	return name_ordinal;
}

void get_names_dictory(fstream& file_stream, DWORD num_sections, DWORD num_names, PIMAGE_SECTION_HEADER section_header,
                       DWORD rva_names_dictory)
{
	DWORD name_address;
	string name;

	for (int num = 0; num < num_names; num++)
	{
		DWORD rva_names_header = read_foa_dictory(file_stream, rva_names_dictory);

		DWORD foa_names_header = rva_convert_foa(rva_names_header, num_sections, section_header);
		file_stream.seekg(foa_names_header, readmode);
		getline(file_stream, name, char(0x00));
		// cout << name << endl;
		rva_names_dictory = rva_names_dictory + 4;
	}
}

void get_names_ordinals_dictory(fstream& file_stream, DWORD num_names, DWORD foa_names_ordinals_dictory)
{
	for (int num = 0; num < num_names; num++)
	{
		WORD name_ordinal = read_name_ordinal(file_stream, foa_names_ordinals_dictory);
		cout << hex << name_ordinal << endl;
		foa_names_ordinals_dictory = foa_names_ordinals_dictory + 2;
	}
}

void get_functions_dictory(fstream& file_stream, DWORD num_functions, DWORD foa_functions_dictory)
{
	for (int num = 0; num < num_functions; num++)
	{
		DWORD function_address = read_foa_dictory(file_stream, foa_functions_dictory);
		cout << hex << function_address << endl;
		foa_functions_dictory = foa_functions_dictory + 4;
	}
	
}

//vector<map<string, any>>
void get_dictorys(fstream& file_stream, IMAGE_DATA_DIRECTORY dll_export_dictory, DWORD num_section_headers,
                  PIMAGE_SECTION_HEADER section_header)
{
	DWORD foa_export_dictory = rva_convert_foa(dll_export_dictory.VirtualAddress, num_section_headers, section_header);
	IMAGE_EXPORT_DIRECTORY export_dictory = read_export_directory(file_stream, foa_export_dictory);

	DWORD address_functions_dictory = export_dictory.AddressOfFunctions;
	DWORD address_names_dictory = export_dictory.AddressOfNames;
	DWORD address_name_ordinals_dictory = export_dictory.AddressOfNameOrdinals;

	DWORD foa_functions_dictory = rva_convert_foa(address_functions_dictory, num_section_headers, section_header);
	DWORD foa_names_dictory = rva_convert_foa(address_names_dictory, num_section_headers, section_header);
	DWORD foa_name_ordinals_dictory = rva_convert_foa(address_name_ordinals_dictory, num_section_headers,
	                                                  section_header);

	DWORD rva_functions_header = read_foa_dictory(file_stream, foa_functions_dictory);
	DWORD rva_names_header = read_foa_dictory(file_stream, foa_names_dictory);
	DWORD rva_name_ordinals_header = read_foa_dictory(file_stream, foa_name_ordinals_dictory);

	DWORD foa_functions_header = rva_convert_foa(rva_functions_header, num_section_headers, section_header);
	DWORD foa_names_header = rva_convert_foa(rva_names_header, num_section_headers, section_header);
	DWORD foa_name_ordinals_header = rva_convert_foa(rva_name_ordinals_header, num_section_headers, section_header);

	DWORD num_names = export_dictory.NumberOfNames;

	get_names_dictory(file_stream, num_section_headers, num_names, section_header, foa_names_dictory);
	get_names_ordinals_dictory(file_stream,num_names, foa_name_ordinals_dictory);
	get_functions_dictory(file_stream, num_names, foa_functions_dictory);
}


DWORD dll_export_dictory_address(fstream& file_stream, char* file_buffer_p)
{
	PIMAGE_DOS_HEADER dos_header;
	PIMAGE_NT_HEADERS nt_header;
	PIMAGE_FILE_HEADER file_header;
	PIMAGE_OPTIONAL_HEADER optional_header;
	PIMAGE_SECTION_HEADER section_header;

	size_t size_file = get_file_size(file_stream);

	dos_header = PIMAGE_DOS_HEADER(file_buffer_p);
	nt_header = PIMAGE_NT_HEADERS(file_buffer_p + dos_header->e_lfanew);
	file_header = PIMAGE_FILE_HEADER(&(nt_header->FileHeader));
	optional_header = PIMAGE_OPTIONAL_HEADER((DWORD64)&(nt_header->OptionalHeader));
	section_header = PIMAGE_SECTION_HEADER((DWORD64)optional_header + (file_header->SizeOfOptionalHeader));

	PIMAGE_DATA_DIRECTORY array_data_dictory = optional_header->DataDirectory;
	IMAGE_DATA_DIRECTORY dll_export_dictory = array_data_dictory[0];

	DWORD num_section_headers = file_header->NumberOfSections;
	get_dictorys(file_stream, dll_export_dictory, num_section_headers, section_header);


	return 1;
}


void read_export_dll(const string file_name)
{
	fstream file_stream;
	file_stream.open(file_name, open_mode);
	size_t size_file = get_file_size(file_stream);

	unique_ptr<char> unique_file_buffer = get_file_buffer(file_stream, size_file);
	char* file_buffer_p = unique_file_buffer.get();
	dll_export_dictory_address(file_stream, file_buffer_p);

	file_stream.close();
}

int main()
{
	const string file_path = "C:\\Users\\chris\\Desktop\\chaos\\kernel32.dll";

	read_export_dll(file_path);

	return 0;
}
