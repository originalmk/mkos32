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
stack_bottom:
/* Stos o wielkości 16 KB */
.skip 16834
stack_top:

.section .data
/* 23, bo będą 3 deskryptory po 8 bajtów, a przekazujemy wielkość - 1*/
gdtr:	.word 23
		.long 0

/*
	Oznaczenie startu, tu zaczyna się kod kernela!
*/
.section .text
.global _start
.type _start, @function
_start:
/* Tutaj podobno jest tryb chroniony już, jednak zastanawia czy faktycznie
 * bootloader ustawia za nas segmentacje pamięci?
 * EDIT: Ustawia, ale pod własne potrzeby, więc i tak trzeba zmienić
 * Ustawię zatem najprostszy model segmentacji */

// 1. Pusty (null) deskryptor
mov $0, %eax
movq $0, (%eax)
// 2. Deskryptor kodu jądra


/* Ustawienie ESP na wierzchołek stosu */
mov $stack_top, %esp

/* Tutaj jakieś ustawienia inicjalizacyjne trzeba zrobić normalnie */

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
