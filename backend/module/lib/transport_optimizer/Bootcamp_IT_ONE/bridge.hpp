#pragma once

// у меня на винде все через длл
#ifdef _WIN32
    #define TO_API __declspec(dllexport)
#else
    #define TO_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// на винде тоже робит
TO_API const char* exec_math_magic(const char* raw_json);
TO_API void free_mem(char* ptr);

#ifdef __cplusplus
}
#endif