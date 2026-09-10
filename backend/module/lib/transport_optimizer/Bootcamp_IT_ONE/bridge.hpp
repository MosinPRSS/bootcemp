#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const char* exec_math_magic(const char* raw_json);
void free_mem(char* ptr);

#ifdef __cplusplus
}
#endif