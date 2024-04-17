#include <stddef.h>
#include <stdint.h>

#include "gdt.h"
#include "print.h"
#include "idt.h"
#include "utils.h"

void kernel_main(void)
{
	terminal_initialize();
	print_texts();
	gdt_setup();
	idt_setup();

	// 8 of master PIC interrupt numbers
	// and 8 of slave PIC interrupt numbers
	// right after 0-31 interrupt numbers reserved for processor
	PIC_remap(0x20, 0x28);

	// Set RTC freq
	uint8_t rate = 15;
	rate &= 0x0F;
	set_interrupts(false);
	outb(0x70, 0x8A);
	char prev = inb(0x71);
	outb(0x70, 0x8A);
	outb(0x71, (prev & 0xF0) | rate);
	set_interrupts(true);

	// Set up RTC periodic interrupts
	set_interrupts(false);
	outb(0x70, 0x8B);
	prev = inb(0x71);
	outb(0x70, 0x8B);
	outb(0x71, prev | 0x44);	
	set_interrupts(true);
	// Change to binary date format ^^

	// Clear IRQ 8 mask
	//outb(0xA1, inb(0xA1) & ~(1 << 0));
	outb(0xA1, ~(1 << 0));
	outb(0x21, ~(1 << 2));

	outb(0x70, 0x0C);
	unsigned short v = inb(0x71);
	terminal_newline();
	display_date();

	__asm__("int $21");
	__asm__("int $10");
	__asm__("int $9");
}
