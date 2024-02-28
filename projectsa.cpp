#include "main.h"

void CProjectSA::InitPatch()
{
	// Patch for fps at DoGameState
	ARMHook::unprotect(g_libGTASA+0x5E4978);
	ARMHook::unprotect(g_libGTASA+0x5E4990);
	*(uint8_t*)(g_libGTASA+0x5E4978) = 90;
	*(uint8_t*)(g_libGTASA+0x5E4990) = 90;
}

void CProjectSA::Update()
{

}

void CProjectSA::InitHooks()
{
	// ARMHook::installPLTHook(g_libGTASA+0x6710C4, (uintptr_t)CProjectSA::Idle_hook, (uintptr_t*)&CProjectSA::Idle_orig);
	// ARMHook::installHook(g_libGTASA+0x26BF20, (uintptr_t)CProjectSA::AND::RunThread_hook, (uintptr_t*)&CProjectSA::AND::RunThread_orig);
}