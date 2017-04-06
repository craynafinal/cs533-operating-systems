.globl thread_switch

thread_switch:
        # 1. Push all callee-save registers (%rbx, %rbp, %r12-15) onto the current stack.
        pushq %rbx
        pushq %rbp
        pushq %r12
        pushq %r13
        pushq %r14
        pushq %r15

        # 2. Save the current stack pointer (%rsp) in old's thread control block.
        movq %rsp, (%rdi)

        # 3. Load the stack pointer from new's thread table entry into %rsp.
        movq (%rsi), %rsp

        # 4. Pop all callee-save registers from the new stack.
        popq %r15
        popq %r14
        popq %r13
        popq %r12
        popq %rbp
        popq %rbx

        # 5. Return.
        ret
