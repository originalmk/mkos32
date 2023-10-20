#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Check if the compiler thinks you are targeting the wrong operating
 * system. 
 */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/*
 * This tutorial will only work for the 32-bit ix86 targets. 
 */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

/*
 * Hardware text mode color constants. 
 */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char *str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t *terminal_buffer;

void terminal_initialize(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t *) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c)
{
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH)
	{
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
		{
			terminal_row--;
			// Fixed at last row now
			// Now we need to move from second line to end to the start
			for (size_t y = 1; y < VGA_HEIGHT; y++)
			{
				for (size_t x = 0; x < VGA_WIDTH; x++)
				{
					const size_t src_index =
					    y * VGA_WIDTH + x;
					char toCopy =
					    terminal_buffer[src_index];
					terminal_putentryat(toCopy,
							    terminal_color,
							    x, y - 1);
				}
			}

			for (size_t x = 0; x < VGA_WIDTH; x++)
			{
				terminal_putentryat(' ', terminal_color,
						    x, VGA_HEIGHT - 1);
			}
		}
	}
	// Write branding
	uint8_t prevColor = terminal_color;

	terminal_color =
	    vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_MAGENTA);
	terminal_putentryat('M', terminal_color, VGA_WIDTH - 4, 0);
	terminal_color = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_CYAN);
	terminal_putentryat('K', terminal_color, VGA_WIDTH - 3, 0);
	terminal_color = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_BLUE);
	terminal_putentryat('O', terminal_color, VGA_WIDTH - 2, 0);
	terminal_color =
	    vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREEN);
	terminal_putentryat('S', terminal_color, VGA_WIDTH - 1, 0);

	terminal_color = prevColor;
}

void terminal_newline()
{
	for (size_t x = terminal_column; x < VGA_WIDTH; x++)
	{
		terminal_putchar(' ');
	}
}

void terminal_write(const char *data, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (data[i] == '\n')
		{
			terminal_newline();
		}
		else
		{
			terminal_putchar(data[i]);
		}
	}
}

void terminal_writenum(uint64_t number, int base)
{
	char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	size_t size = 0;
	char numChars[32];

	while (number != 0)
	{
		uint64_t rest = number / base;
		uint64_t digit = number % base;

		numChars[31 - size++] = digits[digit];
		number = rest;
	}

	terminal_write(numChars + (32 - size), size);
}

void terminal_writestring(const char *data)
{
	terminal_write(data, strlen(data));
}

void terminal_writegreeting()
{
	terminal_writestring("                              .-'''-.        \n");
	terminal_writestring
	    ("                             '   _    \\      \n");
	terminal_writestring
	    (" __  __   ___        .     /   /` '.   \\     \n");
	terminal_writestring
	    ("|  |/  `.'   `.    .'|    .   |     \\  '     \n");
	terminal_writestring("|   .-.  .-.   ' .'  |    |   '      |  '    \n");
	terminal_writestring
	    ("|  |  |  |  |  |<    |    \\    \\     / /     \n");
	terminal_writestring("|  |  |  |  |  | |   | ____`.   ` ..' / _    \n");
	terminal_writestring
	    ("|  |  |  |  |  | |   | \\ .'   '-...-'`.' |   \n");
	terminal_writestring("|  |  |  |  |  | |   |/  .           .   | / \n");
	terminal_writestring
	    ("|__|  |__|  |__| |    /\\  \\        .'.'| |// \n");
	terminal_writestring
	    ("                 |   |  \\  \\     .'.'.-'  /  \n");
	terminal_writestring
	    ("                 '    \\  \\  \\    .'   \\_.'   \n");
	terminal_writestring("                '------'  '---'              \n");
}

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

#define KERNEL_SEGMENT 0
#define USER_SEGMENT 1
#define CODE_SEGMENT 0
#define USER_SEGMENT 1

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
	uint64_t gdt_encoded = 0;

	// Insert base
	gdt_encoded |= ((uint64_t) (entry.base & 0xFF000000) << 32);
	gdt_encoded |= ((uint64_t) (entry.base & 0x00FFFFFF) << 16);

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
	gdt_encoded |= (access_byte << 20);

	return gdt_encoded;
}

struct gdt_table {
	uint16_t entries_count;
	uint32_t *dest_pointer;
	struct gdt_entry entries[16];
};

void apply_table(struct gdt_table table)
{
	// Null entry added implicitly
	uint64_t null_entry_encoded = 0;
	*table.dest_pointer = null_entry_encoded;

	for (int i = 0; i < table.entries_count; i++)
	{
		uint64_t entry_encoded = gdt_entry_encode(table.entries[i]);
		uint32_t *dest_address = table.dest_pointer + (i + 1) * 8;
		*dest_address = entry_encoded;
	}

	__asm__("lgdt (%0)" : : "r" (&table));
}

void kernel_main(void)
{
	/*
	 * Initialize terminal interface 
	 */
	terminal_initialize();

	/*
	 * Newline support is left as an exercise. 
	 */
	terminal_writestring("TEGO NIE POWINNO BYC WIDAC\n");
	terminal_writestring
	    ("Witam i pozdrawiam, MK\nTest wielolinijkowosci\n");
	for (size_t i = 0; i < VGA_HEIGHT - 3; i++)
	{
		for (size_t j = 0; j < i; j++)
		{
			terminal_writestring("*");
		}
		if (i != VGA_HEIGHT - 3)
			terminal_writestring("\n");
	}
	terminal_newline();
	terminal_writenum(123456789, 10);
	terminal_newline();
	terminal_writenum(189, 16);
	terminal_newline();
	terminal_writegreeting();

	size_t start_point;

	__asm__("mov $_start, %%eax": : :"eax");
	__asm__("mov %%eax, %0;": "=r"(start_point):);

	terminal_writestring("\n\nPoczatek kernela:\n");
	terminal_writenum(start_point, 10);
	terminal_newline();

	size_t human_readable = start_point / (1024 * 1024);
	terminal_writenum(human_readable, 10);
	terminal_writestring(" MiB\n");

	terminal_writestring("Test wpisu GDT:\n");
	struct gdt_entry entry =
	    gdt_entry_create(0, 0x400, KERNEL_SEGMENT, CODE_SEGMENT);
	uint64_t entry_encoded = gdt_entry_encode(entry);
	terminal_writenum(entry_encoded, 16);
	terminal_newline();
}
