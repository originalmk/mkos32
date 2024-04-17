#pragma once

/* Taken from OSDEV WIKI */
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */
/* End of taken */

void breakpoint()
{
	__asm__("xchg %bx, %bx");
}

void set_interrupts(bool enabled)
{
	if (enabled)
	{
		__asm__ volatile("sti");
	}
	else
	{
		__asm__ volatile("cli");
	}	
}

void outb(uint8_t port_number, uint8_t new_value)
{
	__asm__ volatile(
		"outb %[new_value], %[port_number]"
		:
		:
		[port_number] "d" ((unsigned short)port_number),
		[new_value] "a" (new_value)
	);	
}

uint8_t inb(uint8_t port_number)
{
	uint8_t out_value;

	__asm__ volatile(
		"in %[port_number], %[out_value];"
		:
		[out_value] "=a" (out_value)
		:
		[port_number] "d" ((unsigned short)port_number)
	);

	return out_value;
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

uint8_t cmos_register_value(uint16_t register_number)
{
	outb(0x70, register_number);
	return inb(0x71);
}


void PIC_remap(int offset1, int offset2)
{
	unsigned char a1, a2;
 
	a1 = inb(PIC1_DATA);                        // save masks
	a2 = inb(PIC2_DATA);
 
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
 
	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();
 
	outb(PIC1_DATA, a1);   // restore saved masks.
	outb(PIC2_DATA, a2);
}

void display_date()
{
	uint8_t year = cmos_register_value(0x09);
	uint8_t month = cmos_register_value(0x08);
	uint8_t day = cmos_register_value(0x07);
	uint8_t hours = cmos_register_value(0x04);
	uint8_t minutes = cmos_register_value(0x02);
	uint8_t seconds = cmos_register_value(0x00);

	terminal_writenumpad(year, 10, 2);
	terminal_writestring("-");
	terminal_writenumpad(month, 10, 2);
	terminal_writestring("-");
	terminal_writenumpad(day, 10, 2);
	terminal_writestring(" ");
	terminal_writenumpad(hours, 10, 2);
	terminal_writestring(":");
	terminal_writenumpad(minutes, 10, 2);
	terminal_writestring(":");
	terminal_writenumpad(seconds, 10, 2);
	terminal_newline();
}
