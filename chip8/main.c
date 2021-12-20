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

const char* PROGRAM = "IBM Logo.ch8";

void decode(uint16_t instruction, struct STACK* st);
void push(struct STACK* s, uint16_t val);
uint16_t pop(struct STACK* s);

// PROGRAM COUNTER
// only addresses 12 bits
uint16_t _PC = 0x0000; 

struct MEM;
struct STACK;
struct DISPLAY;

uint8_t REG[16];
uint16_t INDEX = 0x0000;

struct DISPLAY
{
	uint8_t _D[32][64];
};

struct MEM 
{
	uint8_t _M[MEM_SIZE];
};

struct STACK  {
	int _pos;
	int _size;
	uint16_t* _S;
};

void instr_error(uint16_t instruction, uint8_t parent_op, struct STACK* stack) {
	printf("INSTRUCTION ERROR\n");
	printf("Instruction %X\tParent %X\n", instruction, parent_op);
	/*int i = 0;
	printf("STACK CONTENTS\n\n");
	while (stack->_pos > 0)
	{
		uint16_t val = pop(stack);
		printf("%d\n", val);
	}*/
}

void delay(int number_of_seconds)
{
	// Converting time into milli_seconds
	int milli_seconds = 1000 * number_of_seconds;

	// Storing start time
	clock_t start_time = clock();

	// looping till required time is not achieved
	while (clock() < start_time + milli_seconds);
}

void push(struct STACK* s, uint16_t val)
{
	if (s->_pos == s->_size - 1) {
		uint16_t* nStack = (uint16_t*) realloc(s->_S, s->_size * 2);
		if (nStack == NULL)
		{
			printf("Could not resize stack, current size: %d\nexiting", s->_size);
			free(s->_S);
			exit(1);
		} 
		else
		{
			s->_S = nStack;
		}
		
		s->_size *= 2;
	}

	s->_pos += 1;
	s->_S[s->_pos] = val;
}

uint16_t pop(struct STACK* stack)
{
	if (stack->_pos - 1 >= 0) {
		stack->_pos -= 1;
		return stack->_S[stack->_pos + 1];
	}
}


uint16_t fetch(struct MEM* memory, struct STACK* st)
{
	uint16_t fByte = (uint16_t) memory->_M[_PC];
	uint8_t sByte = memory->_M[_PC + 1];

	uint16_t instruction = (fByte << 8) | sByte;

	_PC += 2;

	return instruction;
}

