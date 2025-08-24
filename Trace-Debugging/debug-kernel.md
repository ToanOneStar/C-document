## B1: tracer
### 1. nop tracer

Ý nghĩa: "No operation" – không thực hiện việc trace gì cả.

Đặc điểm:

- Là trạng thái mặc định khi bạn không muốn tracer nào hoạt động.

- Dùng để tắt tạm thời việc trace mà không xóa cấu hình.

Áp dụng khi:

- Muốn giữ lại cấu hình filter, option… nhưng tạm dừng thu log.

- Dùng như “reset” trước khi chuyển sang tracer khác.

### 2. function tracer

Ý nghĩa: Ghi lại mỗi lần một hàm trong kernel được gọi.

Đặc điểm:

- Cực kỳ chi tiết, có thể log hàng trăm nghìn sự kiện/giây.

- Có thể lọc chỉ trace một số hàm bằng file set_ftrace_filter.

- Ghi thời điểm vào buffer ftrace mỗi khi hàm được gọi.

Áp dụng khi:

- Muốn biết luồng thực thi (execution flow) trong kernel hoặc driver.

- Debug xem hàm nào được gọi, với thứ tự và tần suất thế nào.

Nhược điểm: Overhead cao nếu trace toàn bộ kernel, nên hay dùng filter.

### 3. function_graph tracer

Ý nghĩa: Ghi lại cả call graph (cấu trúc lời gọi hàm) kèm thời gian thực thi từng hàm.

Đặc điểm:

- Hiển thị hàm cha – hàm con theo dạng cây thụt lề.

- Có thể giới hạn độ sâu bằng max_graph_depth.

- Cho thấy thời gian (delta time) mà mỗi hàm chạy.

Áp dụng khi:

- Phân tích performance: xem hàm nào tốn thời gian nhất.

- Debug luồng gọi hàm phức tạp.

Nhược điểm: Nặng hơn function tracer vì phải đo thời gian và dựng cây lời gọi.

### 4. blk tracer

Ý nghĩa: Trace các sự kiện liên quan đến block layer I/O (đọc/ghi thiết bị block như HDD, SSD).

Đặc điểm:

- Ghi lại các event như request được gửi, hoàn tất, sắp xếp (merge)…

- Hữu ích để debug latency I/O, phân tích throughput.

Áp dụng khi:

- Điều tra vấn đề chậm disk I/O ở mức kernel.

- Debug driver block hoặc hệ thống lưu trữ.

### 5. mmiotrace tracer

Ý nghĩa: Trace các truy cập MMIO (Memory-Mapped I/O) trong kernel.

Đặc điểm:

- Log các đọc/ghi tới các vùng bộ nhớ ánh xạ phần cứng.

- Thường dùng để phân tích hành vi driver khi giao tiếp với phần cứng qua MMIO.

Áp dụng khi:

- Reverse-engineering driver (đặc biệt cho thiết bị chưa có driver open source).

- Debug các vấn đề liên quan đến thiết bị phần cứng (GPU, NIC…) dùng MMIO.

## B12: trace and trace_pipe
### 1. ```trace```

- Vị trí: ```/sys/kernel/debug/tracing/trace```

- Chức năng:

1. Lưu snapshot (bản chụp) của dữ liệu trace hiện tại.

2. Mỗi lần bạn đọc file này, nó sẽ trả về toàn bộ nội dung buffer hiện tại và giữ nguyên dữ liệu trong buffer.

3. Thích hợp khi bạn muốn đọc tĩnh để phân tích hoặc lưu log một lần.

4. Nếu dữ liệu trong buffer bị ghi đè (do vòng đệm – ring buffer), bạn sẽ mất thông tin cũ, nhưng việc đọc file này không xóa dữ liệu đang có.

### 2. ```trace_pipe```

- Vị trí: /sys/kernel/debug/tracing/trace_pipe

- Chức năng:

1. Hoạt động giống như ống pipe: dữ liệu được xuất ra liên tục khi có sự kiện.

2. Khi bạn đọc từ file này, dữ liệu sẽ bị xóa khỏi buffer ngay sau khi đọc.

3. Cho phép đọc dữ liệu real-time (thời gian thực) khi các event xảy ra.

4. Không cần phải lo dữ liệu cũ lẫn vào, nhưng nếu bạn đọc không kịp → sẽ mất event.

5. Thường được dùng trong các tình huống streaming log hoặc phân tích trực tiếp.
## B17:

```tracing_thresh``` dùng để lọc thời gian các function mặc định là 0

Ví dụ:
```bash
echo 10000 > tracing_thresh
```
Tức là khi ```cat trace``` sẽ chỉ có các hàm có duration lớn hơn 10ms được hiển thị.
## B18:
Chức năng của ```max_graph_depth```:

- Đây là tham số giới hạn độ sâu tối đa của cây lời gọi hàm (call graph) mà function graph tracer sẽ ghi lại.

- "Depth" (độ sâu) ở đây là số tầng hàm lồng nhau:

1. Depth = 0: chỉ hàm gốc (root function)

2. Depth = 1: hàm gốc + các hàm được gọi trực tiếp từ nó

3. Depth = 2: thêm một tầng hàm con nữa, v.v.

Mục đích: tránh việc trace quá sâu, gây buffer tràn, tốn CPU, và khó phân tích log.

Sử dụng:
```bash
# mặc định là 0
cat /sys/kernel/debug/tracing/max_graph_depth

# Giới hạn độ sâu trace thành 5
echo 5 > /sys/kernel/debug/tracing/max_graph_depth
```
## B19: irqs-off
### 1. IRQ và IRQs-off là gì?

- IRQ (Interrupt Request): tín hiệu từ phần cứng yêu cầu CPU tạm dừng công việc hiện tại để xử lý một sự kiện (ví dụ: dữ liệu từ bàn phím, network card, timer…).

- IRQs-off: trạng thái mà CPU tạm thời vô hiệu hóa việc nhận IRQ.
Điều này thường xảy ra khi kernel đang thực hiện các tác vụ quan trọng cần atomic context (không bị gián đoạn).

### 2. irqs-off tracer làm gì?

Khi bật irqs-off tracer, ftrace sẽ:

- Ghi lại call stack tại thời điểm IRQ bị tắt.

- Đo thời gian từ lúc IRQ bị tắt đến lúc được bật lại.

- Nếu thời gian này vượt ngưỡng cấu hình → ghi log để phát hiện đoạn code giữ IRQs-off quá lâu.

Mục đích:

- Tìm các đoạn code trong kernel giữ interrupts tắt lâu bất thường → dễ gây latency hoặc miss deadline trong hệ thống real-time.

- Giúp debug vấn đề về performance hoặc latency trong driver/hệ thống.

### 3. Cách bật irqs-off
```bash
# Bật ftrace
mount -t debugfs none /sys/kernel/debug

# Chọn tracer
echo irqs-off > /sys/kernel/debug/tracing/current_tracer

# Xem kết quả
cat /sys/kernel/debug/tracing/trace
```
### 4. Ví dụ log
```
# tracer: irqs-off
#
#           TASK-PID   CPU#     TIMESTAMP  FUNCTION
#              | |      |          |       |
     ksoftirqd/0-3     [000] d...  45.123: disable_irqs: duration=120 us
     ...
```

