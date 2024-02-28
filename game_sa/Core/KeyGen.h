/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

// CRC-32-IEEE
// https://www.xilinx.com/support/documentation/application_notes/xapp209.pdf

class CKeyGen {
public:
    static const uint32_t keyTable[256];

public:
    static void InjectHooks();

    [[nodiscard]] static uint32_t AppendStringToKey(uint32_t key, const char* str);
    [[nodiscard]] static uint32_t GetKey(const char* str);
    [[nodiscard]] static uint32_t GetKey(const char* str, int32_t size);
    [[nodiscard]] static uint32_t GetUppercaseKey(const char* str);
};
