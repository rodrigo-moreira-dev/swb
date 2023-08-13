.text

# function f1 pi1
# pi1: %edi
.globl f1
f1:

pushq %rbp
movq %rsp, %rbp

# vi1: -4(%rbp)
# va2: -44(%rbp)
# vi3: -48(%rbp)
# pi1: -52(%rbp)
subq $64, %rsp

# vi1 = ci3
movl $3, -4(%rbp)

# vi3 = ci4
movl $4, -48(%rbp)

# _if vi1 ne vi3 
movl -48(%rbp), %eax
cmpl %eax, -4(%rbp) 
je end_if1

# vi1 = ci0
movl $0, -4(%rbp)

end_if1:

# _if vi1 lt vi3 
movl -48(%rbp), %eax
cmpl %eax, -4(%rbp) 
jge end_if2

# get va2 index ci8 to vi1
movl -12(%rbp), %eax
movl %eax, -4(%rbp)

end_if2:

# _if vi3 le vi1 
movl -4(%rbp), %eax
cmpl %eax, -48(%rbp) 
jg end_if3

# return vi1
movl -4(%rbp), %eax
jmp fim_f1

end_if3:

# _if pi1 le ci10 
# var = pi1
cmpl $10, %edi 
jg end_if4

# return ci0
movl $0, %eax
jmp fim_f1

end_if4:

# _if ci-10 gt ci10 
movl $-10, %eax
cmpl $10, %eax 
jle end_if5

# get va2 index ci2 to vi3
movl -36(%rbp), %eax
movl %eax, -48(%rbp)

end_if5:

# return ci-1
movl $-1, %eax
jmp fim_f1

fim_f1:
leave
ret

# function f2 pa1
# pa1: %rdi
.globl f2
f2:

pushq %rbp
movq %rsp, %rbp

# vr1: -4(%rbp)
# vi2: -8(%rbp)
# va3: -128(%rbp)
# pa1: -136(%rbp)
subq $144, %rsp

# Variaveis locais de registrador
# Guarda os valores previos dos registradores
# vr1: %r12d
movl %r12d, -4(%rbp) # guarda o valor previo do registrador 

# vr1 = ci1
movl $1, %r12d

# vi2 = call f1 vr1 va3
movq %rdi, -136(%rbp) # guarda o valor do parametro pa1

movl %r12d, %edi
leaq -128(%rbp), %rsi
call f1
movl %eax, -8(%rbp)

movq -136(%rbp), %rdi # resgata o valor do parametro pa1

# vi2 = call f1 ci5 pa1
movq %rdi, -136(%rbp) # guarda o valor do parametro pa1

movl $5, %edi
movq -136(%rbp), %rsi
call f1
movl %eax, -8(%rbp)

movq -136(%rbp), %rdi # resgata o valor do parametro pa1

# get pa1 index ci3 to vr1
movabs $3, %rax
imulq $4, %rax
addq %rdi, %rax
movl (%rax), %eax
movl %eax, %r12d

# get pa1 index ci0 to vi2
movabs $0, %rax
imulq $4, %rax
addq %rdi, %rax
movl (%rax), %eax
movl %eax, -8(%rbp)

# get va3 index ci5 to vr1
movl -108(%rbp), %eax
movl %eax, %r12d

# get va3 index ci11 to vi2
movl -84(%rbp), %eax
movl %eax, -8(%rbp)

# return vi2
movl -8(%rbp), %eax
jmp fim_f2

# Restaura os valores previos dos registradores callee-saved
movl -4(%rbp), %r12d # resgata o valor previo do registrador 

fim_f2:
leave
ret

# function f3 pa1 pi2 pa3
# pa1: %rdi
# pi2: %esi
# pa3: %rdx
.globl f3
f3:

pushq %rbp
movq %rsp, %rbp

# va1: -40(%rbp)
# va2: -120(%rbp)
# vi3: -124(%rbp)
# vr4: -128(%rbp)
# vi5: -132(%rbp)
# vr6: -136(%rbp)
# pa1: -144(%rbp)
# pi2: -148(%rbp)
# pa3: -160(%rbp)
subq $160, %rsp

# Variaveis locais de registrador
# Guarda os valores previos dos registradores
# vr4: %r12d
movl %r12d, -128(%rbp) # guarda o valor previo do registrador 
# vr6: %r13d
movl %r13d, -136(%rbp) # guarda o valor previo do registrador 

# return ci0
movl $0, %eax
jmp fim_f3

# Restaura os valores previos dos registradores callee-saved
movl -128(%rbp), %r12d # resgata o valor previo do registrador 
movl -136(%rbp), %r13d # resgata o valor previo do registrador 

fim_f3:
leave
ret

# function f4 pi1 pi2
# pi1: %edi
# pi2: %esi
.globl f4
f4:

pushq %rbp
movq %rsp, %rbp

# vi1: -4(%rbp)
# vr2: -8(%rbp)
# vi3: -12(%rbp)
# vr4: -16(%rbp)
# pi1: -20(%rbp)
# pi2: -24(%rbp)
subq $32, %rsp

# Variaveis locais de registrador
# Guarda os valores previos dos registradores
# vr2: %r12d
movl %r12d, -8(%rbp) # guarda o valor previo do registrador 
# vr4: %r13d
movl %r13d, -16(%rbp) # guarda o valor previo do registrador 

# _if pi1 eq pi2
# var = pi1
cmpl %esi, %edi 
jne end_if6

# return ci1
movl $1, %eax
jmp fim_f4

end_if6:

# _if pi1 eq vi1
# var = pi1
cmpl -4(%rbp), %edi 
jne end_if7

# return ci2
movl $2, %eax
jmp fim_f4

end_if7:

# _if pi1 ne vr2
# var = pi1
cmpl %r12d, %edi 
je end_if8

# return ci3
movl $3, %eax
jmp fim_f4

end_if8:

# _if vi1 lt pi1
cmpl %edi, -4(%rbp) 
jge end_if9

# return ci4
movl $4, %eax
jmp fim_f4

end_if9:

# _if vi1 le vi3
movl -12(%rbp), %eax
cmpl %eax, -4(%rbp) 
jg end_if10

# return ci5
movl $5, %eax
jmp fim_f4

end_if10:

# _if vi1 gt vr2
cmpl %r12d, -4(%rbp) 
jle end_if11

# return ci6
movl $6, %eax
jmp fim_f4

end_if11:

# _if vr2 ge pi1
cmpl %edi, %r12d 
jl end_if12

# return ci7
movl $7, %eax
jmp fim_f4

end_if12:

# _if vr2 gt vi1
cmpl -4(%rbp), %r12d 
jle end_if13

# return ci8
movl $8, %eax
jmp fim_f4

end_if13:

# _if vr2 le vr4
cmpl %r13d, %r12d 
jg end_if14

# return ci9
movl $9, %eax
jmp fim_f4

end_if14:

# return ci0
movl $0, %eax
jmp fim_f4

# Restaura os valores previos dos registradores callee-saved
movl -8(%rbp), %r12d # resgata o valor previo do registrador 
movl -16(%rbp), %r13d # resgata o valor previo do registrador 

fim_f4:
leave
ret

