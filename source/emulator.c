#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct CPU
{
    unsigned short ram[65536];
    unsigned short stack[65536];

    unsigned short sp;
    unsigned short ip;
    unsigned short rp;

    unsigned short registers[5];

    unsigned short f_bit:1;
    unsigned short f_equals:1;
    unsigned short f_less:1;
    unsigned short f_greater:1;
    unsigned short running:1;
} CPU;

unsigned short GetVal(CPU * cpu, unsigned short arg_type, unsigned short arg)
{
    switch (arg_type)
    {
        case 0x00:  // Register
            return cpu->registers[arg];
        case 0x01:  // Memory Address
            return cpu->ram[arg];
        case 0x02:  // Immediate
            return arg;
        case 0x03:  // Pointer
            return cpu->ram[cpu->registers[arg]];
            break;
        case 0xffff:
            return 0xffff;
        default:
            printf("Incorrect Value Code\n\n");
    }
}

void Push(CPU * cpu, unsigned short val)
{
    cpu->stack[cpu->sp++] = val;
}
short Pop(CPU * cpu)
{
    return cpu->stack[--cpu->sp];
}

void Emulate(CPU * cpu)
{
    unsigned short opcode = cpu->ram[cpu->ip++];
    unsigned short arg1_t = cpu->ram[cpu->ip++];
    unsigned short arg1 = cpu->ram[cpu->ip++];
    unsigned short arg2_t = cpu->ram[cpu->ip++];
    unsigned short arg2_val = cpu->ram[cpu->ip++];
    unsigned short arg2 = GetVal(cpu, arg2_t, arg2_val);

    switch (opcode)
    {
        case 0x00:  // nop
            break;
        case 0x01:  // mov
            switch (arg1_t)
            {
                case 0x00:  // Register
                    cpu->registers[arg1] = arg2;
                    break;
                case 0x01:  // Memory Address
                    cpu->ram[arg1] = arg2;
                    break;
                case 0x03:  // Pointer
                    cpu->ram[cpu->registers[arg1]] = arg2;
                    break;
            }
            break;
        case 0x02:  // ptr
            cpu->ram[GetVal(cpu, arg1_t, arg1)] = arg2;
            break;
        case 0x03:  // push
            Push(cpu, GetVal(cpu, arg1_t, arg1));
            break;
        case 0x04:  // pop
            switch (arg1_t)
            {
                case 0x00:  // Register
                    cpu->registers[arg1] = Pop(cpu);
                    break;
                case 0x01:  // Memory Address
                    cpu->ram[arg1] = Pop(cpu);
                    break;
            }
            break;
        case 0x05:  // add
            switch (arg1_t)
            {
                case 0x00:  // Register
                    cpu->registers[arg1] += arg2;
                    break;
                case 0x01:  // Memory Address
                    cpu->ram[arg1] += arg2;
                    break;
                case 0x03:
                    cpu->ram[cpu->registers[arg1]] += arg2;
            }
            break;
        case 0x06:  // sub
            switch (arg1_t)
            {
                case 0x00:  // Register
                    cpu->registers[arg1] -= arg2;
                    break;
                case 0x01:  // Memory Address
                    cpu->ram[arg1] -= arg2;
                    break;
            }
            break;
        case 0x07:  // mul
            switch (arg1_t)
            {
                case 0x00:  // Register
                    cpu->registers[arg1] *= arg2;
                    break;
                case 0x01:  // Memory Address
                    cpu->ram[arg1] *= arg2;
                    break;
            }
            break;
        case 0x08:  // div
            switch (arg1_t)
            {
                case 0x00:  // Register
                    cpu->registers[arg1] /= arg2;
                    break;
                case 0x01:  // Memory Address
                    cpu->ram[arg1] /= arg2;
                    break;
            }
            break;
        case 0x09:  // cmp
            if (GetVal(cpu, arg1_t, arg1) == arg2)
                cpu->f_equals = 1;
            else
                cpu->f_equals = 0;

            if (GetVal(cpu, arg1_t, arg1) > arg2)
                cpu->f_greater = 1;
            else
                cpu->f_greater = 0;

            if (GetVal(cpu, arg1_t, arg1) < arg2)
                cpu->f_less = 1;
            else
                cpu->f_less = 0;
            break;
        case 0x0A:  // jmp
            cpu->ip = GetVal(cpu, arg1_t, arg1) * 5;
            break;
        case 0x0B:  // je
            if (cpu->f_equals == 1)
                cpu->ip = GetVal(cpu, arg1_t, arg1) * 5;
            break;
        case 0x0C:  // jne
            if (cpu->f_equals == 0)
                cpu->ip = GetVal(cpu, arg1_t, arg1) * 5;
            break;
        case 0x0D:  // jg
            if (cpu->f_greater == 1)
                cpu->ip = GetVal(cpu, arg1_t, arg1) * 5;
            break;
        case 0x0E:  // jl
            if (cpu->f_less == 1)
                cpu->ip = GetVal(cpu, arg1_t, arg1) * 5;
            break;
        case 0x0F:  // call
            Push(cpu, cpu->ip);
            cpu->ip = GetVal(cpu, arg1_t, arg1) * 5;
            break;
        case 0x10:  // ret
            cpu->ip = Pop(cpu);
            break;
        case 0x11:  // bit
            switch (arg1_t)
            {
                case 0x00:  // Register
                    cpu->f_equals = (cpu->registers[arg1] >> (7 - arg2)) & 1;
                    cpu->f_bit = (cpu->registers[arg1] >> (7 - arg2)) & 1;
                    break;
                case 0x01:  // Memory Address
                    cpu->f_equals = (cpu->ram[arg1] >> (7 - arg2)) & 1;
                    cpu->f_bit = (cpu->ram[arg1] >> (7 - arg2)) & 1;
                    break;
            }
            break;
        case 0x12:  // ldbit
            switch (arg1_t)
            {
                case 0x00:
                    cpu->registers[arg1] = cpu->f_bit;
                    break;
                case 0x01:
                    cpu->ram[arg1] = cpu->f_bit;
                    break;
                default:
                    break;
            }
            break;
        case 0x13:  // and
            switch (arg1_t)
            {
                case 0x00:
                    cpu->registers[arg1] &= arg2;
                    break;
            }
            break;
        case 0x14:  // or
            switch (arg1_t)
            {
                case 0x00:
                    cpu->registers[arg1] |= arg2;
                    break;
            }
            break;
        case 0x15:  // xor
            switch (arg1_t)
            {
                case 0x00:
                    cpu->registers[arg1] &= arg2;
                    break;
            }
            break;
        case 0x16:  // not
            switch (arg1_t)
            {
                case 0x00:
                    cpu->registers[arg1] = ~cpu->registers[arg1];
                    break;
                case 0x01:
                    cpu->ram[arg1] = ~cpu->ram[arg1];
                    break;
            }
            break;
        case 0x17:  // shl
            switch (arg1_t)
            {
                case 0x00:
                    cpu->registers[arg1] <<= arg2;
                    break;
                case 0x01:
                    cpu->ram[arg1] <<= arg2;
                    break;
            }
            break;
        case 0x18:  // shr
            switch (arg1_t)
            {
                case 0x00:
                    cpu->registers[arg1] >>= arg2;
                    break;
                case 0x01:
                    cpu->ram[arg1] >>= arg2;
                    break;
            }
            break;
        case 0xffff:
            cpu->running = 0;
            break;
        default:
            printf("Invalid Opcode\n\n");
            break;
    }
} 

int main(int argc, char *argv[])
{
    CPU cpu;

    FILE * fptr;

    long filelen;

    fptr = fopen(argv[1], "rb");       // Open the file in binary mode
    fseek(fptr, 0, SEEK_END);           // Jump to the end of the file
    filelen = ftell(fptr);              // Get the current byte offset in the file
    fseek(fptr, 0, SEEK_SET);           // Jump back to the beginning of the file

    fread(&cpu.ram, sizeof(short), filelen, fptr);

    for (int i = 0; i <= 0xffff; i++)
        cpu.ram[i] = (cpu.ram[i] << 8) | ((cpu.ram[i] >> 8) & 0xFF);

    cpu.running = 1;

    while (cpu.running)
    {
        printf("IP: %i\t", cpu.ip / 5);
        Emulate(&cpu);
        printf("0x%04x\n", cpu.registers[0]);
    }

    return 0;
}
