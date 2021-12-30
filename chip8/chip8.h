#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#define MEM_SIZE 1 << 12

#define START_STACK 12

#define DEBUG 1

/// Flag register
#define VF 0xF

#define EXPORT_PROGRAM 0
#define PROGRAM_OFFSET 0x200

#define FONTSET_SIZE 80
#define FONT_OFFSET 0x50

/// Set machine target
#define COSMAC_TARGET 0

/// Display width
#define DW 64

/// Display height
#define DH 32

#define PROGRAM_SIZE 1000

typedef struct
{
	uint16_t _instruction;
	uint16_t _pc;
} DEBUG_INFO;

typedef struct
{
	uint8_t _DSP[DH][DW];

} DISPLAY;

typedef struct
{
	uint8_t _MEM[MEM_SIZE];
} MEMORY;

typedef struct
{
	uint8_t _KEYBOARD[16];
} KEYBOARD;

/// CHIP-8 CPU
typedef struct
{
	/// CPU Memory
	MEMORY* _M;

	KEYBOARD _K;

	/// CPU Display Connection
	DISPLAY* _D;

	uint16_t _STACK[12];
	uint8_t _SP;

	/// CPU Registers
	uint8_t _REG[16];

	/// Current instruction
	uint16_t _CINSTR;

	DEBUG_INFO _CALLS[100];
	uint16_t _CALLSI;

	uint16_t _I;
	uint16_t _PC;
	
} CPU;

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0 loc: 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1 loc: 5
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2 loc: 
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

int fetch(CPU* cpu);
void decode(CPU* cpu);
void clear_display(DISPLAY* display);
void erase_memory(MEMORY* memory);
void draw_display(DISPLAY* disp);

void display_stack(CPU* cpu);
void display_registers(CPU* cpu);

void load_program(CPU* cpu, const char* program);
void delay_cpu(float number_of_seconds);

void run(CPU* cpu);

CPU* create_cpu();

