#include <stdio.h>
#include <stdlib.h>

int main() {
    size_t size = 1024L * 1024L * 1024L * 10L;  // 10 GB
    int *arr = (int *)malloc(size);

    if (arr == NULL) {
        printf("Error: Out of memory\n");
        return 1;
    }

    arr[0] = 123;
    printf("arr[0] = %d\n", arr[0]);

    free(arr);
    return 0;
}