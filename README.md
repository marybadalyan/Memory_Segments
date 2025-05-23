# Memory_Segments

## What are memory segments and what do they do for our programms 

Memory segments are distinct regions in a program's memory layout, each serving a specific purpose during execution. They are managed by the operating system and the compiler to organize a program's data and instructions efficiently.

# Overview of Memory Segments
A program's memory is typically divided into the following segments:

## Text (Code) Segment:
- Purpose: Stores the compiled machine code (instructions) of the program.
- Access: Read-only, executable.
- Typical Size: Depends on the program's complexity (e.g., a few KB to MB).
- Mapping: Loaded from the executable file's text section into memory.

## Data Segment:
- Purpose: Stores initialized global and static variables (e.g., int x = 10;).
- Access: Read-write.
- Typical Size: Proportional to the number of initialized variables (e.g., KB).
- Mapping: Loaded from the executable's data section.

## BSS Segment:
- Purpose: Stores uninitialized global and static variables (e.g., static int y;).
- Access: Read-write.
- Typical Size: Proportional to uninitialized variables (e.g., KB). Often zeroed out by the OS, so it doesn't   occupy space in the executable file.
- Mapping: Allocated at runtime, initialized to zero.

## Stack Segment:
- Purpose: Stores local variables, function call frames, and return addresses.
- Access: Read-write.
- Typical Size: Fixed or dynamically adjustable (e.g., 1 MB default on Linux, adjustable via ulimit).
- Mapping: Grows downward (in most architectures) and is managed by the OS.

## Heap Segment:
- Purpose: Stores dynamically allocated memory (e.g., via malloc or new).
- Access: Read-write.
- Typical Size: Grows as needed, limited by system memory (e.g., MB to GB).
- Mapping: Managed by the runtime library and OS, grows upward.

## Build Instructions

1. Clone the repository:
   ```
   git clone https://github.com/username/Memory_Segments
   ```
2. Navigate to the repository:
   ```
   cd Memory_Segments
   ```
3. Generate build files:
   ```
   cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
   ```
4. Build the project:
   ```
   cmake --build build --config Release
   ```
5. Run the executable from the build directory:
   ```
   ./build/Memory_Segments
   ```

2. Why Optimization Flags Affect Segment Sizes
Compiler optimization flags (e.g., -O0, -O1, -O2, -O3 in GCC) modify how the compiler generates code, which impacts memory segment sizes:

# Text Segment:
 - Optimization Impact: Higher optimization levels (-O2, -O3) inline functions, unroll loops, and remove redundant  instructions, reducing code size in some cases. However, aggressive optimizations like loop unrolling can increase code size if more instructions are generated.
- Example: -O0 (no optimization) includes debugging info and unoptimized code, inflating the text segment. -O3 may  shrink it by removing dead code but could grow it with inlining.

# Data and BSS Segments:
- Optimization Impact: Optimizations may reduce the number of global/static variables by promoting them to registers or eliminating unused variables, shrinking these segments.
- Example: A variable marked static but unused may be removed with -O2, reducing BSS size.

# Stack Segment:
- Optimization Impact: Optimizations reduce stack usage by minimizing local variables or reusing stack space. For example, -O2 may eliminate temporary variables, reducing stack frame size.
- Example: A function with many local arrays may use less stack space with -O3 due to variable reuse.

# Heap Segment:
- Optimization Impact: Indirectly affected if optimizations reduce dynamic memory allocations (e.g., by reusing memory or avoiding unnecessary allocations).
- Example: A loop allocating memory repeatedly might be optimized to reuse a single allocation, reducing heap usage.


### Example: Segment Size Differences

Compiled with `-O2/-O3`:
```
.data name
    1D0 virtual size
    5000 virtual address (0000000140005000 to 00000001400051CF)
```

Compiled with `-O0`:
```
.data name
    C31 virtual size
    E000 virtual address (000000014000E000 to 000000014000EC30)
```

We observe that the `.data` segment size increases significantly at `-O0` due to unoptimized code and debug information.

---

## 4. Analyzing Segments with `dumpbin`

