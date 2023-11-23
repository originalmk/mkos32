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
/* Stos o wielkości 16 KB */
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
.global default_isr
.type default_isr, @function
default_isr:
cli
push %eax
mov $0x0F41, %eax
add int_count, %eax
mov %ax, 0xb8000

incl int_count

cmpl $0x5, int_count
jne end

movl $0x0, int_count

end:
xchg %bx, %bx

mov $0x20, %al
outb %al, $0x20

mov $0x20, %al
outb %al, $0xA0

pop %eax
sti
iretl

.global clock_isr
.type clock_isr, @function
clock_isr:
cli
push %eax

//xchg %bx, %bx

mov $0x0F41, %ax
add clock_count, %ax
mov clock_pos, %edx
shl $1, %edx
add $0xb8002, %edx
movw %ax, (%edx)

incl clock_count

cmpl $0x1F, clock_count
jne end2

movl $0x0, clock_count

end2:

incl clock_pos
cmpl $0x7CF, clock_pos
jne end3

movl $0x0, clock_pos

end3:

//xchg %bx, %bx

mov $0x0c, %al
outb %al, $0x70
inb $0x71

mov $0x20, %al
outb %al, $0x20

mov $0x20, %al
outb %al, $0xA0
// EOI

pop %eax
sti
iretl

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
