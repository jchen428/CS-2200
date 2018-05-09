		addi $a2, $zero, 3
		addi $a1, $zero, 10
		addi $a0, $zero, 0
		
loop1:	addi $a0, $a0, 1
		beq $a0, $a1, done1
		jalr $a2, $zero

done1:	sw $zero, 0($zero)
		addi $a2, $zero, 8

loop2:	lw $a3, 0($zero)
		addi $a3, $a3, 1
		sw $a3, 0($zero)
		beq $a3, $a1, done2
		jalr $a2, $zero

done2:	add $zero, $zero, $zero		! "halt"