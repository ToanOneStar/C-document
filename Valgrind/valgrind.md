# 2. Các lỗi bộ nhớ phổ biến
## 2.1. Invalid read/write of size X (Đọc/ghi không hợp lệ)

Xảy ra khi hương trình cố gắng đọc hoặc ghi X byte vào một vùng bộ nhớ không hợp lệ.4 Đây là một trong những lỗi phổ biến nhất và nghiêm trọng nhất.

Nguyên nhân phổ biến:

1. Truy cập vượt quá giới hạn khối heap được cấp phát (buffer overflow/overrun). Lỗi này thường là kết quả của lỗi "off-by-one" (lệch một đơn vị).
2. Truy cập bộ nhớ đã được giải phóng (use-after-free).
3. Truy cập vào vùng chưa được cấp phát (ví dụ: từ con trỏ chưa khởi tạo).
### 2.1.1. Truy cập vượt quá giới hạn khối heap được cấp phát

Ví dụ:
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    const char *src = "hello";
    
    // Lỗi chỉ cấp phát strlen(src) bytes, thiếu ký tự null ('\0')
    char *dest = (char *)malloc(strlen(src)); 
    
    strcpy(dest, src); // ➜ Gây lỗi ghi ngoài vùng nhớ

    printf("Copied string: %s\n", dest);
    
    free(dest);
    return 0;
}
```

Biên dịch và debug valgrind:
```bash
gcc -g -o exam main.c
valgrind ./exam
```
Debug với valgrind ta thu được:
![error2](../image/error2.png)
Ta thấy Valgrind chỉ rõ lỗi đọc ghi không hợp lệ vào 1 byte ở dòng 11 và 13. Và đưa ra chỉ 5 byte được cấp phát ở dòng 9.

**Cách giải quyết:** cấp thêm 1 byte cho '\0'.
### 2.1.2. Truy cập bộ nhớ đã được giải phóng (use-after-free)
Ví dụ trong đoạn code sau minh họa lỗi xảy ra khi truy cập bộ nhớ đã được giải phóng:
```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int *pointer = (int *)malloc(sizeof(int));
    if (pointer == NULL) {
        perror("malloc failed");
        return 1;
    }
    
    *pointer = 100;
    printf("Value before free: %d\n", *pointer);

    free(pointer); // Giải phóng bộ nhớ
    if (pointer == NULL) {
        printf("error\n");
    }

    // Cố gắng truy cập bộ nhớ đã giải phóng
    printf("Value after free (INVALID READ): %d\n", *pointer); // Lỗi Invalid read
    *pointer = 200; // Lỗi Invalid write
    printf("New value after free (INVALID WRITE): %d\n", *pointer);

    return 0;
}
```
Build chương trình và debug với Valgrind:
```bash
gcc -g -o exam main.c
valgrind ./exam
```
Chạy chương trình ta được kết quả:
![error1](../image/error1.png)

Debug với valgrind:
![error11](../image/error11.png)

Ta thấy Valgrind báo cáo rằng chương trình đã cố gắng đọc/ghi 4 byte vào một vùng bộ nhớ không hợp lệ. Lỗi ghi không hợp lệ ở dòng 21 và đọc không hợp lệ ở dòng 22. Và chỉ ra rõ rằng vùng nhớ đã được giải phóng ở dòng 14.

**Cách khắc phục:** Sau khi giải phóng bộ nhớ, hãy gán con trỏ về ```NULL``` để tránh việc vô tình sử dụng lại nó. Hàm ```free(NULL)``` an toàn và không gây lỗi.
### 2.1.3. Truy cập vào vùng chưa được cấp phát

Ví dụ về con trỏ chưa được khởi tạo.
```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int *uninitialized_ptr; // Con trỏ chưa được khởi tạo

    // Cố gắng ghi vào địa chỉ mà con trỏ chưa khởi tạo trỏ tới
    *uninitialized_ptr = 10; // Lỗi Invalid write

    printf("Value: %d\n", *uninitialized_ptr); // Lỗi Invalid read

    return 0;
}
```
Sau khi chạy chương trình bị lỗi ```Segmentation fault (core dumped)```

Debug với Valgrind:
![error3](../image/error3.png)

Valgrind chỉ ra lỗi không khởi tạo giá trị ở dòng 8 từ đó dẫn tới lỗi invalid write.

**Cách khắc phục:** Luôn khởi tạo con trỏ của bạn. Nếu bạn không biết nó sẽ trỏ đến đâu, hãy khởi tạo nó bằng ```NULL```. Nếu bạn muốn nó trỏ đến bộ nhớ hợp lệ, hãy cấp phát bộ nhớ cho nó bằng ```malloc``` hoặc gán nó cho một biến có địa chỉ hợp lệ.

## 2.2. Invalid free (giải phóng không hợp lệ)
Invalid free(): Chương trình cố gắng giải phóng một địa chỉ không phải từ heap, hoặc giải phóng cùng một khối bộ nhớ nhiều hơn một lần (double-free).

**Nguy hiểm của double-free:** Việc giải phóng bộ nhớ đã được giải phóng có thể làm hỏng cấu trúc quản lý heap của hệ thống, dẫn đến các sự cố không thể đoán trước, lỗi use-after-free hoặc lỗ hổng bảo mật nghiêm trọng.

Ví dụ:
```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    char* str = (char*)malloc(10);
    if (str == NULL) {
        return 1;
    }

    free(str); // hợp lệ
    free(str); // Lỗi: double free
    return 0;
}
```
Chạy chương trình ta gặp lỗi:
![error2-2](../image/error2-2.png)
Debug với Valgrind:
![er2-2](../image/er2-2.png)

Valgrind chỉ ra đã free ở dòng 10 và lỗi invalid free ở dòng 11.

**Giải pháp cho double-free:** Một cách hiệu quả để tránh lỗi này là gán con trỏ về ```NULL``` ngay sau khi giải phóng lần đầu tiên. Hàm ```free(NULL)``` không gây ra lỗi.
## 2.3. Bad permissions
Xảy ra khi chương trình cố gắng ghi vào một vùng bộ nhớ chỉ đọc, ví dụ như các chuỗi ký tự được khai báo tĩnh.

Ví dụ:
```c
#include <stdio.h>

