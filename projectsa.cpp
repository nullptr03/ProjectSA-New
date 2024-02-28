#include "main.h"

void CProjectSA::InitPatch()
{
	// Patch for fps at DoGameState
	ARMHook::unprotect(0x5E4978);
	ARMHook::unprotect(0x5E4990);
	*(uint8_t*)(g_libGTASA+0x5E4978) = 90;
	*(uint8_t*)(g_libGTASA+0x5E4990) = 90;
}

void CProjectSA::Update()
{

}

void CProjectSA::InitHooks()
{
	CKeyGen::InjectHooks();
}