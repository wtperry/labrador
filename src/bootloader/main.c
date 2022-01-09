#include <efi.h>
#include <bootloader/boot_spec.h>
#include <bootloader/psf1.h>
#include <stdbool.h>
#include <stddef.h>

#include "elf64.h"
#include "psf1_helper.h"
#include "efi_helper.h"

#define PAGE_ADDR_MASK 0x000ffffffffff000

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITEABLE (1 << 1)

typedef struct {
    EFI_PHYSICAL_ADDRESS VirtAddr;
    EFI_PHYSICAL_ADDRESS PhysAddr;
    size_t NumPages;
} PageMapping;

void MapPages(PageMapping* ToMap, size_t NumToMap, EFI_PHYSICAL_ADDRESS FreePages[]) {
    //disable write protect
    uint64_t cr0;
    asm("movq %%cr0, %0" : "=r" (cr0));
    cr0 = cr0 & (~0x10000);
    asm("movq %0, %%cr0" : : "r" (cr0));

    EFI_PHYSICAL_ADDRESS PML4T;
    asm("movq %%cr3, %0" : "=r" (PML4T));
    PML4T = PML4T & 0xFFFFFFFFFFFFF000;

    size_t PagesMapped = 0;

    for (size_t i = 0; i < NumToMap; i++) {
        EFI_PHYSICAL_ADDRESS BaseVirtAddr = ToMap[i].VirtAddr;
        EFI_PHYSICAL_ADDRESS BasePhysAddr = ToMap[i].PhysAddr;
        size_t NumPages = ToMap[i].NumPages;

        for (size_t j = 0; j < NumPages; j++) {
            EFI_PHYSICAL_ADDRESS VirtAddr = (EFI_PHYSICAL_ADDRESS)((uint64_t)BaseVirtAddr + j * 4096);
            EFI_PHYSICAL_ADDRESS PhysAddr = (EFI_PHYSICAL_ADDRESS)((uint64_t)BasePhysAddr + j * 4096);

            uint16_t PML4T_Index = ((uint64_t)VirtAddr >> 39) & 0x1ff;
            uint16_t PDPT_Index = ((uint64_t)VirtAddr >> 30) & 0x1ff;
            uint16_t PDT_Index = ((uint64_t)VirtAddr >> 21) & 0x1ff;
            uint16_t PT_Index = ((uint64_t)VirtAddr >> 12) & 0x1ff;

            uint64_t* PML4E = ((uint64_t*)PML4T + PML4T_Index);
            EFI_PHYSICAL_ADDRESS PDPT;
            if (!(*PML4E & PAGE_PRESENT)) {
                //No PDPT for this address, make one
                PDPT = FreePages[PagesMapped++];
                //zero out the new PDPT
                memset((void*)PDPT, 0, 4096);
                //add to the PML4E
                *PML4E = (((uint64_t)PDPT & PAGE_ADDR_MASK) | PAGE_PRESENT | PAGE_WRITEABLE);
            } else {
                PDPT = (EFI_PHYSICAL_ADDRESS)(*PML4E & PAGE_ADDR_MASK);
            }

            uint64_t* PDPE = ((uint64_t*)PDPT + PDPT_Index);
            EFI_PHYSICAL_ADDRESS PDT;
            if (!(*PDPE & PAGE_PRESENT)) {
                //No PDT for this address, make one
                PDT = FreePages[PagesMapped++];
                //zero out the new PDT
                memset((void*)PDT, 0, 4096);
                //add to the PDPE
                *PDPE = (((uint64_t)PDT & PAGE_ADDR_MASK) | PAGE_PRESENT | PAGE_WRITEABLE);
            } else {
                PDT = (EFI_PHYSICAL_ADDRESS)(*PDPE & PAGE_ADDR_MASK);
            }

            uint64_t* PDE = ((uint64_t*)PDT + PDT_Index);
            EFI_PHYSICAL_ADDRESS PT;
            if (!(*PDE & PAGE_PRESENT)) {
                //No PT for this address, make one
                PT = FreePages[PagesMapped++];
                //zero out the new PT
                memset((void*)PT, 0, 4096);
                //add to the PDE
                *PDE = (((uint64_t)PT & PAGE_ADDR_MASK) | PAGE_PRESENT | PAGE_WRITEABLE);
            } else {
                PT = (EFI_PHYSICAL_ADDRESS)(*PDE & PAGE_ADDR_MASK);
            }

            uint64_t* PTE = ((uint64_t*)PT + PT_Index);
            *PTE = (((uint64_t)PhysAddr & PAGE_ADDR_MASK) | PAGE_PRESENT | PAGE_WRITEABLE);
        }
    }

    //Reload Paging
    asm("movq %0, %%cr3" : : "r" (PML4T));
}