duration = thời gian CPU ở trạng thái IRQs-off.

## B22: preempt-depth
Sử dụng để tránh lỗi race condition khi truy cập vào tài nguyên chung. Nếu preempt_count > 0 thì CPU không thể thực hiện chuyển đổi ngữ cảnh sang task khác.

preempt_disable() thì count +1
preempt_enable() thì count -1

Khi count = 0 thì thực hiện chuyển đổi ngữ cảnh.

## B27: Stack dump

Function stack dump là hiển thị thứ tự gọi hàm theo chiều ngược từ kernel -> user space xem hàm nào đã gọi nó ngược với call graph trong function graph

Sử dụng
``` bash
echo 1 options/func_stack_trace
```
để hiển thị stack dump function.

Ứng dụng:

- Debug kernel module: Xem hàm của bạn được gọi trong ngữ cảnh nào.

- Phân tích performance: Tìm call path dài bất thường.

- Tìm bug: Khi 1 hàm chạy không mong muốn, dùng stacktrace để biết nó bị ai gọi.

## B30: Function profilling
Kernel function profiling là quá trình đo lường tần suất và thời gian chạy của các hàm trong kernel.

Mục tiêu: tìm ra hàm nào chạy nhiều nhất, tốn CPU nhất, hoặc gây nút thắt hiệu năng (bottleneck).

Ví dụ
```bash
echo function > current_tracer
echo 1 > function_profile_enable
cd trace_stat
cat function0
```

