#include <kernel/arch/userspace.h>

#include <kernel/arch/cpu.h>

#define EFER_MSR    0xC0000080
#define STAR_MSR    0xC0000081
#define LSTAR_MSR   0xC0000082
#define SFMASK_MSR  0xC0000084

#define EFER_SCE    (1 << 0)
#define RFLAGS_IF   (1 << 9)

static void syscall_handler(void) {
    printf("WAZZUP\n");
    while (1);
}

void syscall_init(void) {
    load_msr(STAR_MSR, (0x08 << 38) | (0x08 << 32));
    load_msr(LSTAR_MSR, &syscall_handler);
    load_msr(SFMASK_MSR, RFLAGS_IF);
    uint64_t efer = read_msr(EFER_MSR);
    efer |= EFER_SCE;
    load_msr(EFER_MSR, efer);
}