void BuildMemoryMap(boot_info* efi_boot_info, EFI_MEMORY_DESCRIPTOR* EfiMemoryMap, UINTN EfiMemoryMapSize, UINTN EfiDescriptorSize) {
    UINTN MemoryMapEntries = EfiMemoryMapSize / EfiDescriptorSize;
    mmap_entry* mmap = &efi_boot_info->mmap;

    for (size_t i = 0; i < MemoryMapEntries; i++) {
        EFI_MEMORY_DESCRIPTOR* EfiEntry = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)EfiMemoryMap + i * EfiDescriptorSize);
        mmap[i].address = EfiEntry->PhysicalStart;
        mmap[i].size = EfiEntry->NumberOfPages * 4096;

        switch (EfiEntry->Type)
        {
        case EfiReservedMemoryType:
        case EfiUnusableMemory:
        //case EfiUnacceptedMemoryType:
        case EfiACPIMemoryNVS:
        case EfiPalCode:
            mmap[i].type = MMAP_RESERVED;
            break;

        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiLoaderCode:
        case EfiLoaderData:
            mmap[i].type = MMAP_LOADER;
            break;

        case EfiConventionalMemory:
        //case EfiPersistentMemory:
            mmap[i].type = MMAP_FREE;
            break;

        case EfiACPIReclaimMemory:
            mmap[i].type = MMAP_ACPI;
            break;

        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
            mmap[i].type = MMAP_UEFI_RUNTIME;
            break;

        case EfiMemoryMappedIO:
        case EfiMemoryMappedIOPortSpace:
            mmap[i].type = MMAP_MMIO;
            break;

        default:
            mmap[i].type = MMAP_OTHER;
        }
    }

    //Sort Memory Map (Bubble sort)
    for (size_t x = 0; x < MemoryMapEntries; x++) {
        bool sorted = true;
        for (size_t i = 0; i < MemoryMapEntries - 1; i++) {
            if (mmap[i].address > mmap[i+1].address) {
                sorted = false;
                mmap_entry temp = mmap[i+1];
                mmap[i+1] = mmap[i];
                mmap[i] = temp;
            }
        }

        if (sorted) break;
    }

    //Combine adjacent entries of the same type
    for (size_t i = 0; i < MemoryMapEntries-1; i++) {
        if (mmap[i].type == mmap[i+1].type && (mmap[i].address + mmap[i].size) == mmap[i+1].address) {
            mmap[i].size += mmap[i+1].size;
            memmove(&mmap[i+1], &mmap[i+2], sizeof(mmap_entry) * (MemoryMapEntries - i - 2));
            MemoryMapEntries--;
            i--;
        }
    }

    efi_boot_info->num_mmap_entries = MemoryMapEntries;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    
    EFI_STATUS Status;

    boot_info* efi_boot_info;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, MAX_BOOT_INFO_SIZE, (void**)&efi_boot_info);

    EFI_GUID GopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop;

    Status = SystemTable->BootServices->LocateProtocol(&GopGuid, NULL, (void**)&Gop);
    if (EFI_ERROR(Status)) {
        PrintLn(L"Unable to locate GOP");
    }

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN SizeOfInfo;
    UINTN NumModes;
    UINTN NativeMode;

    Status = Gop->QueryMode(Gop, Gop->Mode==NULL?0:Gop->Mode->Mode, &SizeOfInfo, &Info);

    if (Status == EFI_NOT_STARTED) {
        Status = Gop->SetMode(Gop, 0);
    }

    if (EFI_ERROR(Status)) {
        PrintLn(L"Unable to get native mode");
    } else {
        NativeMode = Gop->Mode->Mode;
        NumModes = Gop->Mode->MaxMode;
    }

    efi_boot_info->fb_ptr = (void*) Gop->Mode->FrameBufferBase;
    efi_boot_info->fb_size = Gop->Mode->FrameBufferSize;
    efi_boot_info->fb_width = Info->HorizontalResolution;
    efi_boot_info->fb_height = Info->VerticalResolution;
    efi_boot_info->fb_scanline = Info->PixelsPerScanLine;

    for (size_t i = 0; i < NumModes; i++) {
        Status = Gop->QueryMode(Gop, i, &SizeOfInfo, &Info);
        Print(L"Mode: ");
        PrintDec(i);
        Print(L" ");
        PrintDec(Info->HorizontalResolution);
        Print(L"x");
        PrintDec(Info->VerticalResolution);
        Print(L" Pixel Format ");
        PrintHex(Info->PixelFormat);
        i == NativeMode ? Print(L" (current)") : 0;
        PrintLn(L"");
    }

    EFI_FILE* KernelFile = LoadFile(NULL, L"Labrador\\kernel.elf", ImageHandle, SystemTable);
    if (KernelFile == NULL) {
        PrintLn(L"Could not load kernel");
    } else {
        PrintLn(L"Kernel file found");
    }

    Elf64_Ehdr header;
    {
        EFI_GUID EfiFileInfoGuid = EFI_FILE_INFO_ID;
        UINTN FileInfoSize;
        EFI_FILE_INFO* FileInfo;
        KernelFile->GetInfo(KernelFile, &EfiFileInfoGuid, &FileInfoSize, NULL);
        SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
        KernelFile->GetInfo(KernelFile, &EfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

        UINTN size = sizeof(header);
        KernelFile->Read(KernelFile, &size, &header);
    }

    if (
        memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) ||
        header.e_ident[EI_CLASS] != ELFCLASS64 ||
        header.e_ident[EI_DATA] != ELFDATA2LSB ||
        header.e_type != ET_EXEC ||
        header.e_machine != EM_X86_64 ||
        header.e_version != EV_CURRENT
    ) {
        PrintLn(L"Kernel file not ELF64");
    } else {
        PrintLn(L"Kernel header verified");
    }

    Elf64_Phdr* phdrs;
    {
        KernelFile->SetPosition(KernelFile, header.e_phoff);
        UINTN size = header.e_phnum * header.e_phentsize;
        SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**)&phdrs);
        KernelFile->Read(KernelFile, &size, phdrs);
    }

    PageMapping* SegmentsToMap;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, header.e_phnum * sizeof(PageMapping) + 1, (void**)&SegmentsToMap);
    size_t NumSegments = 0;

    for (
        Elf64_Phdr* phdr = phdrs;
        (char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize;
        phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)
    ) {
        if (phdr->p_type == PT_LOAD) {
            UINTN pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000; //size of segment in pages rounded up
            Elf64_Addr segment;
            Status = SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, pages, &segment);
           
            KernelFile->SetPosition(KernelFile, phdr->p_offset);
            UINTN size = phdr->p_filesz;
            KernelFile->Read(KernelFile, &size, (void*)segment);

            SegmentsToMap[NumSegments].PhysAddr = segment;
            SegmentsToMap[NumSegments].VirtAddr = phdr->p_vaddr;
            SegmentsToMap[NumSegments].NumPages = pages;
            NumSegments++;
        }
    }

    KernelFile->Close(KernelFile);

    #define PAGES_TO_RESERVE 20
    EFI_PHYSICAL_ADDRESS FreePages[PAGES_TO_RESERVE];
    for (size_t i = 0; i < PAGES_TO_RESERVE; i++) {
        SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &FreePages[i]);
    }

    PrintLn(L"Kernel Loaded");

    PSF1_FONT* Font = LoadPSF1Font(NULL, L"Labrador\\zap-light16.psf", ImageHandle, SystemTable);
    if (Font == NULL) {
        PrintLn(L"Could not load font");
    } else {
        PrintLn(L"Font file loaded");
    }

    efi_boot_info->font = Font;

    efi_boot_info->rsdp = NULL;
    EFI_GUID Acpi2TableGuid = ACPI_20_TABLE_GUID;
    for (size_t i = 0; i < SystemTable->NumberOfTableEntries; i++) {
        if (!memcmp(&SystemTable->ConfigurationTable[i].VendorGuid, &Acpi2TableGuid, sizeof(EFI_GUID))) {
            if (!memcmp("RSD PTR ", SystemTable->ConfigurationTable[i].VendorTable, 8)) {
                efi_boot_info->rsdp = SystemTable->ConfigurationTable[i].VendorTable;
                PrintLn(L"RSDP Found");
                break;
            }
        }
    }

    EFI_FILE* InitrdFile = LoadFile(NULL, L"Labrador\\initrd.tar", ImageHandle, SystemTable);
    if (InitrdFile == NULL) {
        PrintLn(L"Could not load initrd");
    } else {
        PrintLn(L"Initrd file found");
    }

    EFI_PHYSICAL_ADDRESS InitrdPtr;
    UINTN InitrdSize;

    void* InitrdVirtPtr;

    {
        EFI_GUID FileInfoGuid = EFI_FILE_INFO_ID;
        EFI_FILE_INFO* InitrdInfo;
        UINTN InfoSize = sizeof(*InitrdInfo) + sizeof(WCHAR)*255;
        SystemTable->BootServices->AllocatePool(EfiLoaderData, InfoSize, (void**)&InitrdInfo);
        Status = InitrdFile->GetInfo(InitrdFile, &FileInfoGuid, &InfoSize, InitrdInfo);

        InitrdSize = InitrdInfo->FileSize;
        PrintLn(L"Initrd Size:");
        PrintDec(InitrdSize);
        PrintLn(L"");
        
        SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, (InitrdSize-1)/4096 + 1, &InitrdPtr);
        Status = InitrdFile->Read(InitrdFile, &InitrdSize, InitrdPtr);

        if (EFI_ERROR(Status)) {
            PrintLn(L"Reading Initrd Failed");
        } else {
            PrintLn(L"Successfully loaded Initrd");
            PrintHex(InitrdPtr);
            PrintLn(L"");
        }
    }

    InitrdVirtPtr = SegmentsToMap[NumSegments-1].VirtAddr + SegmentsToMap[NumSegments-1].NumPages * 4096;

    SegmentsToMap[NumSegments].NumPages = (InitrdSize-1)/4096 + 1;
    SegmentsToMap[NumSegments].PhysAddr = InitrdPtr;
    SegmentsToMap[NumSegments].VirtAddr = InitrdVirtPtr;
    NumSegments++;

    efi_boot_info->initrd = InitrdVirtPtr;
    efi_boot_info->initrd_size = InitrdSize;

    efi_boot_info->first_free_page = InitrdVirtPtr + (InitrdSize + 4095)/4096 * 4096;

    EFI_MEMORY_DESCRIPTOR* EfiMemoryMap;
    UINTN EfiMemoryMapSize;
    UINTN EfiMapKey;
    UINTN EfiDescriptorSize;
    UINT32 EfiDescriptorVersion;

    EfiMemoryMapSize = 0;
    EfiMemoryMap = NULL;

    Status = SystemTable->BootServices->GetMemoryMap(&EfiMemoryMapSize, EfiMemoryMap, &EfiMapKey, &EfiDescriptorSize, &EfiDescriptorVersion);
    Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, EfiMemoryMapSize + 2 * EfiDescriptorSize, (void**)&EfiMemoryMap);
    do {
        Status = SystemTable->BootServices->GetMemoryMap(&EfiMemoryMapSize, EfiMemoryMap, &EfiMapKey, &EfiDescriptorSize, &EfiDescriptorVersion);
    } while(EFI_ERROR(Status));

    Status = SystemTable->BootServices->ExitBootServices(ImageHandle, EfiMapKey);
    if (EFI_ERROR(Status)) {
        PrintLn(L"ExitBootServices() Failed");
        while(1);
    }

    MapPages(SegmentsToMap, NumSegments, FreePages);

    BuildMemoryMap(efi_boot_info, EfiMemoryMap, EfiMemoryMapSize, EfiDescriptorSize);

    __attribute__((sysv_abi)) void (*KernelStart)(boot_info*) = ((__attribute__((sysv_abi)) void (*)(boot_info*)) header.e_entry);

    KernelStart(efi_boot_info);

    while (1) {
        asm("hlt");
    }
}