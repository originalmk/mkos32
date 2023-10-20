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
/* 31, bo będą 4 deskryptory po 8 bajtów, a przekazujemy wielkość - 1*/
// TODO
gdtr:	.word 23
/* Na 4 MiB daję bazę dla GDT, czyli tam gdzie zaczynają się dane kernela */
gdtb:	.long 0x400000

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

// Domyślne ustawienia z GRUB-a są takie, że pomijają segmentację!
// Dodam w ramach segmentu danych kernela, czyli u mnie 4-8 MiB

mov gdtb, %eax
// 1. Pusty (null) deskryptor
movl $0, (%eax)
movl $0, 4(%eax)
// 2. Deskryptor kodu jądra
movl $0x000003FF, 8(%eax)
movl $0x00c09800, 12(%eax)
// 3. Deskryptor danych jądra
movl $0x000003FF, 16(%eax)
movl $0x00C09240, 20(%eax)
// 4. Deskryptor kodu użytkownika
// 6. Load GDT
lgdt (gdtr)
// 7. Refresh registers
mov %cs, 8(%eax)
mov %ds, 16(%eax)
mov %es, 16(%eax)
mov %fs, 16(%eax)
mov %gs, 16(%eax)
mov %ss, 16(%eax)

// Wpisywane jest i tak pod %ds:64
movb $69, (64)

/* Ustawienie ESP na wierzchołek stosu */
mov $stack_top, %esp

/* Tutaj jakieś ustawienia inicjalizacyjne trzeba zrobić normalnie */
//mov $69, %esi
//mov $_start, %edi

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
