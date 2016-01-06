### boot.S

```
#include <config.h>
#define MIN_STACK_FRAME 0x60
#define STACK_SIZE 8192

#ifdef CONFIG_SPARCv8
int main() {
    [partCtrlTabPtr] = %g1; //TODO what is g1;

    %fp = stack;
    %fp += STACK_SIZE - 8;
    %o0 = STACK_SIZE - 8;
    %sp = %fp - MIN_STACK_FRAME;
    call MainLdr;

    %g1 = [partCtrlTabPtr]; //load back g1;
    return;
    while (1);
}
```