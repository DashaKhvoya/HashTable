section       .text

global        _ListSearch

; rdi contains list
; rsi contains pair
_ListSearch:  mov rax, [rdi]     ;Pair* data
              mov rcx, [rdi + 8] ;size_t size

              vmovdqu ymm0, [rsi];pair->key

loop_start:   or rcx, rcx
              jz exit_false

              vmovdqu ymm1, [rax]
              vpcmpeqb ymm2, ymm1, ymm0
              vpmovmskb rdx, ymm2

              cmp rdx, -1
              je exit_true

              add rax, 16
              dec rcx
              jmp loop_start

exit_false:   xor rax, rax
              ret

exit_true:    mov rdx, [rsi + 8]
              mov [rax + 8], rdx
              mov rax, 1
              ret
