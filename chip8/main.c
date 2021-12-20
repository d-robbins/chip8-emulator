#include "chip8.h"

const char* PROGRAM = "IBM Logo.ch8";

void instr_error(uint16_t instruction, uint8_t parent_op, STACK* stack) {
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

void push(STACK* s, uint16_t val)
{
	if (s->_pos == s->_size - 1) {
		uint16_t* nStack = (uint16_t*) realloc(s->_stack, s->_size * 2);
		if (nStack == NULL)
		{
			printf("Could not resize stack, current size: %d\nexiting", s->_size);
			free(s->_stack);
			exit(1);
		} 
		else
		{
			s->_stack = nStack;
		}
		
		s->_size *= 2;
	}

	s->_pos += 1;
	s->_stack[s->_pos] = val;
}

uint16_t pop(STACK* stack)
{
	if (stack->_pos - 1 >= 0) {
		stack->_pos -= 1;
		return stack->_stack[stack->_pos + 1];
	}
}

int fetch(CPU* cpu)
{
	uint16_t fByte = (uint16_t) cpu->_M->_MEM[cpu->_PC];
	uint8_t sByte = cpu->_M->_MEM[cpu->_PC + 1];

	uint16_t instruction = (fByte << 8) | sByte;

	cpu->_PC += 2;

	cpu->_CINSTR = instruction;

	return 0;
}

void decode(CPU* cpu)
{
	uint8_t op = ((cpu->_CINSTR >> 12) & 0x000F);

	switch (op)
	{
	case 0x00:
	{
			uint8_t det = (cpu->_CINSTR & 0x00FF);
			switch (det)
			{
			case 0xE0:
			{
				int i, j;
				for (i = 0; i < 32; i++)
				{
					for (j = 0; j < 64; j++)
					{
						cpu->_D->_DSP[i][j] = 0;
					}
				}
			} break;
			case 0xEE:
			{
				uint16_t addr = pop(cpu->_S);
				cpu->_PC = addr;

			} break;
			default: instr_error(cpu->_CINSTR, det,cpu->_S);  break;
			}
		} break;
	case 0x01:
	{
		cpu->_PC = (cpu->_CINSTR) & 0x0FFF;
	} break;
	case 0x02:
	{
		push(cpu->_S, cpu->_PC);
		cpu->_PC = ((cpu->_CINSTR) & 0x0FFF);
	} break;
	case 0x03:
	{
		// 3XNN 
		uint8_t reg = ((cpu->_CINSTR >> 8) & 0x000F);
		uint8_t val = (cpu->_CINSTR & 0x00FF);
		if (cpu->_REG[reg] == val) {
			cpu->_PC += 2;
		}
	} break;
	case 0x04:
	{
		uint8_t reg = ((cpu->_CINSTR >> 8) & 0x000F);
		uint8_t val = (cpu->_CINSTR & 0x00FF);
		if (cpu->_REG[reg] != val) {
			cpu->_PC += 2;
		}
	} break;
	case 0x05:
	{
		uint8_t condition = ((cpu->_CINSTR & 0xFFF0));
		if (condition == 0x0000) {
			uint8_t xReg = ((cpu->_CINSTR >> 8) & 0x000F);
			uint8_t yReg = ((cpu->_CINSTR >> 4) & 0x000F);
			if (cpu->_REG[xReg] == cpu->_REG[yReg])
			{
				cpu->_PC += 2;
			}
		}
	} break;
	case 0x06:
	{
		// 6XNN
		uint8_t reg = ((cpu->_CINSTR >> 8) & 0x000F);
		uint8_t val = (uint8_t)cpu->_CINSTR & 0x00FF;

		cpu->_REG[reg] = val;
	} break;
	case 0x07:
	{
		// ADD 
		uint8_t reg = ((cpu->_CINSTR >> 8) & 0x000F);
		uint8_t val = (uint8_t)cpu->_CINSTR;

		cpu->_REG[reg] += val;
	} break;
	case 0x08:
	{
		uint8_t sub_op = (uint8_t)(cpu->_CINSTR & 0x000F);
		switch (sub_op)
		{
		case 0x00: 
		{
			uint8_t xReg = (cpu->_CINSTR >> 8) & 0x000F;
			uint8_t yReg = (cpu->_CINSTR >> 4) & 0x000F;

			cpu->_REG[xReg] = cpu->_REG[yReg];
		} break;
		case 0x01:
		{
			uint8_t xReg = (cpu->_CINSTR >> 8 ) & 0x000F;
			uint8_t yReg = (cpu->_CINSTR >> 4 ) & 0x000F;

			cpu->_REG[xReg] = cpu->_REG[xReg] | cpu->_REG[yReg];
		} break;
		case 0x02:
		{
			uint8_t xReg = (cpu->_CINSTR >> 8) & 0x000F;
			uint8_t yReg = (cpu->_CINSTR >> 4) & 0x000F;

			cpu->_REG[xReg] = cpu->_REG[xReg] & cpu->_REG[yReg];
		} break;
		case 0x03:
		{
			uint8_t xReg = (cpu->_CINSTR >> 8) & 0x000F;
			uint8_t yReg = (cpu->_CINSTR >> 4) & 0x000F;

			cpu->_REG[xReg] = cpu->_REG[xReg] ^ cpu->_REG[yReg];
		} break;
		case 0x04:
		{
			// ADD with VF FLAG
			uint8_t xReg = (cpu->_CINSTR >> 8) & 0x000F;
			uint8_t yReg = (cpu->_CINSTR >> 4) & 0x000F;
			if ((cpu->_REG[xReg] + cpu->_REG[yReg]) > 255)
			{
				cpu->_REG[VF] = 1;
			}
			else 
			{
				cpu->_REG[VF] = 0;
			}

			cpu->_REG[xReg] += cpu->_REG[yReg];
		} break;
		case 0x05:
		{
			// SUB with VF FLAG
			uint8_t xReg = (cpu->_CINSTR >> 8) & 0x000F;
			uint8_t yReg = (cpu->_CINSTR >> 4) & 0x000F;
			
			if (cpu->_REG[xReg] > cpu->_REG[yReg]) 
			{
				cpu->_REG[VF] = 1;
			}
			else
			{
				cpu->_REG[VF] = 0;
			}

			cpu->_REG[xReg] -= cpu->_REG[yReg];
		} break;
		case 0x06:
		{
			// 8XY6

			uint8_t xReg = (cpu->_CINSTR >> 8 ) & 0x000F;
			uint8_t yReg = (cpu->_CINSTR >> 4 ) & 0x000F;

			if (COSMAC_TARGET)
			{
				cpu->_REG[xReg] = cpu->_REG[yReg];
			}

			uint8_t reg_val = (cpu->_REG[xReg] & 0x01);
			if (reg_val > 0)
			{
				cpu->_REG[VF] = 1;
			}
			else
			{
				cpu->_REG[VF] = 0;
			}

			cpu->_REG[xReg] = cpu->_REG[xReg] >> 1;
		} break;
		case 0x07:
		{
			// SUB with VF FLAG
			uint8_t xReg = (cpu->_CINSTR >> 8) & 0x000F;
			uint8_t yReg = (cpu->_CINSTR >> 4) & 0x000F;

			if (cpu->_REG[yReg] > cpu->_REG[xReg])
			{
				cpu->_REG[VF] = 1;
			}
			else
			{
				cpu->_REG[VF] = 0;
			}

			cpu->_REG[xReg] = cpu->_REG[yReg] - cpu->_REG[xReg];
		} break;
		
		case 0x0E:
		{
			uint8_t xReg = (cpu->_CINSTR >> 8) & 0x000F;
			uint8_t yReg = (cpu->_CINSTR >> 4) & 0x000F;

			if (COSMAC_TARGET)
			{
				cpu->_REG[xReg] = cpu->_REG[yReg];
			}

			uint8_t reg_val = (cpu->_REG[xReg] & 0x80);
			if (reg_val > 0)
			{
				cpu->_REG[VF] = 1;
			} 
			else
			{
				cpu->_REG[VF] = 0;
			}

			cpu->_REG[xReg] = cpu->_REG[xReg] << 1;
		} break;
		default: instr_error(cpu->_CINSTR, sub_op,cpu->_S); break;
		}
	} break;
	case 0x09:
	{
		uint8_t condition = ((cpu->_CINSTR & 0xFFF0));
		if (condition == 0x0000) {
			uint8_t xReg = ((cpu->_CINSTR & 0x0F00) >> 8);
			uint8_t yReg = ((cpu->_CINSTR & 0x00F0) >> 4);
			if (cpu->_REG[xReg] != cpu->_REG[yReg])
			{
				cpu->_PC += 2;
			}
		}
	} break;
	case 0x0A:
	{
		uint16_t NNN = (cpu->_CINSTR & 0x0FFF);
		cpu->_I = NNN;
	} break;
	case 0x0B:
	{
		uint16_t NNN = (cpu->_CINSTR & 0x0FFF);
	
		if (COSMAC_TARGET)
		{
			cpu->_PC = NNN + (uint16_t)(cpu->_REG[0x0]);
		} 
		else
		{
			cpu->_PC = NNN + (cpu->_CINSTR & 0x0F00);
		}
	} break;
	case 0x0C:
	{
		uint8_t random = 23 & (uint8_t)(cpu->_CINSTR & 0x00FF);
		uint8_t reg = ((cpu->_CINSTR & 0x0F00) >> 8);

		cpu->_REG[reg] = random;
	} break;
	case 0x0D:
	{
		// DXYN

		uint8_t height = (uint8_t)(cpu->_CINSTR & 0x000F);
		uint16_t xCord;
		uint16_t yCord = (cpu->_REG[(uint8_t)((cpu->_CINSTR >> 4) & 0x000F)] & (31));
		
		cpu->_REG[VF] = 0;

		uint8_t i;
		for (i = 0; i < height; i++)
		{
			if (yCord > 31)
			{
				break;
			}

			xCord = (cpu->_REG[(uint8_t)((cpu->_CINSTR >> 8) & 0x000F)] & (63));
			uint8_t sData = cpu->_M->_MEM[cpu->_I + (uint16_t)i];

			uint8_t bit;
			for (bit = 0; bit < 8; bit++) 
			{
				if (xCord > 63) {
					break;
				}

				uint8_t val = (sData >> (7 - bit)) & 0x0001;
				if ((val > 0) && (cpu->_D->_DSP[yCord][xCord] == 1))
				{
					uint8_t nData = (sData & (~((0x1 << (7 - bit)))));
					cpu->_M->_MEM[cpu->_I + i] = nData;
					cpu->_D->_DSP[yCord][xCord] = 0;
					cpu->_REG[VF] = 1;
				}
				else if ((val > 0) && (cpu->_D->_DSP[yCord][xCord] == 0))
				{
					cpu->_D->_DSP[yCord][xCord] = 1;
				}
				
				xCord += 1;
			}
			yCord += 1;
		}	
	} break;

	case 0x0F:
	{
		uint8_t sub_op = (uint8_t)((cpu->_CINSTR) & 0x00FF);
		switch (sub_op)
		{
		case 0x55:
		{
			uint8_t reg = 0;
			uint8_t range = (uint8_t)((cpu->_CINSTR >> 8) & 0x000F);
			for (reg = 0; reg <= range; reg++)
			{
				if (COSMAC_TARGET)
				{
					cpu->_M->_MEM[cpu->_I] = cpu->_REG[reg];
					cpu->_I++;
				}
				else
				{
					cpu->_M->_MEM[cpu->_I + reg] = cpu->_REG[reg];
				}
			}
		} break;
		case 0x65:
		{
			uint8_t reg = 0;
			uint8_t range = (uint8_t)((cpu->_CINSTR >> 8) & 0x000F);
			for (reg = 0; reg <= range; reg++)
			{
				if (COSMAC_TARGET)
				{
					cpu->_REG[reg] =cpu->_M->_MEM[cpu->_I];
					cpu->_I++;
				}
				else
				{
					cpu->_REG[reg] =cpu->_M->_MEM[cpu->_I + reg];
				}
			}
		} break;
		case 0x33:
		{
			int reg = ((cpu->_CINSTR >> 8) & 0x000F);
			int val = cpu->_REG[reg];

			uint8_t last_digit = (uint8_t)(val % 10);
			val /= 10;

			uint8_t mid_digit = (uint8_t)(val % 10);
			val /= 10;

			uint8_t first_digit = (uint8_t)(val / 10);

			cpu->_M->_MEM[cpu->_I] = first_digit;
			cpu->_M->_MEM[cpu->_I + 1] = mid_digit;
			cpu->_M->_MEM[cpu->_I + 2] = last_digit;
		} break;
		default: instr_error(cpu->_CINSTR, sub_op,cpu->_S); break;
		}
	} break;

	default: instr_error(cpu->_CINSTR, op,cpu->_S); break;
	}
}

CPU* create_cpu()
{
	// create cpu
	CPU* cpu = malloc(sizeof(CPU));
	if (!cpu) { perror("malloc of cpu\n"); exit(1); }

	// create the display
	cpu->_D = malloc(sizeof(DISPLAY));
	if (!cpu->_D) { perror("malloc of display\n"); exit(1); }

	// create memory
	cpu->_M = malloc(sizeof(MEMORY));
	if (!cpu->_M) { perror("malloc of memory\n"); exit(1); }

	// create stack
	cpu->_S = malloc(sizeof(STACK));
	if (!cpu->_S) { perror("malloc of stack\n"); exit(1); }

	clear_display(cpu->_D);

	erase_memory(cpu->_M);

	init_stack(cpu->_S);

	// set program counter to byte 512
	cpu->_PC = 0x0200;
	
	// set index register to 0
	cpu->_I = 0x0000;

	// set current instruction to 0x0000
	cpu->_CINSTR = 0x0000;

	return cpu;
}

/// <summary>
/// Clear the cpu's display
/// </summary>
/// <param name="display"></param>
void clear_display(DISPLAY* display)
{
	int i;

	for (i = 0; i < DH; i++)
	{
		memset(display->_DSP[i], 0, DW);
	}
}

/// <summary>
/// Clear the CPU's memory
/// </summary>
/// <param name="memory"></param>
void erase_memory(MEMORY* memory)
{
	memset(memory->_MEM, 0, MEM_SIZE);
}

void init_stack(STACK* stack)
{
	stack->_stack = malloc(sizeof(START_STACK));
	stack->_size = START_STACK;
	stack->_pos = 0;
}

void draw_display(DISPLAY* disp)
{
	int i, j;
	for (i = 0; i < 32; i++)
	{
		for (j = 0; j < 64; j++)
		{
			if (disp->_DSP[i][j] != 0)
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
}

int main(int argc, char** argv)
{
	CPU* cpu = create_cpu();

	free(cpu->_S->_stack);
	free(cpu->_S);
	free(cpu->_D);
	free(cpu->_M);
	free(cpu);
	return 0;
}