void decode(uint16_t instruction, struct STACK* st, struct MEM* mem, struct DISPLAY* disp)
{
	uint8_t op = ((instruction >> 12) & 0x000F);

	switch (op)
	{
	case 0x00:
	{
			uint8_t det = (instruction & 0x00FF);
			switch (det)
			{
			case 0xE0:
			{
				int i, j;
				for (i = 0; i < 32; i++)
				{
					for (j = 0; j < 64; j++)
					{
						disp->_D[i][j] = 0;
					}
				}
			} break;
			case 0xEE:
			{
				uint16_t addr = pop(st);
				_PC = addr;

			} break;
			default: instr_error(instruction, det, st);  break;
			}
		} break;
	case 0x01:
	{
		_PC = (instruction) & 0x0FFF;
	} break;
	case 0x02:
	{
		push(st, _PC);
		_PC = ((instruction) & 0x0FFF);
	} break;
	case 0x03:
	{
		// 3XNN 
		uint8_t reg = ((instruction >> 8) & 0x000F);
		uint8_t val = (instruction & 0x00FF);
		if (REG[reg] == val) {
			_PC += 2;
		}
	} break;
	case 0x04:
	{
		uint8_t reg = ((instruction >> 8) & 0x000F);
		uint8_t val = (instruction & 0x00FF);
		if (REG[reg] != val) {
			_PC += 2;
		}
	} break;
	case 0x05:
	{
		uint8_t condition = ((instruction & 0xFFF0));
		if (condition == 0x0000) {
			uint8_t xReg = ((instruction >> 8) & 0x000F);
			uint8_t yReg = ((instruction >> 4) & 0x000F);
			if (REG[xReg] == REG[yReg])
			{
				_PC += 2;
			}
		}
	} break;
	case 0x06:
	{
		// 6XNN
		uint8_t reg = ((instruction >> 8) & 0x000F);
		uint8_t val = (uint8_t)instruction & 0x00FF;

		REG[reg] = val;
	} break;
	case 0x07:
	{
		// ADD 
		uint8_t reg = ((instruction >> 8) & 0x000F);
		uint8_t val = (uint8_t)instruction;

		REG[reg] += val;
	} break;
	case 0x08:
	{
		uint8_t sub_op = (uint8_t)(instruction & 0x000F);
		switch (sub_op)
		{
		case 0x00: 
		{
			uint8_t xReg = (instruction >> 8) & 0x000F;
			uint8_t yReg = (instruction >> 4) & 0x000F;

			REG[xReg] = REG[yReg];
		} break;
		case 0x01:
		{
			uint8_t xReg = (instruction >> 8 ) & 0x000F;
			uint8_t yReg = (instruction >> 4 ) & 0x000F;

			REG[xReg] = REG[xReg] | REG[yReg];
		} break;
		case 0x02:
		{
			uint8_t xReg = (instruction >> 8) & 0x000F;
			uint8_t yReg = (instruction >> 4) & 0x000F;

			REG[xReg] = REG[xReg] & REG[yReg];
		} break;
		case 0x03:
		{
			uint8_t xReg = (instruction >> 8) & 0x000F;
			uint8_t yReg = (instruction >> 4) & 0x000F;

			REG[xReg] = REG[xReg] ^ REG[yReg];
		} break;
		case 0x04:
		{
			// ADD with VF FLAG
			uint8_t xReg = (instruction >> 8) & 0x000F;
			uint8_t yReg = (instruction >> 4) & 0x000F;
			if ((REG[xReg] + REG[yReg]) > 255)
			{
				REG[VF] = 1;
			}
			else 
			{
				REG[VF] = 0;
			}

			REG[xReg] += REG[yReg];
		} break;
		case 0x05:
		{
			// SUB with VF FLAG
			uint8_t xReg = (instruction >> 8) & 0x000F;
			uint8_t yReg = (instruction >> 4) & 0x000F;
			
			if (REG[xReg] > REG[yReg]) 
			{
				REG[VF] = 1;
			}
			else
			{
				REG[VF] = 0;
			}

			REG[xReg] -= REG[yReg];
		} break;
		case 0x06:
		{
			// 8XY6

			uint8_t xReg = (instruction >> 8 ) & 0x000F;
			uint8_t yReg = (instruction >> 4 ) & 0x000F;

			if (COSMAC_TARGET)
			{
				REG[xReg] = REG[yReg];
			}

			uint8_t reg_val = (REG[xReg] & 0x01);
			if (reg_val > 0)
			{
				REG[VF] = 1;
			}
			else
			{
				REG[VF] = 0;
			}

			REG[xReg] = REG[xReg] >> 1;
		} break;
		case 0x07:
		{
			// SUB with VF FLAG
			uint8_t xReg = (instruction >> 8) & 0x000F;
			uint8_t yReg = (instruction >> 4) & 0x000F;

			if (REG[yReg] > REG[xReg])
			{
				REG[VF] = 1;
			}
			else
			{
				REG[VF] = 0;
			}

			REG[xReg] = REG[yReg] - REG[xReg];
		} break;
		
		case 0x0E:
		{
			uint8_t xReg = (instruction >> 8) & 0x000F;
			uint8_t yReg = (instruction >> 4) & 0x000F;

			if (COSMAC_TARGET)
			{
				REG[xReg] = REG[yReg];
			}

			uint8_t reg_val = (REG[xReg] & 0x80);
			if (reg_val > 0)
			{
				REG[VF] = 1;
			} 
			else
			{
				REG[VF] = 0;
			}

			REG[xReg] = REG[xReg] << 1;
		} break;
		default: instr_error(instruction, sub_op, st); break;
		}
	} break;
	case 0x09:
	{
		uint8_t condition = ((instruction & 0xFFF0));
		if (condition == 0x0000) {
			uint8_t xReg = ((instruction & 0x0F00) >> 8);
			uint8_t yReg = ((instruction & 0x00F0) >> 4);
			if (REG[xReg] != REG[yReg])
			{
				_PC += 2;
			}
		}
	} break;
	case 0x0A:
	{
		uint16_t NNN = (instruction & 0x0FFF);
		INDEX = NNN;
	} break;
	case 0x0B:
	{
		uint16_t NNN = (instruction & 0x0FFF);
	
		if (COSMAC_TARGET)
		{
			_PC = NNN + (uint16_t)(REG[0x0]);
		} 
		else
		{
			_PC = NNN + (instruction & 0x0F00);
		}
	} break;
	case 0x0C:
	{
		uint8_t random = 23 & (uint8_t)(instruction & 0x00FF);
		uint8_t reg = ((instruction & 0x0F00) >> 8);

		REG[reg] = random;
	} break;
	case 0x0D:
	{
		// DXYN

		uint8_t height = (uint8_t)(instruction & 0x000F);
		uint16_t xCord;
		uint16_t yCord = (REG[(uint8_t)((instruction >> 4) & 0x000F)] & (31));
		
		REG[VF] = 0;

		uint8_t i;
		for (i = 0; i < height; i++)
		{
			if (yCord > 31)
			{
				break;
			}

			xCord = (REG[(uint8_t)((instruction >> 8) & 0x000F)] & (63));
			uint8_t sData = mem->_M[INDEX + (uint16_t)i];

			uint8_t bit;
			for (bit = 0; bit < 8; bit++) 
			{
				if (xCord > 63) {
					break;
				}

				uint8_t val = (sData >> (7 - bit)) & 0x0001;
				if ((val > 0) && (disp->_D[yCord][xCord] == 1))
				{
					uint8_t nData = (sData & (~((0x1 << (7 - bit)))));
					mem->_M[INDEX + i] = nData;
					disp->_D[yCord][xCord] = 0;
					REG[VF] = 1;
				}
				else if ((val > 0) && (disp->_D[yCord][xCord] == 0))
				{
					disp->_D[yCord][xCord] = 1;
				}
				
				xCord += 1;
			}
			yCord += 1;
		}	
	} break;

	case 0x0F:
	{
		uint8_t sub_op = (uint8_t)((instruction) & 0x00FF);
		switch (sub_op)
		{
		case 0x55:
		{
			uint8_t reg = 0;
			uint8_t range = (uint8_t)((instruction >> 8) & 0x000F);
			for (reg = 0; reg <= range; reg++)
			{
				if (COSMAC_TARGET)
				{
					mem->_M[INDEX] = REG[reg];
					INDEX++;
				}
				else
				{
					mem->_M[INDEX + reg] = REG[reg];
				}
			}
		} break;
		case 0x65:
		{
			uint8_t reg = 0;
			uint8_t range = (uint8_t)((instruction >> 8) & 0x000F);
			for (reg = 0; reg <= range; reg++)
			{
				if (COSMAC_TARGET)
				{
					REG[reg] = mem->_M[INDEX];
					INDEX++;
				}
				else
				{
					REG[reg] = mem->_M[INDEX + reg];
				}
			}
		} break;
		case 0x33:
		{
			int reg = ((instruction >> 8) & 0x000F);
			int val = REG[reg];

			uint8_t last_digit = (uint8_t)(val % 10);
			val /= 10;

			uint8_t mid_digit = (uint8_t)(val % 10);
			val /= 10;

			uint8_t first_digit = (uint8_t)(val / 10);

			mem->_M[INDEX] = first_digit;
			mem->_M[INDEX + 1] = mid_digit;
			mem->_M[INDEX + 2] = last_digit;
		} break;
		default: instr_error(instruction, sub_op, st); break;
		}
	} break;

	default: instr_error(instruction, op, st); break;
	}
}