int main() {
    char *str = "hello"; // chuỗi literal — nằm ở vùng chỉ đọc (read-only)
    str[0] = 'H';        // Lỗi: ghi vào vùng nhớ không được phép

    printf("%s\n", str);
    return 0;
}
```
Chạy chương trình trên ta gặp lỗi ```Segmentation fault (core dumped)```

Debug với Valgrind:
![er2-3](../image/er2-3.png)
Cách sửa đúng:
```c
char str[] = "hello";  // Chuỗi được copy vào vùng stack (có thể sửa)
str[0] = 'H';           // OK
```
# 3. Sử dụng Valgrind nâng cao
## 3.1. Tùy chỉnh Memcheck cho các trường hợp phức tạp
Memcheck là công cụ phổ biến nhất, nhưng nó cũng có nhiều tùy chọn nâng cao.
### 3.1.1. --track-origins=yes (Theo dõi nguồn gốc của giá trị chưa khởi tạo)

Đây là một trong những tùy chọn quan trọng nhất của ```Memcheck```. Khi bạn đọc một biến chưa được khởi tạo, Valgrind sẽ báo lỗi. Với ```--track-origins=yes```, nó không chỉ báo lỗi mà còn cho bạn biết giá trị đó đến từ đâu, giúp bạn truy tìm nguyên nhân gốc rễ dễ dàng hơn.

Ví dụ chương trình C gây lỗi đọc biến chưa được khởi tạo (uninitialized variable).
```c
#include <stdio.h>

int main() {
    int x;
    int y = x + 5;  // x chưa được khởi tạo, nhưng đã được sử dụng

    printf("y = %d\n", y);
    return 0;
}
```
Chạy valgrind không có ```--track-origins```
```bash
valgrind ./exam
```
Kết quả:
![er3-1-1](../image/er3-1-1.png)
 Valgrind báo lỗi sử dụng giá trị chưa khởi tạo, nhưng không rõ nó đến từ đâu.

Chạy lại với ```--track-origins=yes```:
```bash
valgrind --track-origins=yes ./exam
```
Kết quả đầy đủ thông tin hơn:
![er3-1-1-2](../image/er3-1-1-2.png)
 Valgrind bây giờ nói rõ ràng rằng giá trị chưa khởi tạo là do ```x``` được cấp phát trên stack ở dòng 4.