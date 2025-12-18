#include "flaw_demo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// 缺陷 1: Memory Leak (内存泄漏)
void memory_leak_demo() {
    char *ptr = (char *)malloc(50 * sizeof(char));
    if (ptr == NULL) return;
    strcpy(ptr, "This is a memory leak example.");
    free(ptr);
}

// 缺陷 2: Double Free (重复释放)
void double_free_demo() {
    int *ptr = (int *)malloc(sizeof(int));
    if (ptr == NULL) return;
    *ptr = 100;
    
    free(ptr); // 第一次释放
    ptr = NULL;
    
    // 逻辑错误导致再次释放
    if (ptr) free(ptr);
}

// 缺陷 3: Null Pointer Dereference (空指针解引用)
void null_pointer_demo() {
    int *ptr = NULL;
    
    // 模拟逻辑
    if (1 > 2) {
        ptr = (int *)malloc(sizeof(int));
    }

    if (!ptr) {
        // 保证后续使用安全
        ptr = (int *)malloc(sizeof(int));
        if (!ptr) return;
        *ptr = 0;
    }

    printf("Value: %d\n", *ptr);
    
    if (ptr) free(ptr);
}