![profile](https://toanonestar.github.io/C-document/image/profile.png)

Trong đó:
- Hit → số lần hàm được gọi.
- Time (ns) → tổng thời gian CPU dành cho hàm đó.
- Avg (ns) → trung bình mỗi lần gọi mất bao nhiêu thời gian.

## B31:
### Khái niệm: Interrupts Disabled

Khi CPU disable interrupts (tắt ngắt), CPU không thể xử lý bất kỳ external event nào:

Timer interrupt (ngắt đồng hồ) → nếu bị chặn, kernel không nhận biết được “tick” thời gian.

Device interrupt (chuột, bàn phím, card mạng…) → sự kiện không được báo ngay cho kernel.

Hệ quả: gây latency (độ trễ) trong phản ứng của hệ thống.

### irqsoff tracer là gì?

irqsoff là một loại latency tracer trong ftrace.

Nó theo dõi tất cả các đoạn code trong kernel mà interrupts bị disable.

Cách hoạt động:

- Bắt đầu trace khi CPU gọi local_irq_disable() hoặc tương đương.
- Kết thúc trace khi interrupts được bật lại (local_irq_enable()).
- Đo thời gian interrupts bị disable trong khoảng đó.
- Nếu thời gian này lớn hơn giá trị maximum trước đó, nó: 
Ghi lại stack trace/hành trình gọi hàm đã dẫn đến độ trễ.
 Xoá trace cũ và thay bằng trace mới.

Nói cách khác: irqsoff giúp bạn tìm đoạn code tệ nhất (worst-case) gây ra “interrupt-off latency”.

### Tại sao cần dùng?

Kernel realtime (RT), hoặc hệ thống nhúng, cần đảm bảo phản hồi ngắt nhanh.

Nếu có chỗ trong kernel disable IRQ quá lâu → gây deadline miss hoặc jitter cao.

irqsoff giúp bạn phát hiện chỗ đó để tối ưu.

### Điều kiện biên dịch

Tracer này không mặc định bật trong kernel.

Bạn cần bật trong config:
```bash
CONFIG_IRQSOFF_TRACER=y
```
## B34:
### Vấn đề khi debug kernel

Khi kernel panic / oops → hệ thống dừng lại, bạn chỉ thấy backtrace tại thời điểm crash.

Nhưng thường nguyên nhân thật sự xảy ra từ trước đó (một function gọi sai, disable irq quá lâu, deadlock trước khi panic…).

Nếu chỉ nhìn stack ở thời điểm crash → nhiều khi không tìm ra nguồn gốc.

Do đó, ta muốn có ngữ cảnh trước khi crash.

### Ftrace giúp gì?

Ftrace ghi lại lịch sử function call, event, latency… ngay trong kernel.

Nếu cấu hình "ftrace_dump_on_oops" = 1, thì khi kernel panic, Ftrace buffer sẽ được dump thẳng vào console / dmesg cùng với oops message.

Nghĩa là:
Ngoài stacktrace ở chỗ crash, bạn còn có log "lịch sử chạy" của CPU/function → biết kernel đã làm gì ngay trước khi chết.

### Cách bật
```bash
# Bật dump ftrace log khi oops
echo 1 > /proc/sys/kernel/ftrace_dump_on_oops
```
## B35: Trace event - format file
Khi sử dụng Ftrace để theo dõi các sự kiện kernel, bạn sẽ thấy mỗi sự kiện (event) có một tệp `format` riêng để mô tả cấu trúc của dữ liệu được ghi lại. Đoạn code và văn bản bạn cung cấp chính là định dạng của sự kiện `sched_switch`.

Dưới đây là giải thích chi tiết về cấu trúc của tệp `format`:

---

### Cấu trúc chung của tệp `format`

Tệp `format` định nghĩa cách các trường dữ liệu được sắp xếp và lưu trữ trong bộ đệm vòng (ring buffer) của Ftrace. Mỗi trường có các thông tin sau:

* **`field-type`**: Kiểu dữ liệu của trường (ví dụ: `unsigned short`, `int`, `char`).
* **`field-name`**: Tên của trường (ví dụ: `common_type`, `prev_comm`).
* **`offset`**: Vị trí bắt đầu của trường tính từ đầu bản ghi sự kiện, tính bằng byte.
* **`size`**: Kích thước của trường, tính bằng byte.

Dựa vào `offset` và `size`, Ftrace có thể đọc và phân tích từng trường dữ liệu một cách chính xác.

---

### Phân tích từng trường trong `sched_switch`

Sự kiện `sched_switch` xảy ra khi kernel chuyển đổi từ một tiến trình này sang một tiến trình khác.  Định dạng của nó bao gồm hai phần chính: các trường **`common`** và các trường **riêng của sự kiện**.

#### 1. Các trường `common`

Đây là những trường có mặt trong hầu hết mọi sự kiện trace của kernel. Chúng cung cấp các thông tin chung về ngữ cảnh của sự kiện:

* **`field:unsigned short common_type;`**: Kiểu sự kiện. Giá trị này thường khớp với **ID** của sự kiện, trong trường hợp này là **323**.
* **`field:unsigned char common_flags;`**: Các cờ trạng thái, thường cho biết liệu ngắt (interrupts) có được bật hay tắt khi sự kiện xảy ra.
* **`field:unsigned char common_preempt_count;`**: Trạng thái **preemption** (khả năng bị chiếm quyền CPU). Nó cho biết liệu preemption có được bật hay tắt và liệu tiến trình đang chạy trong ngữ cảnh ngắt, tiến trình, hay NMI (Non-Maskable Interrupt).
* **`field:int common_pid;`**: ID của tiến trình (PID) đang thực hiện lệnh gọi hệ thống.

#### 2. Các trường riêng của `sched_switch`

Những trường này cung cấp thông tin chi tiết về hai tiến trình tham gia vào quá trình chuyển đổi.

* **`field:char prev_comm[16];`**: Tên của tiến trình vừa bị ngắt (previous process), được lưu dưới dạng một mảng ký tự có kích thước 16 byte.
* **`field:pid_t prev_pid;`**: ID của tiến trình vừa bị ngắt.
* **`field:int prev_prio;`**: Độ ưu tiên của tiến trình vừa bị ngắt.
* **`field:long prev_state;`**: Trạng thái của tiến trình vừa bị ngắt (ví dụ: đang chạy, đang ngủ, không thể ngắt).
* **`field:char next_comm[16];`**: Tên của tiến trình sắp được chạy (next process).
* **`field:pid_t next_pid;`**: ID của tiến trình sắp được chạy.
* **`field:int next_prio;`**: Độ ưu tiên của tiến trình sắp được chạy.

### Tóm lại

Tệp `format` là "bản thiết kế" cho dữ liệu sự kiện. Nó giúp các công cụ phân tích dữ liệu như Ftrace và `trace-cmd` hiểu cách đọc và hiển thị các bản ghi sự kiện một cách chính xác, từ đó giúp các nhà phát triển kernel dễ dàng gỡ lỗi và phân tích hành vi của hệ thống.

## B36: Trace event - filter file
Filtering Events (lọc sự kiện) trong Ftrace là một cơ chế mạnh mẽ cho phép bạn chỉ ghi lại những sự kiện đáp ứng các tiêu chí cụ thể. Thay vì ghi lại tất cả mọi thứ, bạn có thể "lọc" bớt các sự kiện không cần thiết, giúp giảm dung lượng file trace và làm cho việc phân tích dữ liệu dễ dàng hơn.


### Cách thức hoạt động

Quá trình lọc sự kiện diễn ra ngay lập tức. Khi một sự kiện được ghi vào bộ đệm trace (trace buffer), các trường dữ liệu của nó (ví dụ: `pid`, `comm`, `prio`) sẽ được so sánh với biểu thức lọc mà bạn đã đặt.

  * **Nếu các giá trị trường khớp với biểu thức lọc**, sự kiện đó sẽ được lưu lại và xuất hiện trong output của trace.
  * **Nếu các giá trị không khớp**, sự kiện đó sẽ bị loại bỏ.
  * **Nếu không có bộ lọc nào được thiết lập**, tất cả các sự kiện sẽ được ghi lại theo mặc định.

### Thiết lập bộ lọc

Bạn thiết lập bộ lọc bằng cách ghi một biểu thức lọc vào tệp **`filter`** của sự kiện đó.

Ví dụ: Để lọc sự kiện `sched_switch`, bạn sẽ ghi biểu thức lọc vào tệp `events/sched/sched_switch/filter`.

### Biểu thức lọc (Filter Expressions)

Một biểu thức lọc bao gồm một hoặc nhiều **mệnh đề so sánh (predicates)**, được kết hợp bằng các toán tử logic **`&&` (AND)** hoặc **`||` (OR)**.

#### Mệnh đề so sánh (Predicate)

Một mệnh đề so sánh là một điều kiện kiểm tra giá trị của một trường dữ liệu so với một hằng số.

  * **Cú pháp**: `field-name relational-operator value`

      * **`field-name`**: Tên của trường dữ liệu, tìm thấy trong tệp `format` của sự kiện.
      * **`relational-operator`**: Toán tử so sánh.
      * **`value`**: Giá trị hằng số để so sánh.

Bạn có thể sử dụng dấu ngoặc đơn `()` để nhóm các mệnh đề lại với nhau và dấu ngoặc kép `""` để tránh các ký tự đặc biệt của shell.

#### Các loại toán tử so sánh

Các toán tử này thay đổi tùy thuộc vào kiểu dữ liệu của trường bạn đang so sánh:

  * **Trường số (Numeric Fields)**:
      * `==` (bằng), `!=` (không bằng)
      * `<` (nhỏ hơn), `<=` (nhỏ hơn hoặc bằng)
      * `>` (lớn hơn), `>=` (lớn hơn hoặc bằng)
      * `&` (phép toán AND bitwise)
  * **Trường chuỗi (String Fields)**:
      * `==` (bằng), `!=` (không bằng)
      * `~` (so khớp chuỗi theo mẫu - globbing), hỗ trợ các ký tự đại diện như `*` (đại diện cho một chuỗi bất kỳ), `?` (đại diện cho một ký tự bất kỳ) và `[]` (lớp ký tự).


### Ví dụ thực tế

Giả sử bạn chỉ muốn xem các sự kiện `sched_switch` khi tiến trình **`prev_comm`** là `"my_app"`.

Bạn có thể đặt bộ lọc như sau:

```bash
echo 'prev_comm == "my_app"' > /sys/kernel/debug/tracing/events/sched/sched_switch/filter
```

Hoặc nếu bạn muốn xem các sự kiện khi `prev_pid` là 1234 và `next_pid` không phải là 5678, bạn sẽ viết:

```bash
echo 'prev_pid == 1234 && next_pid != 5678' > /sys/kernel/debug/tracing/events/sched/sched_switch/filter
```

Sử dụng bộ lọc giúp bạn thu hẹp phạm vi trace, chỉ tập trung vào những thông tin bạn quan tâm, làm cho quá trình gỡ lỗi và phân tích hệ thống hiệu quả hơn rất nhiều.

Trong ngữ cảnh của Ftrace, trường `prev_state` trong sự kiện `sched_switch` cho biết trạng thái của tiến trình trước khi nó bị chuyển đổi (hoặc bị ngắt).

Giá trị của `prev_state` tương ứng với các trạng thái của một tiến trình trong Linux, bao gồm:

* **R (TASK_RUNNING)**: Tiến trình đang ở trạng thái **"có thể chạy"**. Nó đang chờ để được lên lịch chạy trên CPU, hoặc thực sự đang chạy trên một CPU khác. Đây là trạng thái thường thấy nhất cho tiến trình bị chuyển đổi, đặc biệt là trong các trường hợp chia sẻ thời gian (time-sharing).
* **S (TASK_INTERRUPTIBLE)**: Tiến trình đang ở trạng thái **ngủ có thể bị ngắt**. Nó đang chờ một sự kiện nào đó (như hoàn thành I/O, một tín hiệu, hoặc một timer) và có thể bị đánh thức bởi một tín hiệu.
* **D (TASK_UNINTERRUPTIBLE)**: Tiến trình đang ở trạng thái **ngủ không thể bị ngắt**. Tương tự như trạng thái `S`, nhưng tiến trình này không thể bị đánh thức bởi một tín hiệu. Trạng thái này thường được dùng khi tiến trình đang chờ một hoạt động I/O cấp thấp và việc bị ngắt giữa chừng có thể gây ra lỗi.
* **T (TASK_STOPPED)**: Tiến trình đã bị **dừng lại** (stopped). Nó không thể chạy cho đến khi nhận được tín hiệu `SIGCONT` để tiếp tục.
* **t (TASK_TRACED)**: Tiến trình bị **dừng lại** do đang được gỡ lỗi bởi một tiến trình khác (dùng `ptrace`).
* **Z (EXIT_ZOMBIE)**: Tiến trình đã **kết thúc** nhưng chưa được tiến trình cha "thu hoạch" (wait-ed). Toàn bộ tài nguyên đã được giải phóng ngoại trừ cấu trúc `task_struct` nhỏ gọn để giữ thông tin kết thúc.
* **X (TASK_DEAD)**: Tiến trình đã **chết** và không thể chạy nữa.

Trong tệp `format` của Ftrace, các giá trị này thường được biểu diễn dưới dạng số (ví dụ: `0` cho `TASK_RUNNING`, `1` cho `TASK_INTERRUPTIBLE`, v.v.). Tuy nhiên, khi Ftrace hiển thị kết quả ra ngoài, nó sẽ chuyển đổi các giá trị số này thành các ký tự chữ cái tương ứng (`R`, `S`, `D`, v.v.) để dễ đọc hơn cho người dùng.

Hiểu các giá trị này rất quan trọng để phân tích hành vi của hệ thống. Ví dụ, nếu bạn thấy một tiến trình liên tục bị chuyển đổi khi `prev_state` của nó là `D` (uninterruptible sleep), điều đó có thể chỉ ra một vấn đề về I/O hoặc tài nguyên, khi tiến trình đó đang chờ một hoạt động quan trọng nào đó nhưng không thể bị ngắt.
## B42: Subsystem filters
Bộ lọc con hệ thống (subsystem filters) là một tính năng hữu ích của Ftrace cho phép bạn áp dụng một bộ lọc cho tất cả các sự kiện trong một nhóm (subsystem) cùng một lúc.

### Cách thức hoạt động

Thay vì phải đi vào từng thư mục của sự kiện (ví dụ: `events/sched/sched_switch`) và đặt bộ lọc riêng lẻ, bạn có thể đặt một bộ lọc chung cho cả subsystem bằng cách ghi biểu thức lọc vào tệp `filter` ở thư mục gốc của subsystem đó.

Ví dụ, để lọc tất cả các sự kiện trong subsystem `sched`, bạn sẽ ghi biểu thức vào tệp `/sys/kernel/debug/tracing/events/sched/filter`. Khi đó, bộ lọc này sẽ được áp dụng cho mọi sự kiện con của nó, chẳng hạn như `sched_switch`, `sched_wakeup`, `sched_stat_wait`, v.v.

### Lưu ý quan trọng

Việc áp dụng bộ lọc chung này có một số quy tắc cần lưu ý để tránh các lỗi không mong muốn:

1.  **Bộ lọc riêng lẻ vẫn được giữ nguyên:** Nếu một sự kiện trong subsystem đã có một bộ lọc riêng được thiết lập trước đó, bộ lọc riêng đó sẽ được giữ lại. Việc đặt bộ lọc chung chỉ ảnh hưởng đến các sự kiện chưa có bộ lọc.

2.  **Lỗi khi không tìm thấy trường:** Nếu biểu thức lọc chung tham chiếu đến một trường dữ liệu **không có** trong một sự kiện con nào đó, bộ lọc sẽ không được áp dụng cho sự kiện đó.

    * **Ví dụ:** Giả sử bạn đặt bộ lọc chung cho subsystem `signal` là `sig == 9`. Bộ lọc này sẽ được áp dụng cho `signal_generate` (vì có trường `sig`) nhưng sẽ **không** được áp dụng cho một sự kiện khác trong cùng subsystem nếu sự kiện đó không có trường `sig`.

3.  **Lỗi cú pháp hoặc lỗi khác:** Nếu bộ lọc không thể được áp dụng vì bất kỳ lý do nào khác (ví dụ: lỗi cú pháp), nó sẽ không có tác dụng với sự kiện đó.

4.  **Các trường `common` luôn hoạt động:** Các trường chung (như `common_pid`, `common_type`) có mặt trong hầu hết mọi sự kiện. Do đó, các bộ lọc chỉ sử dụng các trường `common` luôn đảm bảo sẽ được áp dụng thành công cho tất cả các sự kiện trong một subsystem. Đây là cách an toàn và hiệu quả nhất để lọc đồng loạt.

Tóm lại, **subsystem filters** giúp bạn tiết kiệm thời gian bằng cách áp dụng bộ lọc hàng loạt. Tuy nhiên, bạn cần hiểu rõ rằng bộ lọc này chỉ được áp dụng nếu cú pháp hợp lệ và các trường được tham chiếu tồn tại trong sự kiện con.
## B49: Trace mkdir
Ví dụ:
```bash
cat available_events | grep -i mkdir
echo nop > current_tracer
echo sys_enter_mkdir > set_event
echo sys_enter_mkdirat >> set_event
cat set_event
mkdir /tmp/lwl1
cat trace_pipe
```
Sau khi tạo folder và ```trace_pipe``` ta được kết quả:
```
mkdir-4357    [000] ...1.  1209.627126: sys_mkdir(pathname: 7ffd472de3dc, mode: 1ff)
```
Khi bạn tạo một thư mục bằng lệnh `mkdir` và theo dõi bằng `cat trace_pipe`, dòng thông tin bạn thấy là một bản ghi chi tiết về sự kiện gọi hàm hệ thống `sys_enter_mkdir`. Dòng này cung cấp các thông tin quan trọng về tiến trình, CPU, thời gian và các tham số của lệnh đã thực thi.

Chúng ta sẽ phân tích từng phần của dòng output này:

`mkdir-4357`
* `mkdir`: Tên của tiến trình đã thực hiện lệnh gọi hệ thống.
* `4357`: **Process ID (PID)** của tiến trình `mkdir`.

`[000]`
* Đây là số hiệu của **CPU** nơi tiến trình này đang chạy. Trong trường hợp này là CPU số `0`.

`...1.`
* Các cờ trạng thái (flags) của tiến trình.
    * `...`: Các cờ chưa được sử dụng hoặc không liên quan.
    * `1`: Cờ này cho biết preempt (khả năng bị chiếm quyền CPU) đã bị vô hiệu hóa (disabled) tại thời điểm sự kiện xảy ra. Điều này là bình thường vì các lệnh gọi hệ thống thường vô hiệu hóa preempt để đảm bảo tính toàn vẹn.
    * `.`: Cờ này cho biết tiến trình không chạy trong ngữ cảnh ngắt.

`1209.627126`
* Đây là **thời gian** tính bằng giây kể từ khi hệ thống được khởi động.

`sys_mkdir(pathname: 7ffd472de3dc, mode: 1ff)`
* `sys_mkdir`: Tên của hàm hệ thống đang được gọi. Đây là sự kiện `sys_enter_mkdir` mà chúng ta đã thảo luận trước đó.
* `pathname: 7ffd472de3dc`: Đây là tham số đầu tiên của hàm `mkdir`, là **địa chỉ bộ nhớ** (memory address) nơi chuỗi đường dẫn của thư mục mới được lưu trữ.
* `mode: 1ff`: Đây là tham số thứ hai, đại diện cho **quyền truy cập** (permissions) của thư mục mới. Giá trị `1ff` (trong hệ thập lục phân) tương đương với `777` trong hệ bát phân, có nghĩa là "cho phép tất cả người dùng (chủ sở hữu, nhóm, và người khác) được đọc, ghi và thực thi".

Sau đó nếu chúng ta tiếp tục tạo folder tương tự thì sẽ bị lỗi:
```
mkdir: cannot create directory ‘/tmp/lwl3’: File exists
```

`cat trace_pipe` sẽ có log như sau:
```
mkdir-4387    [000] ...1.  1485.169940: sys_mkdir -> 0xffffffffffffffef
```
Giải thích:
`sys_mkdir -> 0xffffffffffffffef`

- `sys_mkdir`: Tên của hàm hệ thống. Mũi tên -> cho biết đây là sự kiện kết thúc (sys_exit_mkdir).

- `0xffffffffffffffef`: Đây là giá trị trả về của hàm mkdir().

Trong hệ thống Unix/Linux, các hàm hệ thống thường trả về 0 khi thành công và một số âm (negative value) khi gặp lỗi.

Giá trị `0xffffffffffffffef` trong hệ thập lục phân tương ứng với `-17` trong hệ thập phân. Mã lỗi `-17` này chính là giá trị của `EEXIST` (error number 17), có nghĩa là "File exists" (tệp đã tồn tại).

Có thể kiểm tra mã lỗi là lỗi gì bằng:
```bash
perror 17
```

### Các mã lỗi liên quan đến `mkdir`

1.  `EACCES` (Permission denied)
    * **Mã lỗi:** 13
    * **Lý do:** Bạn không có quyền ghi (write permission) vào thư mục cha mà bạn muốn tạo thư mục mới bên trong.
    * **Ví dụ:** Bạn đang ở thư mục `/root` và cố gắng tạo một thư mục mới mà không phải là người dùng `root`. 

2.  `ENOENT` (No such file or directory)
    * **Mã lỗi:** 2
    * **Lý do:** Một phần của đường dẫn mà bạn cung cấp không tồn tại.
    * **Ví dụ:** Bạn cố gắng tạo thư mục `/home/user/new_folder/sub_folder`, nhưng thư mục `/home/user/new_folder` chưa tồn tại.

3.  `EFAULT` (Bad address)
    * **Mã lỗi:** 14
    * **Lý do:** Địa chỉ bộ nhớ của đường dẫn thư mục không hợp lệ, thường xảy ra trong lập trình khi con trỏ chuỗi đường dẫn bị lỗi.
    * **Ví dụ:** Một lập trình viên truyền một con trỏ NULL hoặc một địa chỉ không hợp lệ vào hàm `mkdir()` trong C.

4.  **`ENOSPC` (No space left on device)**
    * **Mã lỗi:** 28
    * **Lý do:** Ổ đĩa đã đầy và không còn chỗ trống để tạo thư mục mới.
    * **Ví dụ:** Bạn cố gắng tạo một thư mục trên một phân vùng đã hết dung lượng.

5.  `EROFS` (Read-only file system)
    * **Mã lỗi:** 30
    * **Lý do:** Bạn đang cố gắng tạo thư mục trên một hệ thống tệp chỉ có quyền đọc (read-only), chẳng hạn như một USB được gắn ở chế độ chỉ đọc.
    * **Ví dụ:** Hệ thống tệp `/` bị gắn ở chế độ chỉ đọc do lỗi hệ thống, và bạn cố gắng tạo một thư mục mới.

Hiểu các mã lỗi này giúp bạn không chỉ biết lệnh `mkdir` thất bại mà còn hiểu **tại sao** nó thất bại, từ đó giúp việc gỡ lỗi nhanh hơn nhiều.
## B50: Trace USB
```bash
echo 1 > events/xhci-hcd/enable
cat set_event
cat trace_pipe
```
## B51: Trace Page fault
Sự kiện **page fault** (lỗi trang) xảy ra khi một chương trình cố gắng truy cập vào một vùng bộ nhớ ảo (virtual memory) nhưng vùng đó lại không có sẵn trong bộ nhớ vật lý (RAM) tại thời điểm đó. Đây là một cơ chế quan trọng của hệ điều hành để quản lý bộ nhớ, không hẳn là một lỗi nghiêm trọng.

```bash
echo 1 > events/exceptions/enable 
```

### Cách thức hoạt động của Page Fault

1.  **Chương trình yêu cầu bộ nhớ:** Một chương trình yêu cầu truy cập một trang bộ nhớ.
2.  **Địa chỉ không có trong RAM:** CPU nhận ra rằng địa chỉ bộ nhớ ảo này không tương ứng với một địa chỉ vật lý nào trong bảng trang hiện tại.
3.  **CPU gây ra một exception:** CPU tạo ra một exception (ngoại lệ), tạm dừng việc thực thi của chương trình và chuyển quyền điều khiển cho kernel.
4.  **Kernel xử lý:** Kernel nhận exception này và tìm trang bộ nhớ cần thiết, thường là từ bộ nhớ swap trên ổ đĩa.
5.  **Nạp trang vào RAM:** Kernel nạp trang bộ nhớ đó từ ổ đĩa vào một khung trang (page frame) trống trong RAM.
6.  **Cập nhật bảng trang:** Kernel cập nhật bảng trang để ánh xạ địa chỉ ảo đến địa chỉ vật lý mới.
7.  **Tiếp tục thực thi:** Kernel trả quyền điều khiển về cho chương trình, và chương trình có thể tiếp tục thực thi như bình thường.

### Các sự kiện Ftrace

Hai sự kiện `page_fault_user` và `page_fault_kernel` cho phép bạn theo dõi page fault xảy ra trong không gian người dùng và không gian kernel.

* **`page_fault_user`**: Sự kiện này được kích hoạt khi một chương trình **người dùng** (user space) gây ra lỗi trang. Đây là trường hợp phổ biến nhất, ví dụ khi một chương trình truy cập một biến hoặc một phần của mã code mà kernel chưa nạp vào RAM. 
* **`page_fault_kernel`**: Sự kiện này xảy ra khi chính **kernel** gây ra lỗi trang. Điều này ít phổ biến hơn và thường chỉ xảy ra khi kernel cần truy cập vào một trang bộ nhớ không có trong RAM.

Cả hai sự kiện này đều được gọi từ hàm **`do_page_fault`** trong `arch/x86/mm/fault.c` của kernel. Bằng cách theo dõi các sự kiện này, bạn có thể phân tích tần suất page fault xảy ra, từ đó đánh giá hiệu suất của hệ thống và xác định các vấn đề về quản lý bộ nhớ.

## B52: Trace module
Khi nói đến các sự kiện trong thư mục `events/module`, chúng ta đang đề cập đến việc theo dõi cách Linux kernel quản lý các module. Các module là những đoạn mã có thể được nạp và gỡ bỏ khi kernel đang chạy, giúp mở rộng chức năng của kernel mà không cần khởi động lại.

```bash
echo 1 > events/module/enable 
```

### Cơ chế quản lý module

Một cơ chế quan trọng là **đếm tham chiếu (reference-counted)**. Kernel sử dụng một bộ đếm để theo dõi xem một module đang được sử dụng bởi bao nhiêu thành phần khác.
* Lệnh `rmmod` (gỡ bỏ module) sẽ thất bại nếu bộ đếm tham chiếu không về 0, để tránh gỡ bỏ một module đang được sử dụng.

---

### Các sự kiện Ftrace chính

Dưới đây là các sự kiện quan trọng trong `events/module` giúp bạn theo dõi vòng đời của một module:

#### 1. Tăng/Giảm bộ đếm tham chiếu

* **`module_get`**:
    * Sự kiện này được gọi từ hàm **`try_module_get()`**.
    * Nó kích hoạt khi một thành phần nào đó của kernel yêu cầu sử dụng một module, làm **tăng** bộ đếm tham chiếu của module đó lên 1.
* **`module_put`**:
    * Sự kiện này được gọi từ hàm **`module_put()`**.
    * Nó kích hoạt khi một thành phần không còn sử dụng module nữa, làm **giảm** bộ đếm tham chiếu của module đó xuống 1.

#### 2. Nạp/Gỡ bỏ module

* **`module_load`**:
    * Được gọi từ hàm **`load_module()`**.
    * Sự kiện này xảy ra khi một module được nạp vào kernel bằng các lệnh gọi hệ thống như **`init_module`** hoặc **`finit_module`**.
* **`module_free`**:
    * Được gọi từ hàm **`free_module()`**.
    * Sự kiện này xảy ra khi một module được gỡ bỏ khỏi kernel, thường là thông qua lệnh gọi hệ thống **`delete_module`**.

#### 3. Yêu cầu nạp module từ kernel

* **`module_request`**:
    * Dùng để load module từ kernel space.
    * Được gọi từ hàm **`__request_module()`**.
    * Sự kiện này kích hoạt khi chính **kernel** cần một chức năng từ một module và tự động yêu cầu nạp module đó. Điều này xảy ra khi một driver hoặc một tính năng được yêu cầu, nhưng code của nó chưa được nạp vào kernel.

## Trace cmd
Việc theo dõi các sự kiện này giúp bạn hiểu rõ cách kernel quản lý các module, xác định các lỗi liên quan đến việc nạp/gỡ bỏ, hoặc phân tích lý do tại sao một module không thể được gỡ bỏ.

`trace-cmd` là một công cụ dòng lệnh mạnh mẽ giúp đơn giản hóa việc sử dụng `ftrace` và `tracefs`. Thay vì phải tương tác trực tiếp với các tệp tin trong `tracefs` bằng các lệnh `echo` và `cat`, `trace-cmd` cung cấp một giao diện thân thiện và dễ nhớ hơn.

### Lý do cần `trace-cmd`

* **Đơn giản hóa việc sử dụng**: `ftrace` và `tracefs` được thiết kế để hoạt động trên các hệ thống hạn chế tài nguyên như BusyBox, nơi chỉ có các lệnh cơ bản. Tuy nhiên, việc ghi nhớ tất cả các tệp tin và đường dẫn để bật/tắt trace có thể rất phức tạp và tốn thời gian.
* **Tự động hóa**: `trace-cmd` tự động xử lý việc gắn kết (mount) `tracefs` và thực hiện các tác vụ cơ bản, giúp người dùng không cần phải lo lắng về cấu trúc thư mục của `ftrace`.
* **Giao diện thống nhất**: Nó cung cấp một bộ lệnh thống nhất để điều khiển `ftrace`, giúp quá trình gỡ lỗi và phân tích trở nên hiệu quả hơn.

### Cách sử dụng

Bạn cần cài đặt `trace-cmd` và chạy nó với quyền root.

* **Cài đặt**: `sudo apt install trace-cmd`
* **Tài liệu**: Lệnh `man trace-cmd` sẽ hiển thị tài liệu hướng dẫn chi tiết về cách sử dụng.
* **Hoàn thành lệnh (bash completion)**: `trace-cmd` hỗ trợ tính năng hoàn thành lệnh, giúp bạn dễ dàng gõ các tùy chọn và tên sự kiện.

### Ví dụ tương đương

| Thao tác Ftrace với `echo`/`cat`                                                              | Thao tác với `trace-cmd`                                           |
| --------------------------------------------------------------------------------------------- | ------------------------------------------------------------------ |
| `echo 1 > /sys/kernel/debug/tracing/tracing_on`                                               | `trace-cmd start`                                                  |
| `echo 0 > /sys/kernel/debug/tracing/tracing_on`                                               | `trace-cmd stop`                                                   |
| `echo 1 > /sys/kernel/debug/tracing/events/sched/sched_switch/enable`                         | `trace-cmd record -e sched_switch`                                 |
| `cat /sys/kernel/debug/tracing/trace`                                                         | `trace-cmd report`                                                 |
| `cat /sys/kernel/debug/tracing/events/sched/sched_switch/format`                              | `trace-cmd list -e sched_switch`                                   |
| `echo > /sys/kernel/debug/tracing/events/sched/sched_switch/filter`                           | `trace-cmd record -e sched_switch --filter 'prev_pid==123'`        |

***
Dưới đây là giải thích chi tiết về các lệnh của `trace-cmd` để liệt kê các `tracers` có sẵn và các tính năng liên quan.

### Liệt kê các `tracers` có sẵn

Các `tracers` là các công cụ theo dõi trong nhân (kernel) được sử dụng để theo dõi các loại sự kiện khác nhau.
* Lệnh `trace-cmd list -t` tương đương với `cat /sys/kernel/debug/tracing/available_tracers`, liệt kê tất cả các `tracers` mà hệ thống hỗ trợ.

### Liệt kê các sự kiện

* Lệnh `trace-cmd list -e` hiển thị danh sách tất cả các sự kiện (events) `ftrace` có sẵn trên hệ thống của bạn. Các sự kiện này được sắp xếp theo các nhóm (subsystems), ví dụ như `sched` (lập lịch), `ext4` (hệ thống tệp tin), `net` (mạng), v.v.
* Lệnh `strace -e open,openat -o strace_output.txt trace-cmd list -e` sử dụng `strace` để theo dõi các lệnh gọi hệ thống `open` và `openat` mà `trace-cmd` thực hiện. Khi bạn chạy lệnh này, `trace-cmd` sẽ đọc các tệp trong `/sys/kernel/debug/tracing/events/` để lấy danh sách sự kiện, và `strace` sẽ ghi lại những lệnh gọi tệp này vào `strace_output.txt`.

### Liệt kê các tùy chọn

* Lệnh `trace-cmd list -o` liệt kê các tùy chọn `ftrace` có sẵn. Các tùy chọn này cho phép bạn tùy chỉnh hành vi của `ftrace`, ví dụ như bật/tắt `function tracing`, `irq-info`, v.v.
* Lệnh `trace-cmd list -f` liệt kê các hàm lọc (filter functions) có sẵn. Các hàm này được sử dụng để tạo các biểu thức lọc phức tạp, ví dụ như lọc các sự kiện dựa trên tên tiến trình hoặc các giá trị cụ thể.

### Biểu thức chính quy (Regular Expressions)

Bạn có thể sử dụng các biểu thức chính quy để lọc kết quả liệt kê.
* `trace-cmd list -e '^sig*'` liệt kê tất cả các sự kiện có tên bắt đầu bằng `sig` (ví dụ: `signal_generate`, `signal_deliver`).
* `trace-cmd list -e '^sys*'` liệt kê tất cả các sự kiện có tên bắt đầu bằng `sys` (ví dụ: `sys_enter_read`, `sys_exit_write`).
* `trace-cmd list -f '^sched*'` liệt kê các hàm lọc có tên bắt đầu bằng `sched`.

### Hiển thị thông tin chi tiết về sự kiện

`trace-cmd` cung cấp các tùy chọn để xem thông tin chi tiết của một sự kiện mà không cần phải duyệt các tệp `format` hay `trigger` trong `tracefs`.
* `trace-cmd list -e '^sig*' -F`: Hiển thị **định dạng (format)** của tất cả các sự kiện bắt đầu bằng `sig`. Điều này tương đương với việc `cat events/signal/signal_generate/format`.
* `trace-cmd list -e '^sig*' -l`: Hiển thị **bộ lọc (filter)** hiện tại của các sự kiện bắt đầu bằng `sig`.
* `trace-cmd list -e '^sig*' -R`: Hiển thị các **trigger** đã được thiết lập cho các sự kiện bắt đầu bằng `sig`.


### Các thông tin được hiển thị
`trace-cmd stat` là một lệnh quan trọng trong `trace-cmd` được dùng để hiển thị trạng thái hiện tại của hệ thống `ftrace`. Nó giúp bạn kiểm tra nhanh những gì đang được theo dõi mà không cần duyệt qua các file trong `tracefs`.

`trace-cmd stat` cung cấp một cái nhìn tổng quan về cấu hình `ftrace` hiện tại, bao gồm:

* **Tracer**: Cho biết `tracer` nào đang hoạt động. Nếu bạn đã bật một `tracer` đặc biệt (ví dụ: `function_graph` hoặc `nop`), tên của nó sẽ được hiển thị ở đây.
* **Events**: Liệt kê các sự kiện trace (events) đang được kích hoạt. Ví dụ, nếu bạn đã bật sự kiện `sched_switch`, nó sẽ xuất hiện trong danh sách này.
* **Event filters**: Hiển thị các bộ lọc (filters) mà bạn đã áp dụng cho bất kỳ sự kiện nào. Điều này giúp bạn xác nhận xem bộ lọc đã được đặt đúng cách chưa.
* **Function filters**: Cho thấy các bộ lọc được áp dụng cho `function tracer`. Nó giúp bạn theo dõi chỉ những hàm cụ thể, thay vì tất cả các hàm.
* **Graph functions**: Khi sử dụng `function_graph tracer`, tùy chọn này hiển thị danh sách các hàm mà `tracer` sẽ vẽ biểu đồ. 
* **`...`**: Ngoài ra, `trace-cmd stat` còn hiển thị các thông tin khác như kích thước bộ đệm trace, trạng thái của trace (`tracing_on` hay `off`), v.v.

`man trace-cmd-stat` cung cấp tài liệu chi tiết về tất cả các tùy chọn và thông tin mà lệnh này có thể hiển thị.

### Setting tracer
`trace-cmd start` và `trace-cmd stop` là hai lệnh cơ bản nhất của `trace-cmd`, được dùng để điều khiển việc ghi trace vào bộ đệm vòng (ring buffer).

`trace-cmd start` lệnh này dùng để **kích hoạt** một `tracer` hoặc một sự kiện cụ thể.

* **Triggers (`-p`)**: Bạn có thể chọn một trong các `tracers` có sẵn bằng tùy chọn `-p` (`plugins`).
    * `trace-cmd start -p function`: Bật **function tracer**, ghi lại mọi lệnh gọi hàm trong kernel.
    * `trace-cmd start -p function_graph`: Bật **function_graph tracer**, ghi lại lệnh gọi hàm và vẽ biểu đồ luồng thực thi của chúng. 
    * `trace-cmd start -p nop`: Bật **nop tracer**, một `tracer` "không làm gì cả". Thường được dùng để vô hiệu hóa một `tracer` đang hoạt động.

* **Events (`-e`)**: Bạn có thể bật các sự kiện cụ thể thay vì toàn bộ `tracer` bằng tùy chọn `-e`.
    * Bạn có thể chỉ định sự kiện theo định dạng `"subsystem:event-name"`, chỉ tên subsystem, hoặc chỉ tên event.
    * `trace-cmd start -e syscalls`: Bật tất cả các sự kiện trong nhóm `syscalls`.
    * `trace-cmd start -e sched`: Bật tất cả các sự kiện trong nhóm `sched` (lập lịch).
    * `trace-cmd start -e sched_switch`: Chỉ bật sự kiện `sched_switch`.
    * `trace-cmd start -e sched_switch -e sched_wakeup`: Bật nhiều sự kiện cùng lúc.
    * Từ khóa `all` có thể dùng để bật tất cả các sự kiện: `trace-cmd start -e all`.

Lệnh `strace` được dùng để theo dõi các lệnh gọi hệ thống của `trace-cmd` khi nó bật các sự kiện. Ví dụ, `strace` sẽ ghi lại các lệnh `open` và `write` mà `trace-cmd` thực hiện để bật các sự kiện.

`trace-cmd stop` là lệnh bổ trợ cho `trace-cmd start`, được dùng để **ngừng ghi dữ liệu** vào bộ đệm vòng.

* Lệnh này chỉ đơn thuần là dừng việc ghi, nhưng **không tắt hoàn toàn** `tracer` hoặc các sự kiện đang hoạt động.
* Việc này có nghĩa là các `hooks` (`móc`) của `ftrace` vẫn đang được kích hoạt trong kernel, và vẫn có một số chi phí hiệu năng nhất định. Tuy nhiên, nó sẽ giảm đáng kể chi phí ghi dữ liệu vào bộ đệm.
* Lệnh này rất hữu ích khi bạn muốn tạm dừng việc ghi dữ liệu để tránh tràn bộ đệm, sau đó có thể tiếp tục bằng `trace-cmd resume`.

### Trace-cmd show
`trace-cmd show` là một lệnh để hiển thị nội dung của các file trace đã được ghi lại bởi `ftrace`. Lệnh này giúp bạn xem kết quả của quá trình theo dõi một cách dễ dàng mà không cần phải tương tác trực tiếp với các file trong thư mục `tracefs`.


#### Cách thức hoạt động

* **Mặc định**: Lệnh `trace-cmd show` mặc định sẽ hiển thị nội dung của file `trace`. File này chứa dữ liệu trace đã được ghi vào bộ đệm vòng (ring buffer) và đã được "đóng băng" (frozen).
* **`trace_pipe`**: Khi bạn sử dụng tùy chọn **`-p`** (viết tắt của `pipe`), `trace-cmd` sẽ hiển thị nội dung của file `trace_pipe`. File này hiển thị dữ liệu trace theo thời gian thực (real-time) và sẽ bị xóa ngay sau khi được đọc.
* **`snapshot`**: Tùy chọn **`-s`** (viết tắt của `snapshot`) cho phép bạn xem nội dung của file `snapshot`. File này chứa một bản sao của bộ đệm vòng tại một thời điểm cụ thể.

#### Các tùy chọn khác

* **`show -f`**: Khi thêm tùy chọn `-f`, `trace-cmd show` sẽ hiển thị đường dẫn đầy đủ của file mà nó đang đọc. Điều này hữu ích để xác minh file nào đang được sử dụng.

#### Ví dụ tương đương

| Thao tác Ftrace với `cat`                                    | Thao tác với `trace-cmd`                 |
| ----------------------------------------------------------- | ---------------------------------------- |
| `cat /sys/kernel/debug/tracing/trace`                       | `trace-cmd show`                         |
| `cat /sys/kernel/debug/tracing/trace_pipe`                  | `trace-cmd show -p`                      |
| `cat /sys/kernel/debug/tracing/snapshot`                    | `trace-cmd show -s`                      |

Tóm lại, `trace-cmd show` cung cấp một giao diện đơn giản và tiện lợi hơn nhiều so với việc sử dụng `cat` để xem các file trace.

### Function filter
**Lọc hàm (Function Filtering)** với `trace-cmd` là một cách để giới hạn `function` và `function_graph` tracer chỉ theo dõi các hàm mà bạn quan tâm. Điều này giúp giảm đáng kể lượng dữ liệu được ghi lại và giúp bạn tập trung vào các khu vực mã nguồn cụ thể.


#### Cách thức hoạt động

Bạn sử dụng tùy chọn `-l` (`--limit`) để chỉ định tên hàm mà bạn muốn theo dõi.

* **Một hàm**: `trace-cmd start -p function -l do_page_fault` sẽ chỉ bật `function tracer` cho hàm `do_page_fault`.
* **Nhiều hàm**: Bạn có thể sử dụng nhiều tùy chọn `-l` để theo dõi nhiều hàm cùng lúc. Ví dụ: `trace-cmd start -p function -l do_page_fault -l vfs_read`.
* **Biểu thức glob**: `trace-cmd` cũng hỗ trợ các biểu thức glob để lọc theo mẫu.
    * `match*`: Lọc các hàm có tên bắt đầu bằng `match`.
    * `*match`: Lọc các hàm có tên kết thúc bằng `match`.
    * `*match*`: Lọc các hàm có tên chứa `match`.

`trace-cmd stat` là lệnh để kiểm tra xem bộ lọc đã được áp dụng hay chưa. Khi bạn chạy lệnh này sau khi đặt bộ lọc, nó sẽ hiển thị danh sách các hàm mà bạn đang theo dõi.

#### Lệnh `trace-cmd reset`

`trace-cmd reset` là một lệnh quan trọng để **đặt lại toàn bộ hệ thống trace**. Nó sẽ:
* Vô hiệu hóa tất cả các `tracers` (`-p nop`).
* Xóa tất cả các bộ lọc hàm và sự kiện.
* Vô hiệu hóa tất cả các sự kiện (`events`).
* Xóa toàn bộ nội dung trong trace buffer.

Lệnh này giúp đảm bảo rằng hệ thống `ftrace` của bạn sạch sẽ trước khi bắt đầu một phiên trace mới.

### Trace record & report
`trace-cmd record` và `trace-cmd report` là hai lệnh cốt lõi của `trace-cmd` được sử dụng để ghi lại và phân tích dữ liệu trace. `record` dùng để thu thập dữ liệu và `report` dùng để hiển thị dữ liệu đã thu thập.

#### `trace-cmd record`

`trace-cmd record` được sử dụng để ghi lại dữ liệu từ `ftrace` và lưu vào một tệp tin. Lệnh này có nhiều chức năng hơn `trace-cmd start`.

* **Cách hoạt động**: `record` thiết lập `ftrace` để ghi lại các sự kiện hoặc `tracers` đã chỉ định. Nó tạo ra một tiến trình ghi dữ liệu riêng cho mỗi CPU, các tiến trình này sẽ đọc trực tiếp dữ liệu nhị phân từ bộ đệm vòng (`trace_pipe_raw`) và lưu vào các tệp tin tạm thời. Sau đó, nó sẽ kết hợp tất cả các tệp này lại thành một tệp duy nhất tên là **`trace.dat`**.
* **Cú pháp**: `trace-cmd record [options] [command]`
    * Nếu bạn không chỉ định một lệnh nào sau `record`, nó sẽ tiếp tục ghi dữ liệu cho đến khi bạn nhấn `Ctrl+C`.
    * Ví dụ: `trace-cmd record -p function` sẽ ghi lại các lệnh gọi hàm cho đến khi bạn dừng nó.
    * Bạn cũng có thể chỉ định một lệnh, ví dụ: `trace-cmd record -p function ls`. Lệnh này sẽ ghi lại các lệnh gọi hàm trong suốt thời gian lệnh `ls` được thực thi và tự động dừng lại khi `ls` kết thúc.
* **Tùy chọn `-F`**: Tùy chọn này lọc để chỉ theo dõi các sự kiện hoặc hàm được kích hoạt bởi chính lệnh mà bạn chỉ định.
    * Ví dụ: `trace-cmd record -F -p function ls`. Lệnh này sẽ chỉ theo dõi các hàm được gọi bởi tiến trình `ls` chứ không phải các tiến trình khác đang chạy trên hệ thống.

#### `trace-cmd report`

`trace-cmd report` là lệnh để hiển thị báo cáo dễ đọc từ tệp `trace.dat` đã được tạo ra bởi `trace-cmd record`.
* **Mặc định**: Lệnh `trace-cmd report` mặc định sẽ phân tích và hiển thị nội dung của tệp `trace.dat` theo định dạng chuẩn.
* **Tùy chọn**:
    * `--events`: Liệt kê tất cả các sự kiện đã được ghi lại trong tệp `trace.dat`.
    * `-f`: Chỉ hiển thị các hàm đã được ghi lại (nếu bạn sử dụng `function tracer`).

#### Tùy chọn `-k` của `record`

* Mặc định, sau khi `trace-cmd record` hoàn tất, nó sẽ tự động đặt lại bộ đệm và vô hiệu hóa các `tracers` mà nó đã bật. Điều này đảm bảo trạng thái `ftrace` của bạn luôn sạch sẽ.
* Tùy chọn `-k` (`--keep`) sẽ ngăn `trace-cmd` thực hiện việc dọn dẹp này. Các `tracers` và các bộ đệm sẽ được giữ nguyên trạng thái sau khi lệnh kết thúc.