int main(int argc, char** argv)
{
	struct MEM memory;
	struct STACK stack;
	struct DISPLAY display;


	int i, j;
	for (i = 0; i < 32; i++)
	{
		memset(display._D[i], 0, 64);
	}

	memset(memory._M, 0, MEM_SIZE);
	memset(REG, 0, 16);

	FILE* fp;
	uint8_t program[130];
	fp = fopen(PROGRAM, "r");
	if (fp == NULL)
	{
		printf("Error");
		return 1;
	}

	while (fgets(program, 130, fp) != NULL) {}

	//_strrev(program);

	fclose(fp);

	if (EXPORT_PROGRAM)
	{
		fp = fopen("program.txt", "w");
		if (fp == NULL)
		{
			printf("Error opening program output file\n");
			exit(1);
		}
		else
		{
			int i = 0;
			for (i = 1; i < 500 - 1; i+=2)
			{
				fputc(program[i - 1], fp);
				fputc(program[i], fp);
				//fputc('\n', fp);
			}
		}

		fclose(fp);
	}

	for (i = 0; i < 130; i++)
	{
		memory._M[512 + i] = program[i];
	}

	_PC = 512;

	stack._S = (uint16_t*)malloc(START_STACK);
	stack._size = START_STACK;
	stack._pos = 0;
	
	uint16_t instruction;

	for (;;)
	{
		instruction = fetch(&memory, &stack);
		decode(instruction, &stack, &memory, &display);

		for (i = 0; i < 32; i++)
		{
			for (j = 0; j < 64; j++)
			{
				if (display._D[i][j] != 0)
				{
					printf("%c", '#');
				}
				else
				{
					printf("%c", ' ');
				}
			}
			printf("\n");
		}

		delay(1);
	}

	free(stack._S);
	return 0;
}