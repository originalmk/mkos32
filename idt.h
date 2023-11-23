#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "print.h"

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
	__asm__("iret");
}

void idt_setup()
{
	uintptr_t default_isr_ptr;

	__asm__("mov $default_isr, %%eax": : :"eax");
	__asm__("mov %%eax, %0;": "=r"(default_isr_ptr):);

	uintptr_t clock_isr_ptr;

	__asm__ volatile("mov $clock_isr, %%eax;"
			"mov %%eax, %0;"
			: 
			"=r" (clock_isr_ptr)
			:
			:
			"eax"
	);

	struct idt_table table;

	table.size_in_bytes_m1 = 256 * 8 - 1;
	table.dest_pointer = (uint64_t*)0x500000;

	for (int i = 0; i < 256; i++)
	{
		struct idt_int_gate new_gate
			= idt_int_gate_create(0x8, default_isr_ptr);
		table.gates[i] = new_gate;
	}
	table.gates[0x70] = idt_int_gate_create(0x8, clock_isr_ptr);

	apply_idt_table(table);

	terminal_newline();
	terminal_writenumpad(default_isr_ptr, 16, 16);

	__asm__ volatile("sti");
}
