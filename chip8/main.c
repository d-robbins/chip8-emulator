#include "chip8.h"
#include <conio.h>
#include <SFML/Window.h>
#include <SFML/Graphics.h>

const char* PROGRAM = "tests/bc_test.ch8";

void instr_error(uint16_t instruction, uint8_t parent_op) {
	printf("INSTRUCTION ERROR\n");
	printf("Instruction %.4X\tParent %.1X\n", instruction, parent_op);
}

void delay_cpu(float number_of_seconds)
{
	// Converting time into milli_seconds
	int milli_seconds = 1000.0 * number_of_seconds;

	// Storing start time
	clock_t start_time = clock();

	// looping till required time is not achieved
	while (clock() < start_time + milli_seconds);
}

int fetch(CPU* cpu)
{
	uint16_t fByte = (uint16_t) cpu->_M->_MEM[cpu->_PC];
	uint8_t sByte = cpu->_M->_MEM[cpu->_PC + 1];

	uint16_t instruction = (fByte << 8) | sByte;

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
				cpu->_SP--;
				if (cpu->_SP < 0) {
					perror("Stack overflow");
					exit(1);
				}
				cpu->_PC = cpu->_STACK[cpu->_SP];

			} break;
			default: instr_error(cpu->_CINSTR, det);  break;
			}
		} break;
	case 0x01:
	{
		cpu->_PC = ((cpu->_CINSTR) & 0x0FFF);
	} break;
	case 0x02:
	{
		cpu->_STACK[cpu->_SP] = cpu->_PC;
		cpu->_SP++;
		cpu->_PC = (cpu->_CINSTR & 0x0FFF) + PROGRAM_OFFSET;
		//cpu->_PC -= 2;
	} break;
	case 0x03:
	{
		// 3XNN 
		uint8_t reg = ((cpu->_CINSTR >> 8) & 0x000F);
		uint8_t val = (cpu->_CINSTR & 0x00FF);
		if (cpu->_REG[reg] == val)
		{
			cpu->_PC += 2;
		}
	} break;
	case 0x04:
	{
		// 4XNN
		uint8_t reg = ((cpu->_CINSTR >> 8) & 0x000F);
		uint8_t val = (cpu->_CINSTR & 0x00FF);
		if (cpu->_REG[reg] != val) {
			cpu->_PC += 2;
		}
	} break;
	case 0x05:
	{
		// 5XY0
		if ((cpu->_CINSTR & 0x000F) == 0)
		{
			// 5XY0
			uint8_t xReg = ((cpu->_CINSTR & 0x0F00) >> 8) & 0x000F;
			uint8_t yReg = ((cpu->_CINSTR & 0x00F0) >> 4) & 0x000F;
			if (cpu->_REG[xReg] == cpu->_REG[yReg])
			{
				cpu->_PC += 2;
			}
		}
	} break;
	case 0x06:
	{
		// 6XNN
		uint8_t val = (uint8_t)cpu->_CINSTR & 0x00FF;
		uint8_t reg = ((cpu->_CINSTR >> 8) & 0x000F);

		cpu->_REG[reg] = val;
	} break;
	case 0x07:
	{
		// 7XNN 
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

			cpu->_REG[xReg] = (cpu->_REG[xReg] + cpu->_REG[yReg]) & 0xFF;
		} break;
		case 0x05:
		{
			// 8XY5
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

			cpu->_REG[xReg] = cpu->_REG[xReg] - cpu->_REG[yReg];
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
			// 8XY7
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
		default: instr_error(cpu->_CINSTR, sub_op); break;
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
			cpu->_PC = NNN + (uint16_t)(cpu->_REG[0]);
		} 
		else
		{
			cpu->_PC = NNN + cpu->_REG[((cpu->_CINSTR >> 8) & 0x000F)];
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
					uint8_t nData = sData ^ (0x0001 << (7 - bit));

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
	case 0x0E:
	{
		uint16_t cond = (cpu->_CINSTR & 0xF0FF);
		switch (cond)
		{
		case 0xE09E:
		{
			if (cpu->_K._KEYBOARD[(cpu->_CINSTR >> 8) & 0x000F] == 1)
			{
				cpu->_PC += 2;
			}
		} break;

		case 0xE0A1:
		{
			if (cpu->_K._KEYBOARD[(cpu->_CINSTR >> 8) & 0x000F] != 1)
			{
				cpu->_PC += 2;
			}
		} break;

		default:
			instr_error(cpu->_CINSTR, 0x0E);
			break;
		}
	} break;
	case 0x0F:
	{
		uint8_t sub_op = (uint8_t)((cpu->_CINSTR) & 0x00FF);
		switch (sub_op)
		{
		case 0x29:
		{
			uint8_t reg = (cpu->_CINSTR >> 8) & 0x000F;
			cpu->_I = FONT_OFFSET + cpu->_REG[reg];
		} break;
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
		case 0x1E:
		{
			uint8_t reg = (cpu->_CINSTR >> 8) & 0x000F;
			cpu->_I += cpu->_REG[reg];
		} break;
		default: instr_error(cpu->_CINSTR, sub_op); break;
		}
	} break;

	default: instr_error(cpu->_CINSTR, op); break;
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

	memset(cpu->_K._KEYBOARD, 0, 16);

	// initialize font
	int i;
	for (i = 0; i < FONTSET_SIZE; i++) {
		cpu->_M->_MEM[FONT_OFFSET + i] = fontset[i];
	}

	memset(cpu->_STACK, 0, 12);
	cpu->_SP = 0;

	clear_display(cpu->_D);

	erase_memory(cpu->_M);

	memset(cpu->_REG, 0, 16);

	memset(cpu->_CALLS, 0, 100);
	cpu->_CALLSI = 0;

	// set program counter to byte 512
	cpu->_PC = PROGRAM_OFFSET;
	
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

void display_stack(CPU* cpu)
{
	int i;
	printf("STACK: ");
	for (i = 0; i < cpu->_SP; i++) {
		printf("%.4x ", cpu->_STACK[i]);
	}
	printf("\n");
}	


void display_registers(CPU* cpu)
{
	printf("------REGISTERS------\n");
	int i;
	for (i = 0; i < 16; i++)
	{
		printf("V%.1X: %.2X ", i, cpu->_REG[i]);
		if (((i+1) % 2) == 0) 
		{
			printf("\n");
		}
	}
	printf("\nINDEX: %.4X\n", cpu->_I);
	printf("---------------------\n");
}

void load_program(CPU* cpu, const char* program)
{
	uint8_t programFile[PROGRAM_SIZE];

	memset(programFile, 0, PROGRAM_SIZE);

	FILE* fp;
	fp = fopen(program, "r");
	if (!fp)
	{
		perror("Error opening program file %s\n", program);
		exit(1);
	}

	while (fgets(programFile, PROGRAM_SIZE, fp) != NULL) {}

	fclose(fp);

	int i;
	for (i = 0; i < PROGRAM_SIZE; i++)
	{
		cpu->_M->_MEM[PROGRAM_OFFSET + i] = programFile[i];
	}
}

void run(CPU* cpu)
{
	for (;;)
	{
		fetch(cpu);

		if (DEBUG)
		{
			display_registers(cpu);
			display_stack(cpu);
			printf("PC: %.4X\n", cpu->_PC);
			printf("CINSTR: %.4X\n", cpu->_CINSTR);
		}

		cpu->_PC += 2;

		decode(cpu);
		draw_display(cpu->_D);
		//delay_cpu(0.75);
		//_getch();
	}
}

int main(int argc, char** argv)
{
	CPU* cpu = create_cpu();
	load_program(cpu, PROGRAM);

	//run(cpu);

	sfRenderWindow* window;
	sfVideoMode vmode;
	vmode.height = 720;
	vmode.width = 1280;
	vmode.bitsPerPixel = 32;

	sfFont* font;
	font = sfFont_createFromFile("arial.ttf");
	sfText *text;
	text = sfText_create();
	sfText_setFont(text, font);
	sfText_setCharacterSize(text, 30);
	sfText_setFillColor(text, sfColor_fromRGB(255, 255, 255));

	window = sfRenderWindow_create(vmode, "Window", sfDefaultStyle, sfContextDefault);
	if (window == NULL) {
		perror("Error Creating window");
		exit(1);
	}

	while (sfRenderWindow_isOpen(window)) {
		sfEvent e;
		sfEventType type;
		while (sfRenderWindow_pollEvent(window, &e)) {
			if (e.type == sfEvtClosed) {
				sfRenderWindow_close(window);
			}
			if (e.type == sfEvtKeyPressed) {
				switch (e.key.code) {
				case sfKeyF:
				{
					fetch(cpu);					
				} break;
				case sfKeyE:
				{
					decode(cpu);
				} break;
				default: break;
				}
			}
		}

		sfRenderWindow_clear(window, sfColor_fromRGB(0, 0, 0));
		
		sfRenderWindow_drawText(window, text, NULL);

		sfRenderWindow_display(window);
	}
	
	free(cpu->_D);
	free(cpu->_M);
	
	free(cpu);
	return 0;
}