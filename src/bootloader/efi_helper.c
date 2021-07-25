#include "efi_helper.h"

void InitializeLib(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    IH = ImageHandle;
    ST = SystemTable;
}

int memcmp(const void* aptr, const void* bptr, size_t size) {
    const unsigned char* a = (const unsigned char*)aptr;
    const unsigned char* b = (const unsigned char*)bptr;
    for (size_t i = 0; i < size; i++) {
        if (a[i] < b[i]) {
            return -1;
        } else if (a[i] > b[i]) {
            return 1;
        }
    }

    return 0;
}

void* memset(void* bufptr, int value, size_t size) {
	unsigned char* buf = (unsigned char*) bufptr;
	for (size_t i = 0; i < size; i++)
		buf[i] = (unsigned char) value;
	return bufptr;
}

void* memmove(void* dstptr, const void* srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
    if (dst < src) {
	    for (size_t i = 0; i < size; i++)
		    dst[i] = src[i];
    } else if (dst > src) {
        for (size_t i = size; i > 0; i--)
            dst[i-1] = src[i-1];
    }
	return dstptr;
}

EFI_STATUS Print(wchar_t* OutputString) {
    return ST->ConOut->OutputString(ST->ConOut, OutputString);
}

EFI_STATUS PrintNum(uint64_t x, int base) {
    wchar_t out_buff[17];
    size_t i = 0;

    do {
        const wchar_t digit = (wchar_t) (x % base);
        if (digit >= 10) {
            out_buff[i++] = L'A' + digit - 10;
        } else {
            out_buff[i++] = L'0' + digit;
        }

        x /= base;
    } while (x);

    wchar_t swap;
    for (size_t j = 0; j < i/2; j++) {
		swap = out_buff[j];
		out_buff[j] = out_buff[i - j - 1];
		out_buff[i - j - 1] = swap;
	}

    out_buff[i] = L'\0';
    return Print(out_buff);
}

EFI_STATUS PrintHex(uint64_t x) {
    PrintNum(x, 16);
}

EFI_STATUS PrintDec(uint64_t x) {
    PrintNum(x, 10);
}

EFI_STATUS PrintLn(wchar_t* OutputString) {
    EFI_STATUS Status = Print(OutputString);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    return Print(L"\r\n");
}

EFI_FILE* LoadFile(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    EFI_FILE* LoadedFile;

    //Get the device our bootloader was loaded from
	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
    EFI_GUID EfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &EfiLoadedImageProtocolGuid, (void**)&LoadedImage);

    //Open that device
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
    EFI_GUID EfiSimpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);

    //No directory specified, open the root directory
    if (Directory == NULL) {
        FileSystem->OpenVolume(FileSystem, &Directory);
    }

    EFI_STATUS Status = Directory->Open(Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
    if (EFI_ERROR(Status)) {
        return NULL;
    }

    return LoadedFile;
}