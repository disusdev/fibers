.code
get_context proc
  mov r9, [rsp]
  mov qword ptr [rcx], r9
  
  lea r9, [rsp + 8]
  mov qword ptr [rcx + 8], r9
  
  mov qword ptr [rcx + 8*2], rbx
  mov qword ptr [rcx + 8*3], rbp
  mov qword ptr [rcx + 8*4], r12
  mov qword ptr [rcx + 8*5], r13
  mov qword ptr [rcx + 8*6], r14
  mov qword ptr [rcx + 8*7], r15

  movups [rcx + 8*10+16*0], xmm6
  movups [rcx + 8*10+16*1], xmm7
  movups [rcx + 8*10+16*2], xmm8
  movups [rcx + 8*10+16*3], xmm9
  movups [rcx + 8*10+16*4], xmm10
  movups [rcx + 8*10+16*5], xmm11
  movups [rcx + 8*10+16*6], xmm12
  movups [rcx + 8*10+16*7], xmm13
  movups [rcx + 8*10+16*8], xmm14
  movups [rcx + 8*10+16*9], xmm15
  
  ret
get_context endp

set_context proc
  mov r9, qword ptr [rcx] ; rip
  mov rsp, qword ptr [rcx + 8] ; rsp

  mov rbx, qword ptr [rcx + 8*2]
  mov rbp, qword ptr [rcx + 8*3]
  mov r12, qword ptr [rcx + 8*4]
  mov r13, qword ptr [rcx + 8*5]
  mov r14, qword ptr [rcx + 8*6]
  mov r15, qword ptr [rcx + 8*7]

  movups xmm6 , [rcx + 8*10+16*0]
  movups xmm7 , [rcx + 8*10+16*1]
  movups xmm8 , [rcx + 8*10+16*2]
  movups xmm9 , [rcx + 8*10+16*3]
  movups xmm10, [rcx + 8*10+16*4]
  movups xmm11, [rcx + 8*10+16*5]
  movups xmm12, [rcx + 8*10+16*6]
  movups xmm13, [rcx + 8*10+16*7]
  movups xmm14, [rcx + 8*10+16*8]
  movups xmm15, [rcx + 8*10+16*9]

  mov rdx, qword ptr [rcx + 8*10+16*10+8*1]
  mov r8,  qword ptr [rcx + 8*10+16*10+8*2]
  mov rcx, qword ptr [rcx + 8*10+16*10+8*0]

  ;mov rcx, rsp

  push r9

  ret
set_context endp

swap_context proc
  mov r9, [rsp]
  mov qword ptr [rcx], r9
  
  lea r9, [rsp + 8]
  mov qword ptr [rcx + 8], r9
  
  mov qword ptr [rcx + 8*2], rbx
  mov qword ptr [rcx + 8*3], rbp
  mov qword ptr [rcx + 8*4], r12
  mov qword ptr [rcx + 8*5], r13
  mov qword ptr [rcx + 8*6], r14
  mov qword ptr [rcx + 8*7], r15

  movups [rcx + 8*10+16*0], xmm6
  movups [rcx + 8*10+16*1], xmm7
  movups [rcx + 8*10+16*2], xmm8
  movups [rcx + 8*10+16*3], xmm9
  movups [rcx + 8*10+16*4], xmm10
  movups [rcx + 8*10+16*5], xmm11
  movups [rcx + 8*10+16*6], xmm12
  movups [rcx + 8*10+16*7], xmm13
  movups [rcx + 8*10+16*8], xmm14
  movups [rcx + 8*10+16*9], xmm15

  mov r9, qword ptr [rdx] ; rip
  mov rsp, qword ptr [rdx + 8] ; rsp

  mov rbx, qword ptr [rdx + 8*2]
  mov rbp, qword ptr [rdx + 8*3]
  mov r12, qword ptr [rdx + 8*4]
  mov r13, qword ptr [rdx + 8*5]
  mov r14, qword ptr [rdx + 8*6]
  mov r15, qword ptr [rdx + 8*7]

  movups xmm6 , [rdx + 8*10+16*0]
  movups xmm7 , [rdx + 8*10+16*1]
  movups xmm8 , [rdx + 8*10+16*2]
  movups xmm9 , [rdx + 8*10+16*3]
  movups xmm10, [rdx + 8*10+16*4]
  movups xmm11, [rdx + 8*10+16*5]
  movups xmm12, [rdx + 8*10+16*6]
  movups xmm13, [rdx + 8*10+16*7]
  movups xmm14, [rdx + 8*10+16*8]
  movups xmm15, [rdx + 8*10+16*9]

  mov rcx, qword ptr [rdx + 8*10+16*10+8*0]
  mov r8,  qword ptr [rdx + 8*10+16*10+8*2]
  mov rdx, qword ptr [rdx + 8*10+16*10+8*1]

  push r9

  ret
swap_context endp
end