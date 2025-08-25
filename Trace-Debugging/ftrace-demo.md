Trong bài này chúng ta sẽ viết một module dưới kernel và insmod trên Raspberry Pi sau đó sẽ tiến hành `ftrace` để đánh giá về hiệu suất và hiểu được các sự kiện xảy ra trong kernel.
# 1. Setup trace-cmd
Các phiên bản trong meta-openembedded branch dunfell mặc định không có sẵn recipe `trace-cmd`.

Để cài đặt `trace-cmd` trên raspberry pi bằng Yocto thì ta cần tự tạo layer riêng.

Tạo layer riêng của bạn (nếu chưa có):
```bash
bitbake-layers create-layer ../meta-myextra
bitbake-layers add-layer ../meta-myextra
```
Bên trong layer vừa tạo, tạo file `trace-cmd.bb` folder files chứa file `0001-trace-cmd-make-it-build-with-musl.patch`.

Nội dung file patch:
```c
From b63f13d2df91ede45288653b21e0d30a6b45f2ac Mon Sep 17 00:00:00 2001
From: Beniamin Sandu <beniaminsandu@gmail.com>
Date: Mon, 30 Nov 2020 18:24:48 +0200
Subject: [PATCH] trace-cmd: make it build with musl

Signed-off-by: Beniamin Sandu <beniaminsandu@gmail.com>
---
 include/trace-cmd/trace-cmd.h |  1 +
 lib/trace-cmd/trace-msg.c     |  1 +
 lib/trace-cmd/trace-plugin.c  |  2 ++
 lib/tracefs/tracefs-events.c  |  1 +
 tracecmd/trace-agent.c        |  1 +
 tracecmd/trace-setup-guest.c  |  1 +
 tracecmd/trace-tsync.c        | 15 ++++++++++-----
 7 files changed, 17 insertions(+), 5 deletions(-)

diff --git a/include/trace-cmd/trace-cmd.h b/include/trace-cmd/trace-cmd.h
index f3c95f3..a697905 100644
--- a/include/trace-cmd/trace-cmd.h
+++ b/include/trace-cmd/trace-cmd.h
@@ -6,6 +6,7 @@
 #ifndef _TRACE_CMD_H
 #define _TRACE_CMD_H
 
+#include <sys/types.h>
 #include "traceevent/event-parse.h"
 
 #define TRACECMD_MAGIC { 23, 8, 68 }
diff --git a/lib/trace-cmd/trace-msg.c b/lib/trace-cmd/trace-msg.c
index 4a0bfa9..fc1f2c7 100644
--- a/lib/trace-cmd/trace-msg.c
+++ b/lib/trace-cmd/trace-msg.c
@@ -21,6 +21,7 @@
 #include <arpa/inet.h>
 #include <sys/types.h>
 #include <linux/types.h>
+#include <byteswap.h>
 
 #include "trace-write-local.h"
 #include "trace-cmd-local.h"
diff --git a/lib/trace-cmd/trace-plugin.c b/lib/trace-cmd/trace-plugin.c
index 92f9edf..c2ef3dc 100644
--- a/lib/trace-cmd/trace-plugin.c
+++ b/lib/trace-cmd/trace-plugin.c
@@ -8,6 +8,8 @@
 #include <dlfcn.h>
 #include <sys/stat.h>
 #include <libgen.h>
+#include <limits.h>
+
 #include "trace-cmd.h"
 #include "trace-local.h"
 
diff --git a/lib/tracefs/tracefs-events.c b/lib/tracefs/tracefs-events.c
index 8e825f5..a8d8560 100644
--- a/lib/tracefs/tracefs-events.c
+++ b/lib/tracefs/tracefs-events.c
@@ -13,6 +13,7 @@
 #include <errno.h>
 #include <sys/stat.h>
 #include <fcntl.h>
+#include <limits.h>
 
 #include "kbuffer.h"
 #include "tracefs.h"
diff --git a/tracecmd/trace-agent.c b/tracecmd/trace-agent.c
index b581696..abfefac 100644
--- a/tracecmd/trace-agent.c
+++ b/tracecmd/trace-agent.c
@@ -20,6 +20,7 @@
 #include <unistd.h>
 #include <linux/vm_sockets.h>
 #include <pthread.h>
+#include <limits.h>
 
 #include "trace-local.h"
 #include "trace-msg.h"
diff --git a/tracecmd/trace-setup-guest.c b/tracecmd/trace-setup-guest.c
index 899848c..99595a1 100644
--- a/tracecmd/trace-setup-guest.c
+++ b/tracecmd/trace-setup-guest.c
@@ -13,6 +13,7 @@
 #include <string.h>
 #include <sys/stat.h>
 #include <unistd.h>
+#include <limits.h>
 
 #include "trace-local.h"
 #include "trace-msg.h"
diff --git a/tracecmd/trace-tsync.c b/tracecmd/trace-tsync.c
index e639788..b8b5ac3 100644
--- a/tracecmd/trace-tsync.c
+++ b/tracecmd/trace-tsync.c
@@ -104,13 +104,15 @@ int tracecmd_host_tsync(struct buffer_instance *instance,
 
 	pthread_attr_init(&attrib);
 	pthread_attr_setdetachstate(&attrib, PTHREAD_CREATE_JOINABLE);
-	if (!get_first_cpu(&pin_mask, &mask_size))
-		pthread_attr_setaffinity_np(&attrib, mask_size, pin_mask);
 
 	ret = pthread_create(&instance->tsync_thread, &attrib,
 			     tsync_host_thread, &instance->tsync);
-	if (!ret)
+	if (!ret) {
+		if (!get_first_cpu(&pin_mask, &mask_size))
+			pthread_setaffinity_np(instance->tsync_thread, mask_size, pin_mask);
 		instance->tsync_thread_running = true;
+	}
+
 	if (pin_mask)
 		CPU_FREE(pin_mask);
 	pthread_attr_destroy(&attrib);
@@ -243,11 +245,14 @@ unsigned int tracecmd_guest_tsync(char *tsync_protos,
 	pthread_attr_init(&attrib);
 	tsync->sync_proto = proto;
 	pthread_attr_setdetachstate(&attrib, PTHREAD_CREATE_JOINABLE);
-	if (!get_first_cpu(&pin_mask, &mask_size))
-		pthread_attr_setaffinity_np(&attrib, mask_size, pin_mask);
 
 	ret = pthread_create(thr_id, &attrib, tsync_agent_thread, tsync);
 
+	if (!ret) {
+		if (!get_first_cpu(&pin_mask, &mask_size))
+			pthread_setaffinity_np(*thr_id, mask_size, pin_mask);
+	}
+
 	if (pin_mask)
 		CPU_FREE(pin_mask);
 	pthread_attr_destroy(&attrib);
-- 
2.25.1
```
Nội dung file `trace-cmd.bb`:
```
SUMMARY = "User-space front-end command-line tool for ftrace"

LICENSE = "GPL-2.0-only & LGPL-2.1-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/rostedt/trace-cmd.git;branch=master \
	file://0001-trace-cmd-make-it-build-with-musl.patch"

SRCREV = "530b1a0caef39466e16bbd49de5afef89656f03f"

S = "${WORKDIR}/git"

do_install() {
       oe_runmake etcdir=${sysconfdir} DESTDIR=${D} install
       mkdir -p ${D}${libdir}/traceevent/plugins/${BPN}
       mv ${D}/${libdir}/traceevent/plugins/*.so ${D}${libdir}/traceevent/plugins/${BPN}/
}

FILES:${PN} += "${libdir}/traceevent/plugins"
```

