# Memory_Segments

## What are memory segments and what do they do for our programms 

Memory segments are distinct regions in a program's memory layout, each serving a specific purpose during execution. They are managed by the operating system and the compiler to organize a program's data and instructions efficiently.

1. Overview of Memory Segments
A program's memory is typically divided into the following segments:

## Text (Code) Segment:
Purpose: Stores the compiled machine code (instructions) of the program.
Access: Read-only, executable.
Typical Size: Depends on the program's complexity (e.g., a few KB to MB).
Mapping: Loaded from the executable file's text section into memory.

## Data Segment:
Purpose: Stores initialized global and static variables (e.g., int x = 10;).
Access: Read-write.
Typical Size: Proportional to the number of initialized variables (e.g., KB).
Mapping: Loaded from the executable's data section.

## BSS Segment:
Purpose: Stores uninitialized global and static variables (e.g., static int y;).
Access: Read-write.
Typical Size: Proportional to uninitialized variables (e.g., KB). Often zeroed out by the OS, so it doesn't occupy space in the executable file.
Mapping: Allocated at runtime, initialized to zero.

## Stack Segment:
Purpose: Stores local variables, function call frames, and return addresses.
Access: Read-write.
Typical Size: Fixed or dynamically adjustable (e.g., 1 MB default on Linux, adjustable via ulimit).
Mapping: Grows downward (in most architectures) and is managed by the OS.

## Heap Segment:
Purpose: Stores dynamically allocated memory (e.g., via malloc or new).
Access: Read-write.
Typical Size: Grows as needed, limited by system memory (e.g., MB to GB).
Mapping: Managed by the runtime library and OS, grows upward.

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
Optimization Impact: Higher optimization levels (-O2, -O3) inline functions, unroll loops, and remove redundant instructions, reducing code size in some cases. However, aggressive optimizations like loop unrolling can increase code size if more instructions are generated.
Example: -O0 (no optimization) includes debugging info and unoptimized code, inflating the text segment. -O3 may shrink it by removing dead code but could grow it with inlining.

# Data and BSS Segments:
Optimization Impact: Optimizations may reduce the number of global/static variables by promoting them to registers or eliminating unused variables, shrinking these segments.
Example: A variable marked static but unused may be removed with -O2, reducing BSS size.

# Stack Segment:
Optimization Impact: Optimizations reduce stack usage by minimizing local variables or reusing stack space. For example, -O2 may eliminate temporary variables, reducing stack frame size.
Example: A function with many local arrays may use less stack space with -O3 due to variable reuse.

# Heap Segment:
Optimization Impact: Indirectly affected if optimizations reduce dynamic memory allocations (e.g., by reusing memory or avoiding unnecessary allocations).
Example: A loop allocating memory repeatedly might be optimized to reuse a single allocation, reducing heap usage.


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

