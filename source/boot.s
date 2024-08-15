/* 
	Nagłówek Multiboot 
*/

/*  Moduły bootujące jak i sam kernel zostanie załaodwany z wyrównaniem do 4 KB
 *  stron */
.set ALIGN, 1<<0
/* Kernelowi zostaną udostępnione informacje o pamięci, które zdobył
 * bootloader. Opcjonalnie nawet mapa pamięci */
.set MEMINFO, 1<<1
/* Połączenie flag */
.set FLAGS, ALIGN | MEMINFO
/* Wartość magicznego numerka dla Multiboot 1 */
.set MAGIC, 0x1BADB002
/* Suma kontrola, która w sumie z flagami i numerkiem ma dać 0, czyli należy
 * podać ich odwrotność. */
.set CHECKSUM, -(MAGIC + FLAGS)

/* Faktyczny nagłówek multiboot z wykorzystaniem powyższych wartości */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
	Alokacja stosu
*/

.section .bss
.align 16
.global stack_top
.global stack_bottom
stack_bottom:
/* Stos o wielkości 64 KB */
.skip 65536
stack_top:

.section .data
int_count: .int 0x0
clock_count: .int 0x0
clock_pos: .int 0x0

/*
	Oznaczenie startu, tu zaczyna się kod kernela!
*/
.section .text

.macro isr number
.global isr_\number
.type isr_\number, @function
isr_\number :
pusha
push $\number

call isr_common_handler

add $4, %esp

popa
sti
iretl
.endm

isr 0
isr 1
isr 2
isr 3
isr 4
isr 5
isr 6
isr 7
isr 8
isr 9
isr 10
isr 11
isr 12
isr 13
isr 14
isr 15
isr 16
isr 17
isr 18
isr 19
isr 20
isr 21
isr 22
isr 23
isr 24
isr 25
isr 26
isr 27
isr 28
isr 29
isr 30
isr 31

.global _start
.type _start, @function
_start:
/* Ustawienie ESP na wierzchołek stosu */
mov $stack_top, %esp

/* Wywołanie kernela */
call kernel_main

/* Wieczne oczekiwanie po zakończeniu kodu kernela */
//	cli
1:	hlt
	/* Skacze do (lokalnej) labelki o nazwie 1, wstecz */
	jmp 1b

/* Ustawienie wielkości funkcji start, co pozwala uniknąć błędów z
 * debuggowaniem */
.size _start, . - _start
