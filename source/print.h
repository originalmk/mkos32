#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define NORMAL_MODE 0
#define FORMAT_MODE 1

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

void terminal_putchar(char);
void terminal_newline()
{
	for (size_t x = terminal_column; x < VGA_WIDTH; x++)
	{
		terminal_putchar(' ');
	}
}

void terminal_putchar(char c)
{
	if (c == '\n')
	{
		terminal_newline();
		return;
	}

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

void terminal_write(const char *data, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		terminal_putchar(data[i]);
	}
}

// Pad length is what number length should be, including padding
void terminal_writenumpad(const uint64_t number, int base, int pad_length)
{
	char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int size = 0;
	char numChars[32];
	// TODO: Remove const and this
	uint64_t numbero = number;

	if (numbero == 0)
	{
		numChars[31] = '0';
		size = 1;
	}
	else
	{
		while (numbero != 0)
		{
			uint64_t rest = numbero / base;
			uint64_t digit = numbero % base;

			numChars[31 - size++] = digits[digit];
			numbero = rest;
		}
	}

	// Write pad_length - size of zeros
	// If padding won't be needed (number is longer than pad_legnth)
	// then loop won't run at all
	while (pad_length > size)
	{
		numChars[31 - size++] = digits[0];
	}

	terminal_write(numChars + (32 - size), size);
}

void terminal_writenum(uint64_t number, int base)
{
	terminal_writenumpad(number, base, 0);
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

void print_texts()
{
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
}

void printf(const char *format, ...)
{
	int current_mode = NORMAL_MODE;
	va_list vl;
	va_start(vl, format);

	for (int i = 0; format[i] != '\0'; i++)
	{
		char current = format[i];
		switch (current)
		{
			case '%':
				if (current_mode == FORMAT_MODE)
				{
					current_mode = NORMAL_MODE;
					terminal_putchar(current);
				}
				else if (current_mode == NORMAL_MODE)
				{
					current_mode = FORMAT_MODE;
				}
				break;
			default:
				if (current_mode == NORMAL_MODE)
				{
					terminal_putchar(current);
				}
				else if (current_mode == FORMAT_MODE)
				{
					// Print argument
					switch (current)
					{
						case 's':
							terminal_writestring(va_arg(vl, char*));
							break;
						case 'd':
							terminal_writenum(va_arg(vl, int), 10);
							break;
						case 'x':
							terminal_writenum(va_arg(vl, int), 16);
					}
					current_mode = NORMAL_MODE;
				}
				break;
		}
	}

	va_end(vl);
}
