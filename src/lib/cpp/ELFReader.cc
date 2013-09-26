/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <algorithm>
#include <cstring>
#include <fstream>
#include <istream>
#include <iomanip>

#include "ELFReader.h"
#include "Misc.h"


using namespace std;
using namespace Misc;


namespace ELFReader
{



/*
 * Class 'Section'
 */


Section::Section(File *file, unsigned int pos)
{
	/* Read section header */
	info = (Elf32_Shdr *) (file->GetBuffer() + pos);
	if (pos < 0 || pos + sizeof(Elf32_Shdr) > file->GetSize())
		fatal("%s: invalid position for section header",
				file->GetPath().c_str());

	/* Initialize */
	this->file = file;
	buffer = NULL;
	size = info->sh_size;

	/* Get section contents, if section type is not SHT_NOBITS
	 * (sh_type = 8). */
	if (info->sh_type != 8)
	{
		/* Check valid range */
		if (info->sh_offset + info->sh_size > file->GetSize())
			fatal("%s: section out of range",
					file->GetPath().c_str());

		/* Set up buffer and stream */
		buffer = file->GetBuffer() + info->sh_offset;
		streambuf *buf = stream.rdbuf();
		buf->pubsetbuf(buffer, size);
	}
}



/*
 * Class 'ProgramHeader'
 */


ProgramHeader::ProgramHeader(File *file, unsigned int pos)
{
	/* Initialize */
	this->file = file;

	/* Read program header */
	info = (Elf32_Phdr *) (file->GetBuffer() + pos);
	if (pos < 0 || pos + sizeof(Elf32_Phdr) > file->GetSize())
		fatal("%s: invalid position for program header",
				file->GetPath().c_str());
}



/*
 * Class 'Symbol'
 */

Symbol::Symbol(File *file, Section *section, unsigned int pos)
{
	/* Initialize */
	this->file = file;
	this->section = section;

	/* Read symbol */
	info = (Elf32_Sym *) (section->GetBuffer() + pos);
	if (pos < 0 || pos + sizeof(Elf32_Sym) > section->GetSize())
		fatal("%s: invalid position for symbol",
				file->GetPath().c_str());

	/* Get section with symbol name */
	unsigned name_section_index = section->GetLink();
	Section *name_section = file->GetSection(name_section_index);
	if (!name_section)
		fatal("%s: invalid index for symbol name section",
				file->GetPath().c_str());

	/* Get symbol name */
	if (info->st_name >= name_section->GetSize())
		fatal("%s: invalid symbol name offset",
				file->GetPath().c_str());
	name = name_section->GetBuffer() + info->st_name;
}


bool Symbol::Compare(Symbol *a, Symbol *b)
{
	int bind_a;
	int bind_b;

	/* Sort by value */
	if (a->info->st_value < b->info->st_value)
		return true;
	else if (a->info->st_value > b->info->st_value)
		return false;

	/* Sort symbol with the same address as per their
	 * ST_BIND field in st_info (bits 4 to 8) */
	bind_a = ELF32_ST_BIND(a->info->st_info);
	bind_b = ELF32_ST_BIND(b->info->st_info);
	if (bind_a < bind_b)
		return true;
	else if (bind_a > bind_b)
		return false;
	
	/* Sort alphabetically */
	return a->name < b->name;
}



/*
 * Class 'File'
 */

static const char *err_64bit =
	"\tThe ELF file being loaded is a 64-bit file, currently not supported\n"
	"\tby Multi2Sim. If you are compiling your own source code on a 64-bit\n"
	"\tmachine, please use the '-m32' flag in the gcc command-line. If you\n"
	"\tget compilation errors related with missing '.h' files, check that\n"
	"\tthe 32-bit gcc package associated with your Linux distribution is\n"
	"\tinstalled.\n";

void File::ReadHeader()
{
	/* Read ELF header */
	info = (Elf32_Ehdr *) buffer;
	if (size < sizeof(Elf32_Ehdr))
		fatal("%s: invalid ELF file", path.c_str());

	/* Check that file is a valid ELF file */
	if (strncmp((char *) info->e_ident, ELFMAG, 4))
		fatal("%s: invalid ELF file", path.c_str());

	/* Check that ELF file is a 32-bit object */
	if (info->e_ident[EI_CLASS] == ELFCLASS64)
		fatal("%s: 64-bit ELF not supported.\n%s",
			path.c_str(), err_64bit);
}


void File::ReadSections()
{
	/* Check section size and number */
	if (!info->e_shnum || info->e_shentsize != sizeof(Elf32_Shdr))
		fatal("%s: number of sections is 0 or section size is not %d",
			path.c_str(), (int) sizeof(Elf32_Shdr));

	/* Read section headers */
	for (int i = 0; i < info->e_shnum; i++)
	{
		Section *section = new Section(this, info->e_shoff +
				i * info->e_shentsize);
		sections.push_back(section);
	}

	/* Read string table */
	if (info->e_shstrndx >= info->e_shnum)
		fatal("%s: invalid string table index", path.c_str());
	string_table = sections[info->e_shstrndx];
	if (string_table->info->sh_type != 3)
		fatal("%s: invalid string table type", path.c_str());

	/* Read section names */
	for (auto it = sections.begin(); it != sections.end(); ++it)
		(*it)->name = string_table->buffer + (*it)->info->sh_name;
}


void File::ReadProgramHeaders()
{
	/* Check program header size */
	if (info->e_phentsize != sizeof(Elf32_Phdr))
		fatal("%s: program header size %d (should be %d)",
				path.c_str(), info->e_phentsize,
				(int) sizeof(Elf32_Phdr));
	
	/* Read program headers */
	for (int i = 0; i < info->e_phnum; i++)
	{
		ProgramHeader *ph = new ProgramHeader(this, info->e_phoff +
				i * info->e_phentsize);
		program_headers.push_back(ph);
	}
}


void File::ReadSymbols()
{
	/* Load symbols from sections */
	for (auto it = sections.begin(); it != sections.end(); ++it)
	{
		Section *section = *it;
		if (section->info->sh_type != 2 &&
				section->info->sh_type != 11)
			continue;

		/* Read symbol table */
		int num_symbols = section->info->sh_size / sizeof(Elf32_Sym);
		for (int i = 0; i < num_symbols; i++)
		{
			/* Create symbol */
			Symbol *symbol = new Symbol(this, section,
					i * sizeof(Elf32_Sym));

			/* Discard empty symbol */
			if (symbol->name == "")
			{
				delete symbol;
				continue;
			}

			/* Add symbol */
			symbols.push_back(symbol);
		}
	}

	/* Sort */
	sort(symbols.begin(), symbols.end(), Symbol::Compare);
}


File::File(std::string path)
{
	/* Initialize */
	this->path = path;

	/* Open file */
	ifstream f(path);
	if (!f)
		fatal("%s: cannot open file", path.c_str());

	/* Get file size */
	f.seekg(0, ios_base::end);
	size = f.tellg();
	f.seekg(0, ios_base::beg);

	/* Load entire file into buffer and close */
	buffer = new char[size];
	f.read(buffer, size);
	f.close();

	/* Make string stream point to buffer */
	stringbuf *buf = stream.rdbuf();
	buf->pubsetbuf(buffer, size);

	/* Read content */
	ReadHeader();
	ReadSections();
	ReadProgramHeaders();
	ReadSymbols();
}


File::~File(void)
{
	/* Free sections */
	for (auto it = sections.begin(); it != sections.end(); ++it)
		delete *it;

	/* Free program headers */
	for (auto it = program_headers.begin();
			it != program_headers.end(); ++it)
		delete *it;

	/* Free symbols */
	for (auto it = symbols.begin(); it != symbols.end(); ++it)
		delete *it;

	/* Free content */
	delete buffer;
}


ostream &operator<<(ostream &os, const File &file)
{
	/* Header */
	os << "ELF header:\n";
	os << "  ehdr.e_ident: EI_CLASS=" << (int) file.info->e_ident[5] <<
			", EI_DATA=" << (int) file.info->e_ident[5] <<
			", EI_VERSION=" << (int) file.info->e_ident[6] << "\n";
	os << "  ehdr.e_type: " << file.info->e_type << "\n";
	os << "  ehdr.e_machine: " << file.info->e_machine << "\n";
	os << "  ehdr.e_entry: 0x" << hex << file.info->e_entry << "\n";
	os << "  ehdr.e_phoff: " << file.info->e_phoff << "\n";
	os << "  ehdr.e_shoff: " << file.info->e_shoff << "\n";
	os << "  ehdr.e_phentsize: " << file.info->e_phentsize << "\n";
	os << "  ehdr.e_phnum: " << file.info->e_phnum << "\n";
	os << "  ehdr.e_shentsize: " << file.info->e_shentsize << "\n";
	os << "  ehdr.e_shnum: " << file.info->e_shnum << "\n";
	os << "  ehdr.e_shstrndx: " << file.info->e_shstrndx << "\n";
	os << '\n';

	/* Dump section headers */
	os << "Section headers:\n";
	os << "  [Nr] type flags addr     offset        size     link name\n";
	os << string(80, '-') << '\n';
	for (unsigned i = 0; i < file.sections.size(); i++)
	{
		Section *section = file.sections[i];
		os << "  [" << setw(2) << i << "] ";
		os << setw(4) << section->GetType() << ' ';
		os << setw(5) << section->GetFlags() << ' ';
		os << setfill('0') << setw(8) << section->GetAddr() << ' ';
		os << setw(8) << section->GetOffset() << ' ';
		os << setfill(' ') << setw(9) << section->GetSize() << ' ';
		os << setw(8) << section->GetLink() << ' ';
		os << section->GetName();
		os << '\n';
	}
	os << '\n';
	
	/* Dump program headers */
	os << "Program headers:\n";
	os << "idx type       offset   vaddr    paddr     "
			<< "filesz     memsz  flags align\n";
	os << string(80, '-') << '\n';
	int index = 0;
	for (auto it = file.program_headers.begin();
			it != file.program_headers.end(); ++it)
	{
		ProgramHeader *ph = *it;
		os << setw(3) << index << ' ';
		os << setw(8) << hex << ph->GetType() << ' ' << dec;
		os << setw(8) << hex << ph->GetOffset() << ' ' << dec;
		os << setw(8) << hex << ph->GetVaddr() << ' ' << dec;
		os << setw(8) << hex << ph->GetPaddr() << ' ' << dec;
		os << setw(9) << ph->GetFilesz() << ' ';
		os << setw(9) << ph->GetMemsz() << ' ';
		os << setw(6) << ph->GetFlags() << ' ';
		os << ph->GetAlign() << ' ';
		os << '\n';
		++index;
	}
	os << '\n';

	/* Dump */
	os << "Symbol table:\n";
	os << setw(40) << "name" << " "
			<< setw(15) << "section" << " "
			<< setw(12) << "value" << " "
			<< setw(12) << "size" << " "
			<< setw(10) << "info" << " "
			<< setw(10) << "other" << '\n';
	os << string(80, '-') << '\n';
	for (auto it = file.symbols.begin();
			it != file.symbols.end(); ++it)
	{
		Symbol *symbol = *it;
		
		/* Symbol name */
		os << setiosflags(ios::left);
		os << setw(40) << setiosflags(ios::left) << symbol->GetName() << " ";
		os << resetiosflags(ios::left);

		/* Print section */
		Section *section = symbol->GetSection();
		if (section)
			os << setw(15) << section->GetName() << " ";
		else
			os << setw(15) << symbol->GetShndx() << " ";

		/* Rest */
		os << setw(10) << hex << symbol->GetValue() << ' ';
		os << setw(12) << symbol->GetSize() << ' ';
		os << setw(10) << (int) symbol->GetInfo() << ' ';
		os << setw(10) << (int) symbol->GetOther() << ' ';
		os << '\n';
	}
	os << '\n';

	/* Done */
	return os;
}


Symbol *File::GetSymbol(string name)
{
	/* Search */
	for (auto it = symbols.begin(); it != symbols.end(); ++it)
		if ((*it)->name == name)
			return *it;
	
	/* Not found */
	return NULL;
}


Symbol *File::GetSymbolByAddress(unsigned int address)
{
	unsigned int offset;
	return GetSymbolByAddress(address, offset);
}


Symbol *File::GetSymbolByAddress(unsigned int address, unsigned int &offset)
{
	Symbol *symbol;
	Symbol *prev_symbol;
	
	int min;
	int max;
	int mid;

	/* Empty symbol table */
	if (!symbols.size())
		return NULL;

	/* All symbols in the table have a higher address */
	if (address < symbols[0]->info->st_value)
		return NULL;

	/* Binary search */
	min = 0;
	max = symbols.size();
	while (min + 1 < max)
	{
		mid = (max + min) / 2;
		symbol = symbols[mid];
		if (symbol->info->st_value > address)
		{
			max = mid;
		}
		else if (symbol->info->st_value < address)
		{
			min = mid;
		}
		else
		{
			min = mid;
			break;
		}
	}

	/* Invalid symbol */
	symbol = symbols[min];
	if (!symbol->info->st_value)
		return NULL;

	/* Go backwards to find first symbol with that address */
	for (;;)
	{
		/* One symbol before */
		min--;
		if (min < 0)
			break;

		/* If address is lower, stop */
		prev_symbol = symbols[min];
		if (prev_symbol->info->st_value != symbol->info->st_value)
			break;

		/* Take symbol if it has global/local/weak binding */
		if (ELF32_ST_BIND(prev_symbol->info->st_info) < 3)
			symbol = prev_symbol;
	}

	/* Return the symbol and its address */
	offset = address - symbol->info->st_value;
	return symbol;
}


void File::GetSymbolContent(Symbol *symbol, char *&buffer, unsigned int& size)
{
	/* Symbol with no content */
	buffer = NULL;
	size = 0;
	if (symbol->info->st_shndx >= sections.size())
		return;
	
	/* Symbol exceeds section size */
	Section *section = sections[symbol->info->st_shndx];
	if (symbol->info->st_value + symbol->info->st_size > section->size)
		return;

	/* Get section where the symbol is pointing */
	buffer = section->buffer + symbol->info->st_size;
	size = symbol->info->st_size;
}


void File::GetSymbolContent(Symbol *symbol, istringstream& ss)
{
	char *buffer;
	unsigned int size;

	/* Get content */
	GetSymbolContent(symbol, buffer, size);

	/* Set up string stream */
	stringbuf *buf = ss.rdbuf();
	buf->pubsetbuf(buffer, size);
}



} /* namespace ELFReader */
