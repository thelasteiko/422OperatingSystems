
#ifindef CPU
#define CPU

#define MAXTIME 300

typedef struct cpu_type {
    unsigned int pseudostack;
    unsigned int pc;
} cpu;
typedef cpu * cpu_ptr;

cpu_ptr cpu_constructor (void);

#endif