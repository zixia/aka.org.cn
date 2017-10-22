//===================================================
//	和进程管理相关的数据类型定义，常量和变量声明
//  
//	注：篇幅较长的变量或结构的具体声明使用 { ... }略去
//===================================================

//===================================================
//文件sched.h中的声明
//===================================================
extern int nr_running, nr_tasks;
extern int last_pid;

#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		4
#define TASK_STOPPED		8
#define TASK_SWAPPING		16

//----------------------------------------------------------
#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2

extern rwlock_t tasklist_lock;
extern spinlock_t runqueue_lock;

//----------------------------------------------------------
/*
 * Open file table structure
 */
struct files_struct {
	atomic_t count;
	int max_fds;
	int max_fdset;
	int next_fd;
	struct file ** fd;	/* current fd array */
	fd_set *close_on_exec;
	fd_set *open_fds;
	fd_set close_on_exec_init;
	fd_set open_fds_init;
	struct file * fd_array[NR_OPEN_DEFAULT];
};

#define INIT_FILES  { ... }

struct fs_struct  { ... }

#define INIT_FS  { ... }

//----------------------------------------------------------
struct mm_struct { ... }

#define INIT_MM  { ... }

struct signal_struct {
	atomic_t		count;
	struct k_sigaction	action[_NSIG];
	spinlock_t		siglock;
};

//----------------------------------------------------------

#define INIT_SIGNALS  { ... }

struct user_struct;

struct task_struct  { ... }

/*
 * Limit the stack by to some sane default: root can always
 * increase this limit if needed..  8MB seems reasonable.
 */
#define _STK_LIM	(8*1024*1024)

#define DEF_PRIORITY	(20*HZ/100)	/* 210 ms time slices */

//----------------------------------------------------------
#define INIT_TASK  { ... }

union task_union {
	struct task_struct task;
	unsigned long stack[2048];
};

extern union task_union init_task_union;
extern struct   mm_struct init_mm;
extern struct task_struct *task[NR_TASKS];

extern struct task_struct **tarray_freelist;
extern spinlock_t taskslot_lock;

//----------------------------------------------------------
#define for_each_task(p) \
	for (p = &init_task ; (p = p->next_task) != &init_task ; )

//===================================================
//文件sched.c中的声明
//===================================================

//----------------------------------------------------------

long tick = (1000000 + HZ/2) / HZ;	/* timer interrupt period */

//----------------------------------------------------------

DECLARE_TASK_QUEUE(tq_timer);
DECLARE_TASK_QUEUE(tq_immediate);
DECLARE_TASK_QUEUE(tq_scheduler);

//----------------------------------------------------------
/*
 * phase-lock loop variables
 */
/* TIME_ERROR prevents overwriting the CMOS clock */
int time_state = TIME_OK;	/* clock synchronization status */
int time_status = STA_UNSYNC;	/* clock status bits */
long time_offset = 0;		/* time adjustment (us) */
long time_constant = 2;		/* pll time constant */
long time_tolerance = MAXFREQ;	/* frequency tolerance (ppm) */
long time_precision = 1;	/* clock precision (us) */
long time_maxerror = NTP_PHASE_LIMIT;	/* maximum error (us) */
long time_esterror = NTP_PHASE_LIMIT;	/* estimated error (us) */
long time_phase = 0;		/* phase offset (scaled us) */
long time_freq = ((1000000 + HZ/2) % HZ - HZ/2) << SHIFT_USEC;	/* frequency offset (scaled ppm) */
long time_adj = 0;		/* tick adjust (scaled 1 / HZ) */
long time_reftime = 0;		/* time at last adjustment (s) */

long time_adjust = 0;
long time_adjust_step = 0;

unsigned long global_event = 0;

extern int do_setitimer(int, struct itimerval *, struct itimerval *);

//----------------------------------------------------------
unsigned long volatile jiffies=0;

//----------------------------------------------------------
struct task_struct * task[NR_TASKS] = {&init_task, };

/*
 * We align per-CPU scheduling data on cacheline boundaries,
 * to prevent cacheline ping-pong.
 */
static union {
	struct schedule_data {
		struct task_struct * curr;
		cycles_t last_schedule;
	} schedule_data;
	char __pad [SMP_CACHE_BYTES];
} aligned_data [NR_CPUS] __cacheline_aligned = { {{&init_task,0}}};

//----------------------------------------------------------

#ifdef __SMP__

#define idle_task(cpu) (task[cpu_number_map[(cpu)]])
#define can_schedule(p)	(!(p)->has_cpu)

#else

#define idle_task(cpu) (&init_task)
#define can_schedule(p) (1)

