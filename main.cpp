#include <iostream>

int global_var_initialized = 42;
int global_var_uninitialized;
const char* msg = "Hello";
void func() { }
int main() {
    int local_var = 10;
    int* heap_var = new int(20);
    std::cout << msg << std::endl;
    delete heap_var;
    return 0;
}  