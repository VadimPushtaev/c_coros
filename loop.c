#include "loop.h"

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define STACK_SIZE 1024 * 1024
#define MINI_STACK_SIZE 16 * 1024

static void *
_loop_allocate_stack(int stack_size)
{
    void *stack = malloc(stack_size);
    stack_t ss;
    ss.ss_sp = stack;
    ss.ss_size = stack_size;
    ss.ss_flags = 0;
    sigaltstack(&ss, NULL);
    return stack;
}

static void
_loop_stop_task(int i)
{
    loop_get()->tasks[i]->stopped = true;
}

static void
_set_task_context(struct loop *loop, ucontext_t *context, ucontext_t *uc_link, int stack_size)
{
    getcontext(context);
    context->uc_link = uc_link;
    context->uc_stack.ss_sp = _loop_allocate_stack(stack_size);
    context->uc_stack.ss_size = stack_size;
}

void
yield()
{
    struct loop *loop = loop_get();
    struct task *task = loop->tasks[loop->current_task];

    swapcontext(&task->context, &loop->context);
}

void
loop_create_task(struct loop *loop, task_code code)
{
    struct task* task = (struct task*)malloc(sizeof(struct task));
    loop->tasks[loop->number_of_tasks] = task;
    task->started = false;
    task->stopped = false;
    task->code = code;

    loop->number_of_tasks++;
}

void
loop_init(struct loop *loop)
{
    loop->number_of_tasks = 0;
}

void
loop_start_task(struct loop *loop, unsigned char i)
{
    struct task *task = loop->tasks[i];
    task->started = true;

    _set_task_context(loop, &task->exit_context, &loop->context, MINI_STACK_SIZE);
    makecontext(&task->exit_context, (void (*)(void))_loop_stop_task, 1, (int)i);

    _set_task_context(loop, &task->context, &task->exit_context, STACK_SIZE);
    makecontext(&task->context, task->code, 0);

    loop->current_task = i;
    setcontext(&task->context);
}

void
loop_resume_task(struct loop *loop, unsigned char i)
{
    struct task *task = loop->tasks[i];

    loop->current_task = i;
    swapcontext(&loop->context, &task->context);
}

struct loop *_loop;
struct loop *
loop_get()
{
    return _loop;
}

void
loop_run(struct loop *loop)
{
    _loop = loop;
    struct task *task;

    getcontext(&loop->context);
    while (1) {
        bool all_stopped = true;
        for (int i=0; i < loop->number_of_tasks; i++) {
            task = loop->tasks[i];
            if (task->started && !task->stopped) {
                all_stopped = false;
                loop_resume_task(loop, i);
            }
            if (!task->started) {
                all_stopped = false;
                loop_start_task(loop, i);
            }
        }
        if (all_stopped) {
            break;
        }
    }
}
