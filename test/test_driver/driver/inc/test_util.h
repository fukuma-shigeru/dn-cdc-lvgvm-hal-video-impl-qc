
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <string>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <experimental/source_location>
#include "test_main.h"
#include "file_observer.h"

bool TestFileRead_bin(const char* p_path, void* p_value, size_t size);
bool TestFileWrite_bin(const char* p_path, const void* p_value, size_t size);

void PrintMsg(const std::string msg, const std::experimental::source_location);
void PrintMsg(const char* msg, const std::experimental::source_location);
void PrintMsgV(const std::experimental::source_location, const char* fmt, ...);

#ifndef NO_PRINTDBG
#define PrintDbg(...)	PrintMsgV(std::experimental::source_location::current(), __VA_ARGS__)
#else
  #define PrintDbg(...)	do{}while(false)
#endif

#endif /* TEST_UTIL_H */
