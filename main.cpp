#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <cstdint>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>  // for readlink
    #include <elf.h>
#endif

enum class BinaryType {
    ELF,
    PE,
    Unknown
};

BinaryType detect_file_type(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return BinaryType::Unknown;

    std::array<uint8_t, 4> magic{};
    file.read(reinterpret_cast<char*>(magic.data()), magic.size());

    if (magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')
        return BinaryType::ELF;

    if (magic[0] == 'M' && magic[1] == 'Z')
        return BinaryType::PE;

    return BinaryType::Unknown;
}

#ifdef _WIN32

void parse_pe(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << path << "\n";
        return;
    }

    IMAGE_DOS_HEADER dosHeader;
    file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "Not a valid PE file.\n";
        return;
    }

    file.seekg(dosHeader.e_lfanew, std::ios::beg);
    DWORD signature;
    file.read(reinterpret_cast<char*>(&signature), sizeof(signature));
    if (signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "Invalid NT signature.\n";
        return;
    }

    IMAGE_FILE_HEADER fileHeader;
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));

    IMAGE_OPTIONAL_HEADER64 optionalHeader;
    file.read(reinterpret_cast<char*>(&optionalHeader), sizeof(optionalHeader));

    std::cout << "\nPE Metadata:\n";
    std::cout << "- Machine: 0x" << std::hex << fileHeader.Machine << "\n";
    std::cout << "- Sections: " << std::dec << fileHeader.NumberOfSections << "\n";
    std::cout << "- Entry point RVA: 0x" << std::hex << optionalHeader.AddressOfEntryPoint << "\n";
    std::cout << "- Image base: 0x" << optionalHeader.ImageBase << "\n";
    std::cout << "- Subsystem: " << optionalHeader.Subsystem << "\n";
    std::cout << "- Size of image: 0x" << optionalHeader.SizeOfImage << "\n";

    // Section Headers
    std::cout << "\nSections:\n";
    file.seekg(dosHeader.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + fileHeader.SizeOfOptionalHeader, std::ios::beg);
    for (int i = 0; i < fileHeader.NumberOfSections; ++i) {
        IMAGE_SECTION_HEADER section;
        file.read(reinterpret_cast<char*>(&section), sizeof(section));
        std::cout << "  - " << std::string(reinterpret_cast<char*>(section.Name), 8)
                  << "\n    RVA: 0x" << std::hex << section.VirtualAddress
                  << ", Size: 0x" << section.Misc.VirtualSize << "\n";
    }
}

#else // Linux ELF parsing

void parse_elf(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << path << "\n";
        return;
    }

    // Read ELF header
    Elf64_Ehdr elfHeader;
    file.read(reinterpret_cast<char*>(&elfHeader), sizeof(elfHeader));

    if (!(elfHeader.e_ident[0] == 0x7F && elfHeader.e_ident[1] == 'E' &&
          elfHeader.e_ident[2] == 'L' && elfHeader.e_ident[3] == 'F')) {
        std::cerr << "Not a valid ELF file.\n";
        return;
    }

    std::cout << "\nELF Metadata:\n";
    std::cout << "- Entry point: 0x" << std::hex << elfHeader.e_entry << "\n";
    std::cout << "- Number of program headers: " << std::dec << elfHeader.e_phnum << "\n";
    std::cout << "- Number of section headers: " << elfHeader.e_shnum << "\n";

    // Read section headers
    file.seekg(elfHeader.e_shoff, std::ios::beg);
    std::vector<Elf64_Shdr> sections(elfHeader.e_shnum);
    for (size_t i = 0; i < elfHeader.e_shnum; ++i) {
        file.read(reinterpret_cast<char*>(&sections[i]), sizeof(Elf64_Shdr));
    }

    // Read section header string table to get section names
    if (elfHeader.e_shstrndx >= elfHeader.e_shnum) {
        std::cerr << "Invalid section header string table index.\n";
        return;
    }
    Elf64_Shdr& shstrtab = sections[elfHeader.e_shstrndx];
    std::vector<char> shstrtabData(shstrtab.sh_size);
    file.seekg(shstrtab.sh_offset, std::ios::beg);
    file.read(shstrtabData.data(), shstrtab.sh_size);

    std::cout << "\nSections:\n";
    for (size_t i = 0; i < sections.size(); ++i) {
        const char* name = shstrtabData.data() + sections[i].sh_name;
        std::cout << "  - " << name
                  << " (Addr: 0x" << std::hex << sections[i].sh_addr
                  << ", Size: 0x" << sections[i].sh_size << ")\n";
    }
}

#endif

int main() {
    std::string exePath;

#ifdef _WIN32
    char pathBuffer[MAX_PATH];
    GetModuleFileNameA(nullptr, pathBuffer, MAX_PATH);
    exePath = pathBuffer;
#else
    char pathBuffer[1024];
    ssize_t len = readlink("/proc/self/exe", pathBuffer, sizeof(pathBuffer) - 1);
    if (len == -1) {
        std::cerr << "Failed to get executable path\n";
        return 1;
    }
    pathBuffer[len] = '\0';
    exePath = pathBuffer;
#endif

    std::cout << "Analyzing: " << exePath << "\n";

    auto type = detect_file_type(exePath);

    if (type == BinaryType::PE) {
#ifdef _WIN32
        parse_pe(exePath);
#else
        std::cerr << "PE parsing supported only on Windows\n";
#endif
    }
    else if (type == BinaryType::ELF) {
#ifndef _WIN32
        parse_elf(exePath);
#else
        std::cerr << "ELF parsing supported only on Linux\n";
#endif
    }
    else {
        std::cerr << "Unknown or unsupported binary format\n";
    }

    return 0;
}