In the `cmake-multi-platform.yml`, we use:

```cmake
COMMAND cmd /c "dumpbin /headers \"$<TARGET_FILE:memory_segments>\" > \"${CMAKE_BINARY_DIR}/analysis/memory_segments_headers.txt\""
```

This redirects the output of `dumpbin /headers` to a `.txt` file. By inspecting it and correlating with `main.cpp`, we can track where variables are placed in memory.

Example:
```cpp
int global_var_initialized = 42;
```
Shows up in:
```
009 00000000 SECT3  notype       External     | ?global_var_initialized@@3HA
```
Which maps to `.data` in the memory segment headers.

Similarly:
```cpp
int global_var_uninitialized;
```
Maps to:
```
00C 00000000 SECT4  notype       External     | ?global_var_uninitialized@@3HA
```
Which is part of `.bss`.

Other variables like:
```cpp
const char* msg = "hello";
```
Appear in `.rdata`.

---

## 5. Tracking Local Variables

Variables defined inside `main()` or other functions are not present in `.data` or `.bss`. Instead, they reside on the **stack**, which is only visible at **runtime** via a debugger.

**Note:**
- A local `static` variable **with initialization** (e.g., `static int x = 10;`) is stored in the `.data` segment.
- A local `static` variable **without initialization** (e.g., `static int y;`) is stored in `.bss`.
- Though local in scope, they are **not** stored on the stack because their lifetime is the entire program.

Function code always resides in the **text** segment. The `static` keyword affects **linkage**, not memory placement.


!Note 
If initialized  a local ststic varaible (e.g., static int x = 10;), stored in the data segment.
If uninitialized (e.g., static int y;), stored in the BSS segment. 
Despite being local to a function, it’s not stored on the stack because its lifetime is the entire program.

The function’s code resides in the text (code) segment, like any other function. The static keyword only affects linkage, not memory placement.

## Overview of the Cross-Platform PE & ELF Parser

On different platforms, executable files have different formats: ELF (for Linux) and PE (for Windows).

we determine the type of the executable using `BinaryType` struct  and `detect_file_type` function.
### Windows PE Parsing.

Step-by-step:

- Open the executable file in binary mode.

- Read IMAGE_DOS_HEADER:

- The DOS header is the very first structure in a PE file.

- Check e_magic — should be 0x5A4D ('MZ'), confirming a DOS stub exists.

Jump to NT Headers:

- The e_lfanew field in the DOS header tells us where the PE header (NT headers) start.

- We seek to that offset.

Read PE Signature:

- It should be IMAGE_NT_SIGNATURE (usually "PE\0\0").

- Validates that this is a PE file.

Read IMAGE_FILE_HEADER:

- Contains machine type, number of sections, timestamp, etc.

Read IMAGE_OPTIONAL_HEADER64:

Contains lots of info: entry point, image base address, section alignment, subsystem, etc.

Print metadata:

- Machine architecture (x64, x86, ARM)

- Number of sections (like .text, .rdata, .data)

- Entry point RVA (relative virtual address)

- Image base address (where the executable is loaded in memory)

- Subsystem (console, GUI, etc.)

- Size of the image in memory

Read Section Headers:

- After optional header, the section headers start.

Loop through each section:

- Print name (8 chars max)

- Virtual address (RVA)

- Virtual size

### Linux ELF Parsing

Step-by-step:

- Open the ELF file in binary mode.

Read the ELF header (Elf64_Ehdr):

- The first 16 bytes (e_ident) must be: 0x7F 'E' 'L' 'F'

- Validate magic number to confirm it's ELF.

Print some key metadata:

- Entry point address (where execution starts)

- Number of program headers (segments loaded into memory)

- Number of section headers (sections like .text, .data, .bss)

Read the Section Header Table:

- Seek to section header offset e_shoff.

- Read all section headers (Elf64_Shdr) into a vector.

Read the Section Header String Table:

- Section names are stored as offsets into a special string table.

- The index of this string table section is e_shstrndx.

- Load that string table from the file.

