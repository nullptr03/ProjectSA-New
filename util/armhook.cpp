#include "main.h"
#include "ARMHook.h"
#include <sys/mman.h>

#define HOOK_PROC_ARM "\x01\xB4\x01\xB4\x01\x48\x01\x90\x01\xBD\x00\xBF\x00\x00\x00\x00"

uintptr_t arm_mmap_start 	= 0;
uintptr_t arm_mmap_end		= 0;
uintptr_t ARMHook::local_trampoline	= 0;
uintptr_t ARMHook::remote_trampoline = 0;

uintptr_t ARMHook::getLibraryAddress(const char* library)
{
    char filename[0xFF] = {0},
    buffer[2048] = {0};
    FILE *fp = 0;
    uintptr_t address = 0;

    sprintf(filename, "/proc/%d/maps", getpid());

    fp = fopen(filename, "rt");

    if(fp == 0) goto done;

    while(fgets(buffer, sizeof(buffer), fp))
    {
        if(strstr(buffer, library))
        {
            address = (uintptr_t)strtoul(buffer, 0, 16);
            break;
        }
    }

    done:

    if(fp)
      fclose(fp);

    return address;
}

uint8_t ARMHook::getByteSumFromAddress(uintptr_t dest, uint16_t count)
{
    uintptr_t destAddr = (g_libGTASA + dest);
	uint8_t sum = 0;
	uint16_t byte = 0;
	while (byte != count)
		sum ^= *(uint8_t*)(destAddr + byte++) & 0xCC;
	
	return sum;
}

uintptr_t ARMHook::getSymbolAddress(const char *library, const char *symbol)
{
    void *handle = dlopen(library, RTLD_LAZY);
    if(handle)
    {
        return (uintptr_t)dlsym(handle, symbol);
    }
    return 0;
}

void ARMHook::initialiseTrampolines(uintptr_t dest, uintptr_t size)
{
	local_trampoline   = g_libGTASA + dest;
	remote_trampoline  = local_trampoline + size;

	arm_mmap_start = (uintptr_t)mmap(0, PAGE_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	mprotect((void*)(arm_mmap_start & 0xFFFFF000), PAGE_SIZE, PROT_READ | PROT_EXEC | PROT_WRITE);
	arm_mmap_end = (arm_mmap_start + PAGE_SIZE);
}

void ARMHook::uninitializeTrampolines()
{
	local_trampoline = 0;
	remote_trampoline = 0;
	arm_mmap_start = 0;
	arm_mmap_end = 0;
}

void ARMHook::unprotect(uintptr_t ptr)
{
    uintptr_t destAddr = (g_libGTASA + ptr);
	mprotect((void*)(destAddr & 0xFFFFF000), PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
}

void ARMHook::writeMemory(uintptr_t dest, uintptr_t src, size_t size)
{
    uintptr_t destAddr = (g_libGTASA + dest);

	ARMHook::unprotect(destAddr);
	memcpy((void*)destAddr, (void*)src, size);
	cacheflush(destAddr, destAddr+size, 0);
}

void ARMHook::readMemory(uintptr_t dest, uintptr_t src, size_t size)
{
    uintptr_t destAddr = (g_libGTASA + dest);

	ARMHook::unprotect(src);
    memcpy((void*)destAddr, (void*)src, size);
}

void ARMHook::makeRET(uintptr_t dest)
{
    uintptr_t destAddr = (g_libGTASA + dest);

    ARMHook::writeMemory(destAddr, (uintptr_t)"\x00\x20\xF7\x46", 4);
}

void ARMHook::makeNOP(uintptr_t addr, unsigned int count)
{
    uintptr_t destAddr = (g_libGTASA + addr);

	ARMHook::unprotect(destAddr);

    for(uintptr_t ptr = destAddr; ptr != (destAddr+(count*2)); ptr += 2)
    {
        *(char*)ptr = 0x00;
        *(char*)(ptr+1) = 0x46;
    }
}

void ARMHook::makeJump(uintptr_t func, uintptr_t addr)
{
    uintptr_t destAddr = (g_libGTASA + addr);

	uint32_t code = ((destAddr-func-4) >> 12) & 0x7FF | 0xF000 | ((((destAddr-func-4) >> 1) & 0x7FF | 0xB800) << 16);
    ARMHook::writeMemory(func, (uintptr_t)&code, 4);
}

void ARMHook::writeMemHookProc(uintptr_t addr, uintptr_t func)
{
    uintptr_t destAddr = (g_libGTASA + addr);

    char code[16];
    memcpy(code, HOOK_PROC_ARM, 16);
    *(uint32_t*)&code[12] = (func | 1);
    ARMHook::writeMemory(destAddr, (uintptr_t)code, 16);
}

void ARMHook::installPLTHook(uintptr_t addr, uintptr_t func, uintptr_t *orig)
{
    uintptr_t destAddr = (g_libGTASA + addr);

    ARMHook::unprotect(destAddr);
    *orig = *(uintptr_t*)destAddr;
    *(uintptr_t*)destAddr = func;
}

void ARMHook::installHook(uintptr_t addr, uintptr_t func, uintptr_t *orig)
{
    uintptr_t destAddr = (g_libGTASA + addr);

    if(remote_trampoline < (local_trampoline + 0x10) || arm_mmap_end < (arm_mmap_start + 0x20))
        return std::terminate();

    ARMHook::readMemory(arm_mmap_start, destAddr, 4);
    ARMHook::writeMemHookProc(arm_mmap_start + 4, destAddr+4);
    *orig = arm_mmap_start + 1;
    arm_mmap_start += 32;

    ARMHook::makeJump(destAddr, local_trampoline);
    ARMHook::writeMemHookProc(local_trampoline, func);
    local_trampoline += 16;
}

void ARMHook::installHook(uintptr_t addr, uintptr_t func)
{
    uintptr_t destAddr = (g_libGTASA + addr);

    if(remote_trampoline < (local_trampoline + 0x10) || arm_mmap_end < (arm_mmap_start + 0x20))
        return std::terminate();

    ARMHook::readMemory(arm_mmap_start, destAddr, 4);
    ARMHook::writeMemHookProc(arm_mmap_start + 4, destAddr+4);
    arm_mmap_start += 32;

    ARMHook::makeJump(destAddr, local_trampoline);
    ARMHook::writeMemHookProc(local_trampoline, func);
    local_trampoline += 16;
}

void ARMHook::installMethodHook(uintptr_t addr, uintptr_t func)
{
    uintptr_t destAddr = (g_libGTASA + addr);

    ARMHook::unprotect(destAddr);
    *(uintptr_t*)destAddr = func;
}

void ARMHook::putCode(uintptr_t addr, uintptr_t point, uintptr_t func)
{
    uintptr_t destAddr = (g_libGTASA + addr);

    ARMHook::unprotect(destAddr+point);
    *(uintptr_t*)(destAddr+point) = func;
}

void ARMHook::injectCode(uintptr_t addr, uintptr_t func, int reg)
{
    uintptr_t destAddr = (g_libGTASA + addr);

    char injectCode[12];

    injectCode[0] = 0x01;
    injectCode[1] = 0xA0 + reg;
    injectCode[2] = (0x08 * reg) + reg;
    injectCode[3] = 0x68;
    injectCode[4] = 0x87 + (0x08 * reg);
    injectCode[5] = 0x46;
    injectCode[6] = injectCode[4];
    injectCode[7] = injectCode[5];
    
    *(uintptr_t*)&injectCode[8] = func;

    ARMHook::writeMemory(destAddr, (uintptr_t)injectCode, 12);
}