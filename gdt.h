#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "print.h"

#define KERNEL_SEGMENT 0
#define USER_SEGMENT 1
#define CODE_SEGMENT 0
#define DATA_SEGMENT 1

struct gdt_entry {
	size_t base;
	size_t limit;
	bool is_present;
	int permission_level;
	int segment_type;
	bool is_executable;
	int direction_conforming;
	int readable_writable;
	bool was_accessed;
	int granularity;
	int segment_mode;
	bool is_long_mode;
};

struct gdt_entry gdt_entry_create(size_t start_address, size_t pages_size,
				  int kernel_or_user, int code_or_data)
{
	struct gdt_entry new_entry;

	new_entry.base = start_address;
	new_entry.limit = pages_size - 1;
	new_entry.is_present = true;

	if (kernel_or_user == KERNEL_SEGMENT)
	{
		new_entry.permission_level = 0;
	}
	else
	{
		new_entry.permission_level = 3;
	}

	new_entry.segment_type = 1;
	new_entry.is_executable = (code_or_data == CODE_SEGMENT);
	new_entry.direction_conforming = 0;

	if (code_or_data == CODE_SEGMENT)
	{
		new_entry.readable_writable = 0;
	}
	else
	{
		new_entry.readable_writable = 1;
	}

	new_entry.was_accessed = false;
	new_entry.granularity = 1;
	new_entry.segment_mode = 1;
	new_entry.is_long_mode = false;

	return new_entry;
}

// BB F L AA BBBBBB LLLL
// AA = P DPL(2) S E DC RW A
// F = G DB L -
uint64_t gdt_entry_encode(struct gdt_entry entry)
{
	entry.base = entry.base << 12;

	uint64_t gdt_encoded = 0;

	// Insert base
	gdt_encoded |= ((((uint64_t) entry.base) & 0xFF000000) << 32);
	gdt_encoded |= ((((uint64_t) entry.base) & 0x00FFFFFF) << 16);

	// Insert limit
	gdt_encoded |= ((uint64_t) (entry.limit & 0xF0000) << 32);
	gdt_encoded |= (entry.limit & 0x0FFFF);

	// Prepare flags
	uint64_t flags = 0;
	flags |= (entry.granularity << 3);
	flags |= (entry.segment_mode << 2);
	flags |= (entry.is_long_mode << 1);

	// Insert flags
	gdt_encoded |= (flags << 52);

	// Prepare access byte
	uint64_t access_byte = 0;
	access_byte |= (entry.is_present << 7);
	access_byte |= (entry.permission_level << 5);
	access_byte |= (entry.segment_type << 4);
	access_byte |= (entry.is_executable << 3);
	access_byte |= (entry.direction_conforming << 2);
	access_byte |= (entry.readable_writable << 1);
	access_byte |= entry.was_accessed;

	// Insert access byte
	gdt_encoded |= (access_byte << 40);

	return gdt_encoded;
}

struct __attribute__((__packed__)) gdt_table
{
	uint16_t size_in_bytes_m1;
	uint64_t *dest_pointer;
	struct gdt_entry entries[16];
};

void apply_gdt_table(struct gdt_table table)
{
	// Null entry added implicitly
	uint64_t null_entry_encoded = 0;
	*table.dest_pointer = null_entry_encoded;

	int entries_count = (table.size_in_bytes_m1 + 1) / 8;
	for (int i = 0; i < entries_count; i++)
	{
		uint64_t entry_encoded = gdt_entry_encode(table.entries[i]);
		uint64_t *dest_address = table.dest_pointer + i + 1;
		*dest_address = entry_encoded;
	}

 __asm__("lgdt (%0)": :"r"(&table));
}

void gdt_setup()
{
	struct gdt_entry kernel_code_entry =
	    gdt_entry_create(0x0, 0x100000, KERNEL_SEGMENT, CODE_SEGMENT);
	struct gdt_entry kernel_data_entry =
	    gdt_entry_create(0x0, 0x100000, KERNEL_SEGMENT, DATA_SEGMENT);
	struct gdt_entry user_code_entry =
	    gdt_entry_create(0x0, 0x100000, USER_SEGMENT, CODE_SEGMENT);
	struct gdt_entry user_data_entry =
	    gdt_entry_create(0x0, 0x100000, USER_SEGMENT, DATA_SEGMENT);

	struct gdt_table gdt_table;
	gdt_table.size_in_bytes_m1 = 5 * 8 - 1;
	gdt_table.dest_pointer = (uint64_t *) 0x400000;
	gdt_table.entries[0] = kernel_code_entry;
	gdt_table.entries[1] = kernel_data_entry;
	gdt_table.entries[2] = user_code_entry;
	gdt_table.entries[3] = user_data_entry;

	apply_gdt_table(gdt_table);
	terminal_writestring("GDT table applied\n");

	void (*gdt_setup_ptr)() = gdt_setup;
	terminal_writenumpad((uintptr_t)gdt_setup_ptr, 16, 16);

	__asm__ volatile ("mov $0x10, %ax;"
			  "mov %ax, %ds;"
			  "mov %ax, %es;"
			  "mov %ax, %fs;"
			  "mov %ax, %gs;"
			  "mov %ax, %ss;" "jmp $0x08,$csrefresh;");

	__asm__ volatile ("csrefresh:");
}