Print the section names, addresses, and sizes:

- Using the section header string table, get readable section names.

# Example output:

## Windows
```

PE Metadata:
- Machine: 0x8664
- Sections: 6
- Entry point RVA: 0x3cdc
- Image base: 0x140000000
- Subsystem: 3
- Size of image: 0xf000
- Size of headers: 0x400
- Number of sections: 6
- Size of headers: 0x400

Sections:
  - .text
    Virtual Size:       0x3a6b
    Virtual Address:    0x1000
    Size of Raw Data:   0x3c00
    Pointer to Raw Data:0x400
    Characteristics:    0x60000020
  - .rdata
    Virtual Size:       0x5996
    Virtual Address:    0x5000
    Size of Raw Data:   0x5a00
    Pointer to Raw Data:0x4000
    Characteristics:    0x40000040
  - .data
    Virtual Size:       0x938
    Virtual Address:    0xb000
    Size of Raw Data:   0x400
    Pointer to Raw Data:0x9a00
    Characteristics:    0xc0000040
  - .pdata
    Virtual Size:       0x498
    Virtual Address:    0xc000
    Size of Raw Data:   0x600
    Pointer to Raw Data:0x9e00
    Characteristics:    0x40000040
  - .rsrc
    Virtual Size:       0x1e0
    Virtual Address:    0xd000
    Size of Raw Data:   0x200
    Pointer to Raw Data:0xa400
    Characteristics:    0x40000040
  - .reloc
    Virtual Size:       0x9c
    Virtual Address:    0xe000
    Size of Raw Data:   0x200
    Pointer to Raw Data:0xa600
    Characteristics:    0x42000040

PE parsing completed.

```
## Linux

```
ELF Metadata:
- Entry point: 0x1160
- Number of program headers: 13
- Number of section headers: 32

Sections:
  -  (Addr: 0x0, Size: 0x0)
  - .interp (Addr: 0x318, Size: 0x1c)
  - .note.gnu.property (Addr: 0x338, Size: 0x20)
  - .note.gnu.build-id (Addr: 0x358, Size: 0x24)
  - .note.ABI-tag (Addr: 0x37c, Size: 0x20)
  - .gnu.hash (Addr: 0x3a0, Size: 0x30)
  - .dynsym (Addr: 0x3d0, Size: 0x2d0)
  - .dynstr (Addr: 0x6a0, Size: 0x418)
  - .gnu.version (Addr: 0xab8, Size: 0x3c)
  - .gnu.version_r (Addr: 0xaf8, Size: 0xd0)
  - .rela.dyn (Addr: 0xbc8, Size: 0x108)
  - .rela.plt (Addr: 0xcd0, Size: 0x1b0)
  - .init (Addr: 0x1000, Size: 0x1b)
  - .plt (Addr: 0x1020, Size: 0x130)
  - .plt.got (Addr: 0x1150, Size: 0x8)
  - .text (Addr: 0x1160, Size: 0xab9)
  - .fini (Addr: 0x1c1c, Size: 0xd)
  - .rodata (Addr: 0x2000, Size: 0x1ca)
  - .eh_frame_hdr (Addr: 0x21cc, Size: 0x4c)
  - .eh_frame (Addr: 0x2218, Size: 0x1dc)
  - .gcc_except_table (Addr: 0x23f4, Size: 0x8c)
  - .init_array (Addr: 0x3d90, Size: 0x8)
  - .fini_array (Addr: 0x3d98, Size: 0x8)
  - .dynamic (Addr: 0x3da0, Size: 0x210)
  - .got (Addr: 0x3fb0, Size: 0x38)
  - .got.plt (Addr: 0x3fe8, Size: 0xa8)
  - .data (Addr: 0x4090, Size: 0x18)
  - .bss (Addr: 0x40a8, Size: 0x8)
  - .comment (Addr: 0x0, Size: 0x52)
  - .symtab (Addr: 0x0, Size: 0x618)
  - .strtab (Addr: 0x0, Size: 0x6be)
  - .shstrtab (Addr: 0x0, Size: 0x12c)
```