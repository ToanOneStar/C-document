- **Chương trình phải được nạp hoàn toàn vào bộ nhớ vật lý**: Nếu chương trình lớn hơn RAM, thì không thể chạy được.
Gây giới hạn kích thước chương trình.
- **Không gian địa chỉ bộ nhớ bị giới hạn và cố định**: Tất cả chương trình đều phải dùng địa chỉ vật lý trực tiếp, dẫn đến va chạm địa chỉ (address conflict) nếu nhiều chương trình cố truy cập cùng vùng nhớ.
- **Thiếu tính bảo vệ bộ nhớ (memory protection)**: Một tiến trình có thể truy cập vùng nhớ của tiến trình khác, dễ gây lỗi hoặc nguy hiểm bảo mật.
- **Không có khả năng phân mảnh hợp lý**: Quản lý bộ nhớ bị phân mảnh ngoài (external fragmentation) nghiêm trọng do phải cấp phát bộ nhớ liên tục.
- **Khó hỗ trợ đa nhiệm (multitasking) hiệu quả**: Vì phải nạp chương trình nguyên vẹn vào RAM và cấp phát bộ nhớ tĩnh, việc chạy đồng thời nhiều tiến trình sẽ tốn tài nguyên lớn và khó điều phối.

Chính vì vậy bộ nhớ ảo đã ra đời để khắc phục những vấn đề trên. Bộ nhớ ảo là kỹ thuật quản lý bộ nhớ nền tảng, tạo ảo ảnh về một bộ nhớ rất lớn cho người dùng và ứng dụng. Nó kết hợp RAM với bộ nhớ trên ổ đĩa (SSD/HDD) để tạo không gian địa chỉ liên tục. Hệ điều hành (OS), với sự hỗ trợ của phần cứng và phần mềm, ánh xạ địa chỉ ảo (chương trình sử dụng) thành địa chỉ vật lý (RAM thực tế).

Mục đích chính của bộ nhớ ảo không chỉ mở rộng dung lượng bộ nhớ vật lý bằng cách tận dụng không gian đĩa. Nó còn cung cấp bảo vệ bộ nhớ bằng cách dịch địa chỉ ảo sang vật lý, ngăn chặn truy cập trái phép. Điều này cho phép chạy các chương trình lớn hơn RAM, giải phóng ứng dụng khỏi quản lý bộ nhớ chia sẻ, tăng cường bảo mật qua cô lập bộ nhớ, và cho phép nhà phát triển sử dụng nhiều bộ nhớ hơn mức vật lý có sẵn.

![virtual](image/virtual.png)

Bộ nhớ ảo ra đời những năm 1960-1970 khi RAM đắt đỏ, giúp các hệ thống lớn chạy trên máy tính ít RAM, mang lại lợi ích kinh tế. Dù chi phí RAM giảm, bộ nhớ ảo vẫn thiết yếu nhờ các lợi ích phi chi phí như không gian địa chỉ ảo riêng biệt, tăng cường bảo mật và độ tin cậy, đặc biệt trong môi trường đa nhiệm. Nó đã trở thành thành phần kiến trúc cốt lõi của các hệ điều hành hiện đại, hỗ trợ bảo mật và hiệu suất cao.

Trước khi đi vào tìm hiểu chi tiết về bộ nhớ ảo chúng ta sẽ đi phân tích sâu hơn các vấn đề đã gặp phải trước khi bộ nhớ ảo ra đời như đã liệt kê phía trên.