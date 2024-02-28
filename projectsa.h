#pragma once

#include <vector>
#include <thread>
#include <mutex>

class CProjectSA
{
public:
    static void InitPatch();
    static void InitHooks();
    static void Update();
};