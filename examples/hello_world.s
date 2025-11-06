    .globl      main            
    .text                       
main:
    pushq       %rbp            
    movq        %rsp, %rbp      
    subq        $128, %rsp      
    movl        $72, %edi       
    call        putchar         
    movl        %eax, -8(%rbp)  
    movl        $101, %edi      
    call        putchar         
    movl        %eax, -16(%rbp) 
    movl        $108, %edi      
    call        putchar         
    movl        %eax, -24(%rbp) 
    movl        $108, %edi      
    call        putchar         
    movl        %eax, -32(%rbp) 
    movl        $111, %edi      
    call        putchar         
    movl        %eax, -40(%rbp) 
    movl        $44, %edi       
    call        putchar         
    movl        %eax, -48(%rbp) 
    movl        $32, %edi       
    call        putchar         
    movl        %eax, -56(%rbp) 
    movl        $87, %edi       
    call        putchar         
    movl        %eax, -64(%rbp) 
    movl        $111, %edi      
    call        putchar         
    movl        %eax, -72(%rbp) 
    movl        $114, %edi      
    call        putchar         
    movl        %eax, -80(%rbp) 
    movl        $108, %edi      
    call        putchar         
    movl        %eax, -88(%rbp) 
    movl        $100, %edi      
    call        putchar         
    movl        %eax, -96(%rbp) 
    movl        $33, %edi       
    call        putchar         
    movl        %eax, -104(%rbp)
    movl        $10, %edi       
    call        putchar         
    movl        %eax, -112(%rbp)
    movl        $0, %eax        
    movq        %rbp, %rsp      
    popq        %rbp            
    ret                         

.section .note.GNU-stack,"",@progbits
