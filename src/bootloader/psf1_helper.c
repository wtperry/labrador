#include "psf1_helper.h"
#include "efi_helper.h"

PSF1_FONT* LoadPSF1Font(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    EFI_FILE* FontFile = LoadFile(Directory, Path, ImageHandle, SystemTable);
    if (FontFile == NULL) return NULL;

    PSF1_HEADER* FontHeader;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void**)&FontHeader);
    UINTN size = sizeof(PSF1_HEADER);
    FontFile->Read(FontFile, &size, FontHeader);

    if (FontHeader->magic[0] != PSF1_MAGIC0 || FontHeader->magic[1] != PSF1_MAGIC1) {
        return NULL;
    }

    UINTN NumChars = FontHeader->mode & PSF1_MODE512 ? 512 : 256;
    UINTN GlyphBufferSize = FontHeader->charsize * NumChars;
    
    void* GlyphBuffer;
    FontFile->SetPosition(FontFile, sizeof(PSF1_HEADER));
    SystemTable->BootServices->AllocatePool(EfiLoaderData, GlyphBufferSize, (void**)&GlyphBuffer);
    FontFile->Read(FontFile, &GlyphBufferSize, GlyphBuffer);

    PSF1_FONT* Font = NULL;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)Font);
    Font->psf1_header = FontHeader;
    Font->glyph_buffer = GlyphBuffer;

    return Font;
}