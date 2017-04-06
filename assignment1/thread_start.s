.globl thread_start

thread_start:
        # 1. Push all callee-save registers onto the current stack.
        pushq %rbx
        pushq %rbp
        pushq %r12
        pushq %r13
        pushq %r14
        pushq %r15

        # 2. Save the current stack pointer in old's thread control block.
        movq %rsp, (%rdi)

        # 3. Load the stack pointer from new's thread table entry into %rsp.
        movq (%rsi), %rsp

        # 4. Call the initial function of new.
        jmp thread_wrap
