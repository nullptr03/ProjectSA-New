#include "main.h"

uintptr_t g_libGTASA = 0;

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	__android_log_print(ANDROID_LOG_DEBUG, "AXLD", "Project SA library loaded! Build time: " __DATE__ " " __TIME__);

	g_libGTASA = ARMHook::getLibraryAddress(GTASA_LIBNAME);
	if(g_libGTASA == 0)
	{
		__android_log_print(ANDROID_LOG_ERROR, "AXLD", "Failed to find " GTASA_LIBNAME "!");
		return 0;
	}

	ARMHook::makeRET(0x3F6580);
	ARMHook::initialiseTrampolines(0x3F6584, 0x2D2);

	CProjectSA::InitPatch();
	CProjectSA::InitHooks();
	return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM *vm, void *reserved)
{
	__android_log_print(ANDROID_LOG_DEBUG, "AXLD", "Project SA library unloaded!");
	ARMHook::uninitializeTrampolines();
}