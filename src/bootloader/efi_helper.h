#pragma once

#include <efi.h>
#include <stddef.h>

EFI_HANDLE IH;
EFI_SYSTEM_TABLE* ST;

void InitializeLib(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

int memcmp(const void* aptr, const void* bptr, size_t size) ;
void* memset(void* bufptr, int value, size_t size);
void* memmove(void* dstptr, const void* srcptr, size_t size);

EFI_STATUS Print(wchar_t* OutputString);
EFI_STATUS PrintNum(uint64_t x, int base);
EFI_STATUS PrintHex(uint64_t x);
EFI_STATUS PrintDec(uint64_t x);
EFI_STATUS PrintLn(wchar_t* OutputString);

EFI_FILE* LoadFile(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);