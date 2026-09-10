#ifndef BRIDGE_H
#define BRIDGE_H

#ifdef __cplusplus
    extern "C" {
        #endif
        const char* exec_math_magic(const char* raw_json);
        
        void free_mem(const char* ptr);
        #ifdef __cplusplus
    }
    #endif
#endif