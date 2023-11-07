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
.skip 16384
stack_top:

.section .data

/*
	Oznaczenie startu, tu zaczyna się kod kernela!
*/
.section .text
.global _start
.type _start, @function
_start:
/* Ustawienie ESP na wierzchołek stosu */
mov $stack_top, %esp

/* Wywołanie kernela */
call kernel_main

/* Wieczne oczekiwanie po zakończeniu kodu kernela */
	cli
1:	hlt
	/* Skacze do (lokalnej) labelki o nazwie 1, wstecz */
	jmp 1b

/* Ustawienie wielkości funkcji start, co pozwala uniknąć błędów z
 * debuggowaniem */
.size _start, . - _start
