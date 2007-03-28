# crt.asm
#	C RunTime:
#	Used for initialization of small data 
#	anchors and stack for programs compiled using 
#	Xilinx Gnu Tools. 
#	Used for initialization of user's bss area
#	All external and static variables are initialized 

/*      Vector map (Interrupts, Exceptions, Breakpoints)                 */
#	# 0x00 #		Jump to Start	(reset vector)
#	# 0x04 #		nop 
#	# 0x08 #		Imm instr for soft exception address [Hi halfword]
#	# 0x0c #		Jump to sof Exception handler        [Lo halfword]
#	# 0x10 #		Imm instr for interrupt address      [Hi halfword]
#	# 0x14 #		Jump to interrupt handler            [Lo halfword]
#       # 0x18 #                nop - Reserved for breakpoint vector
#       # 0x1C #                nop - Reserved for breakpoint vector
#       # 0x20 #                Imm instr for hw exception address   [Hi halfword]
#       # 0x24 #                Jump instr to hw exception handler   [Lo halfword]                        

	.globl _start

	.align 2
	.ent _start
_start:
        bri     _start1                 # 0x00		# reset vector
        nop                             # 0x04
        nop                             # 0x08          # Reserve space for software exception vector
        nop                             # 0x0c
        nop                             # 0x10          # Reserve space for interrupt vector
        nop                             # 0x14
        nop                             # 0x18          # Reserve space for breakpoint vector
        nop                             # 0x1c
        nop                             # 0x18          # Reserve space for hw exception vector
        nop                             # 0x1c        

_start1:				/* Set the Small Data Anchors and the Stack pointer  */
	la r13, r0, _SDA_BASE_
	la r2, r0, _SDA2_BASE_
	la r1, r0, _STACK_START		# stack is at end of block-ram
		
_crtinit:				/* clear sbss */
	addi	r6,r0,__sbss_start      /* SBSS beginning	*/
	addi	r7,r0,__sbss_end	/* SBSS end		*/
	rsub	r18,r6,r7		/* Compare		*/
	blei	r18,.Lendsbss
.Lloopsbss:	
	sw	r0,r6,r0
	addi	r6,r6,4
	rsub	r18,r6,r7
	bgti	r18,.Lloopsbss
.Lendsbss:				/* clear bss */
	addi	r6,r0,__bss_start      	/* BSS beginning	*/
	addi	r7,r0,__bss_end		/* BSS end		*/
	rsub	r18,r6,r7		/* Compare		*/
	blei	r18,.Lendbss
.Lloopbss:	
	sw	r0,r6,r0
	addi	r6,r6,4
	rsub	r18,r6,r7
	bgti	r18,.Lloopbss
.Lendbss:

    	brlid	r15,main		# enter main program (ignoring parameters: r5, r6 & r7)
	nop				# fall throught to exit
        .end _start

        .globl exit                  	# exit library call 
        .ent exit        
exit:
	bri	exit
	.end exit        