Build recipe:
```bash
bitbake trace-cmd
```

Sau đó thêm `IMAGE_INSTALL_append = " trace-cmd"` vào file `local.config` và tiến hành build lại image `bitbake core-image-minimal` rồi flash vào thẻ nhớ.

Sau khi flash thẻ nhớ, tiến hành khởi động pi và kiểm tra phiên bản `trace-cmd` xem đã có trên pi chưa:
![ftrace-1](https://toanonestar.github.io/C-document/image/ftrace-1.png)
# 2. Chương trình module kernel
Bài viết này minh họa cách viết và sử dụng một Linux kernel module đơn giản để đánh giá độ trễ (scheduling latency) của tiến trình bằng cách kết hợp với `ftrace` và công cụ `trace-cmd`.

Cụ thể:

* Tạo module gpio-ftrace-demo.ko sinh ra hai tiến trình kernel thread (`demo_consumer` và `demo_contender`).

* Dùng `trace-cmd` để ghi lại sự kiện `sched_wakeup` và `sched_switch`.

* Phân tích log để đo thời gian trễ từ lúc tiến trình `demo_consumer` được đánh thức đến khi nó thực sự chạy trên CPU.

Tạo file `gpio-ftrace-demo.c`:
```c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/hrtimer.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include <linux/wait.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Toan");
MODULE_DESCRIPTION("Raspberry Pi ftrace demo: hrtimer->kthread, mutex, optional GPIO toggle");

static unsigned int period_ms = 100;         // hrtimer period
module_param(period_ms, uint, 0644);
MODULE_PARM_DESC(period_ms, "Hrtimer period (ms).");

static unsigned int busy_us = 1000;          // CPU load inside kthread handler (udelay)
module_param(busy_us, uint, 0644);
MODULE_PARM_DESC(busy_us, "udelay workload in kthread (us).");

static int iterations = 0;                   // 0 = run forever
module_param(iterations, int, 0644);
MODULE_PARM_DESC(iterations, "Number of processing iterations before stopping (0 = infinite).");

static int gpio = 26;                        // -1 = no GPIO; >=0 = toggle GPIO
module_param(gpio, int, 0644);
MODULE_PARM_DESC(gpio, "GPIO output to toggle on each tick (-1 = disabled).");

static unsigned int contender_period_ms = 37; // background thread creates contention
module_param(contender_period_ms, uint, 0644);
MODULE_PARM_DESC(contender_period_ms, "Period of contender thread (ms).");

static unsigned int contender_hold_us = 500; // how long contender holds the mutex
module_param(contender_hold_us, uint, 0644);
MODULE_PARM_DESC(contender_hold_us, "Mutex hold time by contender thread (us).");

static int nice_consumer = 0;                // scheduling priority (nice value) of consumer
module_param(nice_consumer, int, 0644);
MODULE_PARM_DESC(nice_consumer, "Nice value for consumer thread (negative = higher priority).");

// --------- Internal state ----------
static struct hrtimer tick_timer;
static atomic_t pending = ATOMIC_INIT(0);
static wait_queue_head_t wq;

static struct task_struct *consumer_task;
static struct task_struct *contender_task;

static struct mutex demo_lock;
static bool stop_flag;

#define MAX_NICE_VALUE 19
#define MIN_NICE_VALUE -20

/**
 * @brief Write a formatted message into the ftrace buffer.
 *
 * This function is a lightweight wrapper around trace_printk,
 * allowing printf-style formatted strings to be written into
 * the ftrace tracing buffer for debugging and performance analysis.
 *
 * @param fmt [in]  Format string (printf-like).
 * @param ... [in]  Variable arguments corresponding to the format string.
 *
 * @return void
 */
static inline void demo_trace(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    trace_printk(fmt, args);
    va_end(args);
}

/**
 * @brief High-resolution timer callback for periodic tick events.
 *
 * This function is invoked when the hrtimer expires. It increments the
 * pending counter, records a trace event, wakes up any waiting consumers,
 * and then re-arms the timer to trigger again after the specified period.
 *
 * @param t [in] Pointer to the hrtimer structure associated with this callback.
 *
 * @return HRTIMER_RESTART to indicate that the timer should be restarted.
 */
static enum hrtimer_restart tick_fn(struct hrtimer *t) {
    int old = atomic_fetch_add(1, &pending);
    demo_trace("DEMO TICK: pend_before=%d period_ms=%u\n", old, period_ms);
    wake_up_interruptible(&wq);

    hrtimer_forward_now(&tick_timer, ms_to_ktime(period_ms));
    return HRTIMER_RESTART;
}

/**
 * @brief Kernel thread function for the demo consumer.
 *
 * This function represents the main loop of the consumer thread.
 * It waits for pending events (set by the timer or contender),
 * processes them one by one, and simulates workload by optionally
 * toggling a GPIO pin and executing a busy-wait delay.
 *
 * Behavior:
 * - Sets the thread’s scheduling nice value if configured.
 * - Waits for events to be signaled via the wait queue.
 * - Handles pending events by acquiring a mutex, executing
 *   demo work, and tracing begin/end markers.
 * - Optionally toggles GPIO state on each iteration.
 * - Optionally inserts a busy delay (`udelay`) for load simulation.
 * - Stops once the requested number of iterations is reached or
 *   when the thread is asked to terminate.
 *
 * @param data [in] Unused thread argument (may be NULL).
 *
 * @return 0 Always returns 0 on thread exit.
 */
static int consumer_thread_fn(void *data) {
    int handled, seq = 0;
    if (nice_consumer)
        set_user_nice(current, clamp_val(nice_consumer, MIN_NICE_VALUE, MAX_NICE_VALUE));

    demo_trace("DEMO CONSUMER: start (nice=%d)\n", task_nice(current));

    while (!kthread_should_stop()) {
        wait_event_interruptible(wq, kthread_should_stop() || atomic_read(&pending) > 0);
    
        if (kthread_should_stop())
            break;

        handled = atomic_xchg(&pending, 0);
        while (handled-- > 0) {
            mutex_lock(&demo_lock);
            demo_trace("DEMO RUN: seq=%d begin\n", ++seq);

            if (gpio >= 0) {
                static bool level;
                gpio_set_value(gpio, level);
                level = !level;
            }

            if (busy_us)
                udelay(busy_us);

            demo_trace("DEMO RUN: seq=%d end\n", seq);
            mutex_unlock(&demo_lock);

            if (iterations > 0 && seq >= iterations) {
                demo_trace("DEMO CONSUMER: reached iterations=%d\n", iterations);
                stop_flag = true;
                wake_up_interruptible(&wq);
                return 0;
            }
        }
    }
    demo_trace("DEMO CONSUMER: stop\n");
    return 0;
}

/**
 * @brief Kernel thread function for the demo contender.
 *
 * This function models a "contender" task that periodically acquires
 * the demo mutex to simulate contention with the consumer thread.
 *
 * Behavior:
 * - Logs start parameters (period and hold time).
 * - Sleeps for @p contender_period_ms between iterations
 *   (interruptible sleep).
 * - On each wake-up:
 *   - Acquires the demo mutex.
 *   - Optionally holds the mutex for @p contender_hold_us microseconds
 *     to simulate lock contention.
 *   - Releases the mutex.
 *   - Emits a trace marker to indicate a contender tick.
 * - Stops gracefully when the thread is signaled to terminate.
 *
 * @param data [in] Unused thread argument (may be NULL).
 *
 * @return 0 Always returns 0 on thread exit.
 */
static int contender_thread_fn(void *data) {
    demo_trace("DEMO CONTENDER: start (period_ms=%u hold_us=%u)\n",
               contender_period_ms, contender_hold_us);
    while (!kthread_should_stop()) {
        if (msleep_interruptible(contender_period_ms)) break;

        mutex_lock(&demo_lock);
        if (contender_hold_us) {
            udelay(contender_hold_us);
        }
        mutex_unlock(&demo_lock);
        demo_trace("DEMO CONTENDER: tick\n");
    }
    demo_trace("DEMO CONTENDER: stop\n");
    return 0;
}

/**
 * @brief Module initialization function for rpi_ftrace_demo.
 *
 * This function is called when the kernel module is loaded.
 * It sets up synchronization primitives, GPIO (optional),
 * worker threads (consumer and contender), and starts the
 * periodic high-resolution timer.
 *
 * Steps performed:
 * - Initializes wait queue and mutex.
 * - Requests and configures GPIO pin (if @p gpio >= 0).
 * - Creates the consumer kernel thread (@ref consumer_thread_fn).
 * - Creates the contender kernel thread (@ref contender_thread_fn).
 * - Initializes and starts the high-resolution timer (@ref tick_fn).
 * - Logs success/failure messages to the kernel log buffer.
 *
 * Error handling:
 * - If GPIO request fails, initialization is aborted.
 * - If creating consumer or contender thread fails, previously
 *   allocated resources (threads, GPIO) are cleaned up.
 *
 * @param void No arguments.
 *
 * @return 0 on success,
 *         negative error code on failure (e.g., thread creation or GPIO request failure).
 */
static int __init rpi_ftrace_demo_init(void) {
    int ret;

    pr_info("rpi_ftrace_demo: load (period=%ums, busy=%uus, iter=%d, gpio=%d)\n",
            period_ms, busy_us, iterations, gpio);

    init_waitqueue_head(&wq);
    mutex_init(&demo_lock);

    if (gpio >= 0) {
        ret = gpio_request(gpio, "rpi_ftrace_demo");
        if (ret) {
            pr_err("rpi_ftrace_demo: gpio_request(%d) failed=%d\n", gpio, ret);
            return ret;
        }
        gpio_direction_output(gpio, 0);
    }

    consumer_task = kthread_run(consumer_thread_fn, NULL, "demo_consumer");
    if (IS_ERR(consumer_task)) {
        ret = PTR_ERR(consumer_task);
        consumer_task = NULL;
        pr_err("rpi_ftrace_demo: failed to create consumer kthread, err=%d\n", ret);
        if (gpio >= 0) gpio_free(gpio);
        return ret;
    }

    contender_task = kthread_run(contender_thread_fn, NULL, "demo_contender");
    if (IS_ERR(contender_task)) {
        ret = PTR_ERR(contender_task);
        contender_task = NULL;
        pr_err("rpi_ftrace_demo: failed to create contender kthread, err=%d\n", ret);
        kthread_stop(consumer_task);
        consumer_task = NULL;
        if (gpio >= 0) gpio_free(gpio);
        return ret;
    }

    hrtimer_init(&tick_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    tick_timer.function = tick_fn;
    hrtimer_start(&tick_timer, ms_to_ktime(period_ms), HRTIMER_MODE_REL);

    pr_info("rpi_ftrace_demo: started.\n");
    return 0;
}

/**
 * @brief Module cleanup function for rpi_ftrace_demo.
 *
 * This function is called when the kernel module is unloaded.
 * It stops active timers and threads, releases allocated GPIO,
 * and ensures all resources are properly cleaned up.
 *
 * Steps performed:
 * - Sets @p stop_flag to notify worker threads to exit.
 * - Cancels the high-resolution timer (@ref tick_timer).
 * - Wakes and stops the consumer thread (if running).
 * - Stops the contender thread (if running).
 * - Resets and frees the GPIO pin (if allocated).
 *
 * @param void No arguments.
 *
 * @return void
 */
static void __exit rpi_ftrace_demo_exit(void) {
    pr_info("rpi_ftrace_demo: unload\n");

    stop_flag = true;

    hrtimer_cancel(&tick_timer);

    if (consumer_task) {
        wake_up_interruptible(&wq);
        kthread_stop(consumer_task);
    }
    if (contender_task) {
        kthread_stop(contender_task);
    }
    if (gpio >= 0) {
        gpio_set_value(gpio, 0);
        gpio_free(gpio);
    }
}

module_init(rpi_ftrace_demo_init);
module_exit(rpi_ftrace_demo_exit);
```
Tạo `Makefile`:
```bash
obj-m := gpio-ftrace-demo.o

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

install: modules_install

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install
```
Tạo file `ftrace-kernel.bb`:
```bash
SUMMARY = "A simple GPIO kernel module"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://gpio-ftrace-demo.c \
           file://Makefile"

S = "${WORKDIR}"

inherit module

EXTRA_OEMAKE:append:class-target = " KERNEL_SRC=${STAGING_KERNEL_DIR}"
KERNEL_MODULE_PROBE = "gpio-ftrace-demo"
```

Build recipe:
```bash
bitbake ftrace-kernel
```
Quá trình build kết thúc sẽ tạo ra file `gpio-ftrace-demo.ko`.
Tìm đường dẫn tới file bằng lệnh `find . -name "gpio-ftrace-demo.ko"` ở trong thư mục `yocto/poky` sau đó scp file .ko qua Raspberry Pi.

# 3. Tiến hành ftrace
## 3.1. Nạp module

1. Copy file `.ko` sang Pi (nếu build trên PC Yocto thì file nằm trong `tmp/deploy/images/.../modules/` hoặc `tmp/work/.../rpi-ftrace-demo/`):

   ```bash
   scp gpio-ftrace-demo.ko pi@<IP_RPI>:/tmp/
   ```

2. Trên Pi, nạp module với tham số mặc định (chu kỳ 100ms, bận CPU 1000us, không GPIO):

   ```bash
   sudo insmod /tmp/gpio-ftrace-demo.ko
   ```

3. Kiểm tra log kernel để chắc chắn module chạy:
    ```bash
    dmesg | tail -n 20
    ```
![ftrace-3](https://toanonestar.github.io/C-document/image/ftrace-3.png)


## 3.2. Ghi lại trace với trace-cmd.
Chạy trên Pi:

```bash
# Reset mọi trace cũ
sudo trace-cmd reset

# Ghi trace trong 5 giây
sudo trace-cmd record -p function_graph -e sched -e timer -o demo_trace.dat sleep 5

# Xem nhanh 50 dòng đầu
sudo trace-cmd report -i demo_trace.dat | head -n 50
```
Kết quả:
![ftrace-4](https://toanonestar.github.io/C-document/image/ftrace-4.png)

**function\_graph tracer** ghi lại call stack các hàm kernel khi process `sleep` đang chạy. Cột bên trái có:

* **PID / process**: ở đây là `sleep-8880`.
* **Timestamp**: ví dụ `1640.846298`.
* **funcgraph\_entry/exit**: điểm vào và ra của hàm kernel.
* **Thời gian thực thi**: vd `+14.000 us` cho biết hàm đó mất bao nhiêu µs.

Ví dụ trong ảnh:

```
mutex_unlock();              +14.000 us
do_DataAbort() { ... }       +21.000 us
sys_execve() { ... }         +86.000 us
```

Điều này cho thấy:

* `sleep` gọi `execve` → kernel xử lý phân trang (`do_page_fault`) → update mm → thoát ra.
* Mỗi bước có thời gian đo được → ta thấy đường đi và hiệu suất của từng hàm.

**Đánh giá nhanh:**

* Trace đã ghi thành công, thấy rõ thời gian từng hàm.
* Thời gian µs (micro giây) rất nhỏ, tức là không có bottleneck lớn.
* Đây mới chỉ thấy process user (`sleep`) → ta cần xem thêm hoạt động của **module của bạn** (`gpio-ftrace-demo`).

Rồi chuẩn luôn 🚀. Với hướng 2 mình sẽ bật **scheduler events** (`sched_switch`, `sched_wakeup`) để thấy rõ khi thread trong module chạy, sleep, hoặc bị wakeup.

### 3.2.1. Reset và ghi lại trace với sched events

Chạy trên Pi:

```bash
# Xóa dữ liệu trace cũ
sudo trace-cmd reset

# Ghi trace trong 5 giây, bật sched events
sudo trace-cmd record -e sched_switch -e sched_wakeup -p function_graph -o sched_trace.dat sleep 5
```

Lệnh này sẽ:

* Dùng `function_graph` để có stack call.
* Ghi thêm sự kiện **chuyển đổi luồng** (`sched_switch`) và **đánh thức luồng** (`sched_wakeup`).

Sau khi chạy xong:

```bash
sudo trace-cmd report -i sched_trace.dat | head -n 50
```

Rồi thử lọc riêng luồng của module (`gpio-ftrace-demo`):

```bash
sudo trace-cmd report -i sched_trace.dat | grep gpio-ftrace-demo -A5 -B5
```

Kết quả:
![ftrace-5](https://toanonestar.github.io/C-document/image/ftrace-5.png)


### 3.2.2. Phân tích kết quả

* Hai kthread của module đã chạy và **có mặt trong sched\_switch**:

  * `demo_contender-8001`
  * `demo_consumer-8000`
* Ví dụ dòng:

  ```
  ... sched_switch: prev_comm=trace-cmd prev_state=R+ ==> next_comm=demo_contender next_pid=8001 ...
  ```

  → CPU **chuyển sang chạy `demo_contender`**. `prev_state=R+` nghĩa là tác vụ trước vẫn “runnable” nhưng bị **preempt** (bị cướp CPU).
* Các dòng:

  ```
  prev_comm=demo_contender ... prev_state=S ==> next_comm=ksoftirqd/0 ...
  ```

  → `demo_contender` **tự ngủ** (`msleep_interruptible` trong code), nên trạng thái `S` (sleep) và scheduler chọn tác vụ khác (`ksoftirqd/0` hoặc user task như `sh`, `head`, `trace-cmd`).
* Ở cuối ảnh có:

  ```
  ... ==> next_comm=demo_consumer next_pid=8000 ...
  ```

  → `demo_consumer` được **lên CPU** (thường là sau khi được đánh thức bởi **hrtimer** qua `wake_up_interruptible()` trong `tick_fn`).

\=> Tóm lại: luồng chuyển CPU qua lại giữa `demo_contender` ↔ các tác vụ khác ↔ `demo_consumer` **đúng như thiết kế**. Bây giờ ta sẽ đo **độ trễ đánh thức (wakeup latency)** từ lúc hrtimer hết hạn → `sched_wakeup` của `demo_consumer` → `sched_switch` vào `demo_consumer`.


## 3.3. Đo “wakeup latency”

```bash
# Ghi EVENT-ONLY (không bật function_graph để thời gian chuẩn hơn)
# -P <pid>: chỉ theo dõi PID liên quan -> giảm nhiễu
trace-cmd record -o wake.dat -p nop \
  -P $PID_CONS -P $PID_CONT \
  -e timer:hrtimer_start -e timer:hrtimer_expire_entry \
  -e sched:sched_wakeup -e sched:sched_switch \
  sleep 5

# Trích các mốc quan trọng để quan sát chuỗi: expire -> wakeup -> switch
trace-cmd report -i wake.dat | egrep 'hrtimer_expire_entry|sched_wakeup: .*demo_consumer|sched_switch: .*next_comm=demo_consumer' | head -n 60
```
 Kết quả thu được cho thấy trace đã ghi nhận đầy đủ các event mong muốn:

* Có nhiều lần `sched_wakeup` của `demo_consumer`
* Sau đó liền kề là `sched_switch` → từ `sleep` hoặc `<idle>` sang `demo_consumer`.

Ví dụ:

```
sleep-16080 [000]  2980.168159: sched_wakeup:         comm=demo_consumer pid=8000 prio=120 target_cpu=000
sleep-16080 [000]  2980.168277: sched_switch:         prev_comm=sleep prev_pid=16080 prev_prio=120 prev_state=R ==> next_comm=demo_consumer next_pid=8000 next_prio=120
```

Điều này xác nhận luồng hoạt động:

1. `sleep` hoặc `tick_fn` gọi hẹn giờ (`hrtimer_expire_entry`).
2. Kernel đánh thức `demo_consumer` (`sched_wakeup`).
3. CPU thực sự chuyển sang chạy `demo_consumer` (`sched_switch`).

Trích xuất chuỗi wakeup → chạy để phân tích latency

```bash
trace-cmd report -i wake.dat | \
egrep 'sched_wakeup: .*demo_consumer|sched_switch: .*next_comm=demo_consumer' \
| awk '
/sched_wakeup/ {wake=$2; printf "Wakeup at %s ms ", $2}
/sched_switch/ {run=$2; printf "Run at %s ms, latency = %.3f ms\n", $2, ($2-wake)}'
```

Cái này sẽ in độ trễ từ lúc `demo_consumer` được đánh thức cho đến khi thực sự chạy.


Có thể dùng grep thêm `-A` và `-B` (before/after):

```bash
trace-cmd report -i wake.dat | \
grep "sched_wakeup:.*demo_consumer" -A5 -B5
```
Kết quả:

![ftrace-6](https://toanonestar.github.io/C-document/image/ftrace-6.png)

**Đánh giá hoạt động & độ trễ**

Ta có đúng chuỗi sự kiện theo thiết kế:

```
hrtimer_expire_entry ... function=tick_fn
print: demo_trace: DEMO TICK ...
sched_wakeup: comm=demo_consumer pid=8000
sched_switch: ... ==> next_comm=demo_consumer
print: demo_trace: DEMO RUN: ... begin
print: demo_trace: DEMO RUN: ... end
```

Ví dụ một cụm tiêu biểu (mình quy đổi chênh lệch thời gian):

* `2985.068105` — `sched_wakeup demo_consumer`
* `2985.068159` — `sched_switch` sang `demo_consumer`
  ➜ **wakeup→run latency ≈ 54 µs**

Các cụm sau cũng rất sát (thường 40–70 µs). Với Raspberry Pi và thread `SCHED_OTHER` (nice=0), đây là con số tốt, cho thấy:

* Timer nổ → đánh thức consumer nhanh.
* Scheduler gần như lập tức cấp CPU cho `demo_consumer` (vì thời điểm đó CPU rảnh: `prev_comm=swapper`).

Khoảng cách giữa các lần `tick_fn` của bạn cũng đều đặn \~100 ms (nhìn các mốc 2980.168/268/368/468/568/668/768…), đúng với `period_ms=100`.

\=> Tổng kết: module đang hoạt động chuẩn, latency đánh thức thấp & ổn định, không thấy trì hoãn do cạnh tranh CPU.

## 3.4. Phân tích độ trễ wakeup → run

Ở bước 3 bạn đã thấy consumer được `sched_wakeup` và `sched_switch` gần như ngay lập tức. Bây giờ ta **định lượng** bằng cách lọc và tính toán thống kê độ trễ từ file trace.


### 3.4.1. Trích xuất các sự kiện liên quan

Trong file `wake.dat` bạn có đủ thông tin:

* `sched_wakeup: comm=demo_consumer ...`
* `sched_switch: ... next_comm=demo_consumer`

Ta cần lấy cặp này để tính khoảng cách thời gian.

### 3.4.2. Lệnh phân tích nhanh

Chạy trên Pi:

```bash
trace-cmd report -i /tmp/wake.dat | \
egrep 'sched_wakeup: .*demo_consumer|sched_switch: .*next_comm=demo_consumer' | \
awk '
/sched_wakeup:/ {t=$3; sub(":","",t); wake=t}
/sched_switch:/ {t=$3; sub(":","",t); run=t; lat=(run-wake)*1000;  # ms→µs
                 if(lat>=0){ sum+=lat; n++; if(min==""||lat<min)min=lat; if(lat>max)max=lat } }
END { printf "samples=%d  min=%.1f us  max=%.1f us  avg=%.1f us\n", n, min, max, (n?sum/n:0) }'
```

Kết quả:
```bash
samples=50  min=0.0 us  max=0.5 us  avg=0.1 us
```
### 3.4.3. Đánh giá kết quả
Ta thấy:

* **samples=50** → tức là consumer đã được wakeup \~50 lần trong khoảng trace của bạn (khớp với timer tick).
* **min=0.0 µs** → có trường hợp consumer gần như chạy ngay lập tức khi kernel wakeup (không có độ trễ thực sự đo được).
* **avg=0.1 µs** và **max=0.5 µs** → độ trễ cực kỳ thấp, gần như bằng 0.

Ý nghĩa

* Đây là **baseline latency** trong tình huống hệ thống gần như không có tải → wakeup → run gần như tức thời.
* Kernel scheduler của bạn trên Raspberry Pi (Yocto build) đang hoạt động tối ưu trong trường hợp đơn giản này.
* Kết quả này cho thấy không có bottleneck ở hrtimer → wakeup → sched path.

Để thấy sự khác biệt, ta cần tạo **tải cạnh tranh**:

* `demo_contender` sẽ chiếm CPU lâu hơn (tăng `contender_hold_us`).
* Consumer có thể bị delay → bạn sẽ thấy **avg latency tăng lên, max latency nhảy vọt**.
* Đây chính là cơ chế giúp bạn **đánh giá performance impact** khi có contention.

## 3.5. Điều chỉnh thông số để quan sát sự khác biệt
Tạo file:

```bash
nano analyze_latency.sh
```

Dán nội dung sau:

```bash
#!/bin/sh
# Usage: ./analyze_latency.sh <trace.dat> <task_name>
# Example: ./analyze_latency.sh wake.dat demo_consumer

if [ $# -lt 2 ]; then
    echo "Usage: $0 <trace.dat> <task_name>"
    exit 1
fi

TRACE_FILE=$1
TASK_NAME=$2

trace-cmd report -i "$TRACE_FILE" | awk -v task="$TASK_NAME" '
function ts() {
  for (i=1;i<=NF;i++) if ($i ~ /^[0-9]+\.[0-9]+:$/) {x=$i; sub(":","",x); return x}
  return ""
}
# khi task được wakeup
/sched_wakeup: .*comm=/ || /sched_wakeup_new: .*comm=/ {
  if ($0 ~ "comm=" task) {
    wake = ts(); next
  }
}
# khi CPU chuyển sang task đó
/sched_switch: .*next_comm=/ {
  if ($0 ~ "next_comm=" task) {
    run = ts();
    if (wake != "") {
      lat = (run - wake) * 1000; # ms → µs
      if (lat >= 0) {
        sum += lat; n++;
        if (min == "" || lat < min) min = lat;
        if (lat > max) max = lat;
      }
      wake = ""; # reset sau mỗi lần đo
    }
  }
}
END {
  printf "Task=%s  samples=%d  min=%.1f us  max=%.1f us  avg=%.1f us\n",
         task, n, (min==""?0:min), (max==""?0:max), (n?sum/n:0)
}'
```

Lưu lại và cấp quyền chạy:

```bash
chmod +x analyze_latency.sh
```


Unload và load lại module với `contender_hold_us` lớn hơn, ví dụ:

```bash
sudo rmmod gpio-ftrace-demo
sudo insmod gpio-ftrace-demo.ko period_ms=100 busy_us=1000 contender_hold_us=20000
```

(`contender_hold_us=20000` → contender giữ mutex 20ms, lâu gấp 40 lần so với tick period).

Dùng lệnh sau và quan sát kết quả:

```bash
./analyze_latency.sh wake.dat demo_consumer
./analyze_latency.sh wake.dat demo_contender
```

Kết quả:
```bash
Task=demo_consumer  samples=50  min=0.0 us  max=4.5 us  avg=1.1 us
Task=demo_contender  samples=63  min=0.0 us  max=0.2 us  avg=0.0 us
```

## 3.6. Đánh giá kết quả

### 3.6.1. Kết quả trước khi thay đổi

* Khi module **chưa có tham số `contender_hold_us`** hoặc giá trị nhỏ:

  ```
  samples=50  min=0.0 us  max=0.5 us  avg=0.1 us
  ```

  ➝ Độ trễ (wake → run của `demo_consumer`) cực thấp:

  * **Trung bình \~0.1 µs**
  * **Đỉnh cao nhất (max) chỉ \~0.5 µs**
    → Điều này cho thấy CPU gần như luôn sẵn sàng chạy `demo_consumer` ngay sau khi có wakeup.
    → Hiệu suất **tối ưu**, gần như “ideal scheduling latency”.

### 3.6.2. Kết quả sau khi thêm `contender_hold_us=20000` (20ms)

```
samples=50  min=0.0 us  max=4.5 us  avg=1.1 us
```

➝ Độ trễ tăng rõ rệt:

* **Trung bình \~1.1 µs** (gấp 11 lần so với trước)
* **Đỉnh cao nhất (max) \~4.5 µs** (gấp 9 lần trước đó)

---

### 3.6.3. Phân tích nguyên nhân

* `demo_contender` bây giờ giữ CPU lâu hơn (`20,000 µs = 20 ms`) trước khi nhường CPU.
* Điều này làm cho `demo_consumer` khi được wakeup thường phải **chờ contender nhả CPU**.
* Vì vậy:

  * Có những lần `latency` tăng lên vài micro giây (đỉnh 4.5 µs).
  * Tuy nhiên, do Linux scheduler vẫn cố gắng phân bổ công bằng và tick-based preemption, nên latency **không bùng nổ quá lớn** (chỉ vài µs, chứ không lên hàng ms).

---

### 3.6.4. Đánh giá hiệu suất

* **Trước**: hệ thống có độ trễ cực thấp, rất phù hợp cho ứng dụng **real-time** cần phản hồi tức thì.
* **Sau**: độ trễ tăng nhưng vẫn ở mức **micro giây** (cực kỳ nhỏ so với 20ms workload của contender).
* Với kết quả này có thể kết luận:

  * Module và cách trace hoạt động đúng.
  * `contender_hold_us` làm lộ ra tác động của **CPU contention** lên độ trễ wakeup → run.
  * Hiệu suất tổng thể vẫn **rất tốt**, nhưng **tính real-time predictability** đã kém hơn (max latency cao hơn).

# 4. Tổng kết
Trong bài lab này, ta đã xây dựng một kernel module đơn giản (gpio-ftrace-demo.ko) để sinh ra hai tiến trình thử nghiệm:

- `demo_consumer`: mô phỏng một tác vụ cần được đánh thức định kỳ.

- `demo_contender`: mô phỏng tải nền chiếm CPU trong một khoảng thời gian cấu hình được (`contender_hold_us`).

Sau đó sử dụng công cụ `ftrace` thông qua `trace-cmd` để đo độ trễ từ thời điểm tiến trình `demo_consume`r được wakeup cho đến khi thật sự chạy trên CPU (wake → run latency).