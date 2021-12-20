#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>

#define MEM_SIZE 1 << 12
#define START_STACK 12
#define VF 0xF
#define EXPORT_PROGRAM 0
#define COSMAC_TARGET 1

/// Display width
#define DW 64

/// Display height
#define DH 32

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
	int _pos;
	int _size;
	uint16_t* _stack;
} STACK;

/// CHIP-8 CPU
typedef struct
{
	/// CPU Stack
	STACK* _S;

	/// CPU Memory
	MEMORY* _M;

	/// CPU Display Connection
	DISPLAY* _D;

	/// CPU Registers
	uint8_t _REG[16];

	/// Current instruction
	uint16_t _CINSTR;

	uint16_t _I;
	uint16_t _PC;
	
} CPU;

int fetch(CPU* cpu);
void decode(CPU* cpu);
void clear_display(DISPLAY* display);
void erase_memory(MEMORY* memory);
void init_stack(STACK* stack);
void draw_display(DISPLAY* disp);

CPU* create_cpu();

void push(STACK* s, uint16_t val);
uint16_t pop(STACK* s);