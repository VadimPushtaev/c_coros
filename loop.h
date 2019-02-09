#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

struct loop;
struct task;

typedef void (*task_code)();

struct loop* loop_get();
void yield();
void loop_create_task(struct loop*, task_code);
void loop_init(struct loop*);
void loop_run(struct loop*);

struct loop {
    ucontext_t context;
    unsigned char number_of_tasks;
    unsigned char current_task;
    struct task* tasks[10];
};

struct task {
    ucontext_t context;
    ucontext_t exit_context;
    bool started;
    bool stopped;
    task_code code;
};
