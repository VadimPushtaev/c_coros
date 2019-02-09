#include "loop.h"

void
coro1() {
    printf("1 - 1\n");
    yield();
    printf("1 - 2\n");
    yield();
    printf("1 - 3\n");
}

void
coro2() {
    printf("2 - 1\n");
    yield();
    yield();
    yield();
    yield();
    yield();
    printf("2 - 2\n");
}

void
coro3() {
    printf("3 - 1\n");
    yield();
    printf("3 - 2\n");
}

void
main_coro() {
    struct loop *loop = loop_get();

    printf("MAIN - 1\n");
    loop_create_task(loop, coro1);
    yield();
    loop_create_task(loop, coro2);
    loop_create_task(loop, coro3);
    printf("MAIN - 2\n");
}

int
main(int argc, char *argv[])
{
    struct loop loop;
    loop_init(&loop);

    loop_create_task(&loop, main_coro);
    loop_run(&loop);

    return 0;
}
