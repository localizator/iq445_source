.section .text.start

.equ MODE_USR       ,0x10
.equ MODE_FIQ       ,0x11
.equ MODE_IRQ       ,0x12
.equ MODE_SVC       ,0x13
.equ MODE_MON       ,0x16
.equ MODE_ABT       ,0x17
.equ MODE_UNDEF     ,0x1B
.equ MODE_SYS       ,0x1F
.equ I_BIT          ,0x80
.equ F_BIT          ,0x40
.equ INT_BIT        ,0xC0

.extern sys_stack
.extern sys_stack_sz

.globl _start
_start : 
    b resethandler
bss_start:
    .word _bss_start
bss_end:
    .word _bss_end
stack :
    .long sys_stack
stacksz:
    .long sys_stack_sz

resethandler :
    MOV r0, #0
    MOV r1, #0
    MOV r2, #0
    MOV r3, #0
    MOV r4, #0
    MOV r5, #0
    MOV r6, #0
    MOV r7, #0
    MOV r8, #0
    MOV r9, #0
    MOV r10, #0
    MOV r11, #0
    MOV r12, #0
    MOV sp, #0
    MOV lr, #0

    /* set the cpu to SVC32 mode */
    MRS	r0,cpsr
    BIC	r0,r0,#0x1f
    ORR	r0,r0,#0xd3
    MSR	cpsr,r0

    /* disable interrupt */
    MRS r0, cpsr
    MOV r1, #INT_BIT
    ORR r0, r0, r1
    MSR cpsr_cxsf, r0    
      
    /* enable I+Z bits */
    MRC p15, 0, ip, c1, c0, 0
    ORR ip, ip, #0x1800   /* I+Z bits */
    MCR p15, 0, ip, c1, c0, 0

clear_bss :
    LDR r0, bss_start  /* find start of bss segment */
    LDR r1, bss_end    /* stop here */
    MOV r2, #0x00000000 /* clear */
    
    CMP r0, r1
    BEQ setup_stk
    
    /*  clear loop... */
clbss_l : 
    STR r2, [r0]    
    ADD r0, r0, #4
    CMP r0, r1
    BNE clbss_l

setup_stk :
    /* setup stack */
    LDR r0, stack
    LDR r1, stacksz

    /* buffer overflow detect pattern */
    LDR r2, =0xDEADBEFF
    STR r2, [r0]

    LDR r1, [r1]
    SUB r1, r1, #0x04
    ADD r1, r0, r1
    
    MOV sp, r1
    
entry :
    B   main

.globl jump   
jump:
    MOV r4, r1   /* r4 argument */
    MOV r5, r2   /* r5 argument */
    MOV pc, r0   /* jump to addr */

.globl apmcu_icache_invalidate
apmcu_icache_invalidate:
    MOV r0, #0
    MCR p15, 0, r0, c7, c5, 0
    BX  lr 

.globl apmcu_dcache_invalidate
apmcu_dcache_invalidate:
    MRC p15, 2, r3, c0, c0, 0
    BIC r3, r3, #0xf         
    MCR p15, 2, r3, c0, c0, 0
    MRC p15, 1, r3, c0, c0, 0

    MOV r1, #0x1
    RSB r1, r1, r1, LSL #15
    AND r3, r1, r3, LSR #13  
    MOV r0, #0               

ci_way_loop:
    MOV r1, #0               

ci_set_loop:
    MOV r2, #0
    ORR r2, r0, LSL #30                
    ORR r2, r1, LSL #5       
    MCR p15, 0, r2, c7, c6, 2
    ADD r1, r1, #1           
    CMP r1, r3               
    BLE ci_set_loop          
    ADD r0, r0, #1           
    CMP r0, #4               
    BLT ci_way_loop          
    BX  lr


.globl apmcu_dcache_clean_invalidate
apmcu_dcache_clean_invalidate:
    MRC p15, 2, r3, c0, c0, 0 
    BIC r3, r3, #0xf
    MCR p15, 2, r3, c0, c0, 0             
    MRC p15, 1, r3, c0, c0, 0             
    
    MOV r1, #0x1
    RSB r1, r1, r1, LSL #15
    AND r3, r1, r3, LSR #13               
    MOV r0, #0                            
cci_way_loop:
    MOV r1, #0                            
cci_set_loop:
    MOV r2, #0
    ORR r2, r0, LSL #30                
    ORR r2, r1, LSL #5                    
    MCR p15, 0, r2, c7, c14, 2            
    ADD r1, r1, #1                        
    CMP r1, r3                            
    BLE cci_set_loop                      
    ADD r0, r0, #1                        
    CMP r0, #4                            
    BLT cci_way_loop                      
    BX  lr

.globl apmcu_dsb
apmcu_dsb:
    MOV r0, #0
    MCR p15, 0, r0, c7, c10, 4
    BX  lr

.globl apmcu_disable_dcache   
apmcu_disable_dcache:

    MRC p15,0,r0,c1,c0,0
    BIC r0,r0,#0x4
    MCR p15,0,r0,c1,c0,0
    BX  lr

.globl apmcu_disable_icache   
apmcu_disable_icache:
    MOV r0,#0
    MCR p15,0,r0,c7,c5,6   /* Flush entire branch target cache */
    MRC p15,0,r0,c1,c0,0
    BIC r0,r0,#0x1800      /* I+Z bits */
    MCR p15,0,r0,c1,c0,0
    BX  lr

    MCR p15,0,r0,c2,c0,0
    BX lr
 
