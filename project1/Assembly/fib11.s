!============================================================
! CS-2200 Homework 1
!
! Please do not change mains functionality, 
! except to change the argument for fibonacci or to meet your 
! calling convention
!============================================================

main:       la $sp, stack				! load address of stack label into $sp
            lw $sp, 0x0($sp)			! load value of the stack (0x4000) into $sp
            la $at, fibonacci	        ! load address of fibonacci label into $at
            addi $a0, $zero, 11 	        ! $a0 = 8, the fibonacci argument
            jalr $at, $ra				! jump to fibonacci, set $ra to return addr
            halt						! when we return, just halt


fibonacci:  add $fp, $sp, $zero 		! set frame pointer to stack pointer
			addi $sp, $sp, 4			! move stack pointer to top of stack

			sw $ra, 0x0($fp)			! save previous return address on stack
			add $t0, $a0, $zero			! save original argument (n) in $t0
			addi $a0, $a0, -1			! store new argument n - 1 in $a0
			sw $a0, 0x1($fp)			! save n - 1 on stack
			addi $t1, $a0, -1			! store new argument $a0 - 1 in $t1 (n - 2)
			sw $t1, 0x2($fp)			! save n - 2 on stack

			addi $t2, $zero, 1			! set $t2 to 1 to check if $a0 == 1
			beq $t0, $zero, if_zero		! if $t0 (n) == $a1 (0) then go to if_zero
			beq $t0, $t2, if_one		! if $t0 (n) == $a2 (1) then go to if_one

			! else:
			jalr $at, $ra				! recurse fib(n - 1)
			lw $ra, 0x0($fp)			! restore return address
			sw $v0, 0x3($fp)			! save first result on stack

			lw $a0, 0x2($fp)			! load n - 2
			jalr $at, $ra				! recurse fib(n - 2)
			lw $ra, 0x0($fp)			! restore return address

			lw $t1, 0x3($fp)			! load first result into $t1 from stack
			add $v0, $t1, $v0			! store fib(n - 1) + fib(n - 2) in $v0 (second result is already in $v0)
			addi $sp, $sp, -4			! move stack pointer back to previous call
			addi $fp, $fp, -4			! move frame pointer back to previous call
			jalr $ra, $zero 			! return


if_zero:	addi $v0, $zero, 0			! set return value to 0
			addi $sp, $sp, -4			! move stack pointer back to previous call
			addi $fp, $fp, -4			! move frame pointer back to previous call
			jalr $ra, $zero 			! return

if_one:		addi $v0, $zero, 1	 		! set return value to 1
			addi $sp, $sp, -4			! move stack pointer back to previous call
			addi $fp, $fp, -4			! move frame pointer back to previous call
			jalr $ra, $zero 			! return

stack:	    .word 0x4000				! the stack begins here

