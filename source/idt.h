#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "print.h"
#include "utils.h"

extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_4();
extern void isr_5();
extern void isr_6();
extern void isr_7();
extern void isr_8();
extern void isr_9();
extern void isr_10();
extern void isr_11();
extern void isr_12();
extern void isr_13();
extern void isr_14();
extern void isr_15();
extern void isr_16();
extern void isr_17();
extern void isr_18();
extern void isr_19();
extern void isr_20();
extern void isr_21();
extern void isr_22();
extern void isr_23();
extern void isr_24();
extern void isr_25();
extern void isr_26();
extern void isr_27();
extern void isr_28();
extern void isr_29();
extern void isr_30();
extern void isr_31();

// Interrupt Gate Entry
struct idt_int_gate 
{
	uint32_t offset;
	uint16_t segment_selector;
	bool is_present;
	int permission_level;
	bool is_32_bit;
};

struct idt_int_gate idt_int_gate_create(
		uint16_t segment_selector, 
		uint32_t offset) 
{
	struct idt_int_gate new_gate;

	new_gate.segment_selector = segment_selector;
	new_gate.offset = offset;

	new_gate.is_present = true;
	new_gate.permission_level = 0;
	new_gate.is_32_bit = true;

	return new_gate;
}

uint64_t idt_int_gate_encode(struct idt_int_gate gate)
{
	uint64_t gate_encoded = 0;

	// Insert offset
	gate_encoded |= ((((uint64_t) gate.offset) & 0xFFFF0000) << 32);
	gate_encoded |= ((((uint64_t) gate.offset) & 0x0000FFFF));

	// Insert segment selector
	gate_encoded |= ((((uint64_t) gate.segment_selector)) << 16);

	// Insert P(resent) bit
	gate_encoded |= ((((uint64_t) gate.is_present)) << 47);

	// Inesrt DPL
	gate_encoded |= ((((uint64_t) gate.permission_level)) << 45);

	// Insert D (size of gate)
	gate_encoded |= ((((uint64_t) gate.is_32_bit)) << 43);
	
	// Insert some magic bits
	uint8_t magic_bits = 0x30;
	gate_encoded |= ((((uint64_t) magic_bits)) << 37);

	return gate_encoded;
}

struct __attribute__((__packed__)) idt_table
{
	// Size in bytes minus one!
	uint16_t size_in_bytes_m1;
	uint64_t *dest_pointer;
	struct idt_int_gate gates[256];
};

void apply_idt_table(struct idt_table table)
{
	int entries_count = (table.size_in_bytes_m1 + 1) / 8;
	for (int i = 0; i < entries_count; i++)
	{
		uint64_t entry_encoded = idt_int_gate_encode(table.gates[i]);
		uint64_t *dest_address = table.dest_pointer + i;
		*dest_address = entry_encoded;
	}
	
	__asm__("lidt (%0)": :"r"(&table));
}

void default_int_handler()
{
	__asm__("iretl");
}

void clock_int_handler()
{
	printf("\n");
	set_interrupts(false);

	int old_row = terminal_row;
	int old_col = terminal_column;
	terminal_row = 0;
	terminal_column = 0;
	display_date();
	terminal_row = old_row;
	terminal_column = old_col;

	// Zero CMOS C register
	// Select C register
	outb(0x70, 0x0c);
	// Read C register contents (which resets it, so RTC can send next requests)
	inb(0x71);

	// Send EOI to slave PIC (because IRQ 8 from RTC belongs to this PIC)
	outb(0xA0, 0x20);
	// Send EOI to master PIC (because master also needs to acknowledge
	// handling of interrupt even if it came from slave)
	outb(0x20, 0x20);

	set_interrupts(true);

	printf("Updated!");
}

void idt_setup()
{
	struct idt_table table;

	table.size_in_bytes_m1 = 256 * 8 - 1;
	table.dest_pointer = (uint64_t*)0x500000;

	// Empty ISR-s, for the sake of completeness
	for (int i = 0; i < 256; i++)
	{
		struct idt_int_gate new_gate
			= idt_int_gate_create(0x8, (intptr_t)default_int_handler);
		table.gates[i] = new_gate;
	}

	// Clock ISR
	table.gates[0x28] = idt_int_gate_create(0x8, (intptr_t)clock_int_handler);

	// 0-31 ISR-s
	table.gates[0] = idt_int_gate_create(0x8, (intptr_t)isr_0);	
	table.gates[1] = idt_int_gate_create(0x8, (intptr_t)isr_1);
	table.gates[2] = idt_int_gate_create(0x8, (intptr_t)isr_2);
	table.gates[3] = idt_int_gate_create(0x8, (intptr_t)isr_3);
	table.gates[4] = idt_int_gate_create(0x8, (intptr_t)isr_4);
	table.gates[5] = idt_int_gate_create(0x8, (intptr_t)isr_5);
	table.gates[6] = idt_int_gate_create(0x8, (intptr_t)isr_6);
	table.gates[7] = idt_int_gate_create(0x8, (intptr_t)isr_7);
	table.gates[8] = idt_int_gate_create(0x8, (intptr_t)isr_8);
	table.gates[9] = idt_int_gate_create(0x8, (intptr_t)isr_9);
	table.gates[10] = idt_int_gate_create(0x8, (intptr_t)isr_10);
	table.gates[11] = idt_int_gate_create(0x8, (intptr_t)isr_11);
	table.gates[12] = idt_int_gate_create(0x8, (intptr_t)isr_12);
	table.gates[13] = idt_int_gate_create(0x8, (intptr_t)isr_13);
	table.gates[14] = idt_int_gate_create(0x8, (intptr_t)isr_14);
	table.gates[15] = idt_int_gate_create(0x8, (intptr_t)isr_15);
	table.gates[16] = idt_int_gate_create(0x8, (intptr_t)isr_16);
	table.gates[17] = idt_int_gate_create(0x8, (intptr_t)isr_17);
	table.gates[18] = idt_int_gate_create(0x8, (intptr_t)isr_18);
	table.gates[19] = idt_int_gate_create(0x8, (intptr_t)isr_19);
	table.gates[20] = idt_int_gate_create(0x8, (intptr_t)isr_20);
	table.gates[21] = idt_int_gate_create(0x8, (intptr_t)isr_21);
	table.gates[22] = idt_int_gate_create(0x8, (intptr_t)isr_22);
	table.gates[23] = idt_int_gate_create(0x8, (intptr_t)isr_23);
	table.gates[24] = idt_int_gate_create(0x8, (intptr_t)isr_24);
	table.gates[25] = idt_int_gate_create(0x8, (intptr_t)isr_25);
	table.gates[26] = idt_int_gate_create(0x8, (intptr_t)isr_26);
	table.gates[27] = idt_int_gate_create(0x8, (intptr_t)isr_27);
	table.gates[28] = idt_int_gate_create(0x8, (intptr_t)isr_28);
	table.gates[29] = idt_int_gate_create(0x8, (intptr_t)isr_29);
	table.gates[30] = idt_int_gate_create(0x8, (intptr_t)isr_30);
	table.gates[31] = idt_int_gate_create(0x8, (intptr_t)isr_31);

	apply_idt_table(table);

	__asm__ volatile("sti");
}

void isr_common_handler(int number)
{
	printf("Interrupt %d\n", number);
}
