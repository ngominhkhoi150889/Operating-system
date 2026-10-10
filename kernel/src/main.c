#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

/* Ask Limine to use a supported base protocol revision. */
__attribute__((used, section(".limine_requests")))
static volatile uint64_t base_revision[] =
    LIMINE_BASE_REVISION(6);

/* Request a text terminal from the Limine bootloader. */
__attribute__((used, section(".limine_requests")))
static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST_ID,
    .revision = 0
};

/* Keep the Limine request section discoverable by the bootloader. */
__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t requests_start[] =
    LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t requests_end[] =
    LIMINE_REQUESTS_END_MARKER;

_Noreturn void cpu_halt(void);

void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED(base_revision)) {
        cpu_halt();
    }

    if (terminal_request.response == NULL ||
        terminal_request.response->terminal_count == 0) {
        cpu_halt();
    }

    struct limine_terminal *terminal =
        terminal_request.response->terminals[0];

    static const char message[] =
        "\n"
        "================================\n"
        "       Welcome to KhoiOS!\n"
        "================================\n"
        "Kernel architecture: x86_64\n"
        "Boot protocol: Limine\n"
        "Firmware target: UEFI\n"
        "\n"
        "KhoiOS has booted successfully.\n";

    terminal_request.response->write(
        terminal, message, sizeof(message) - 1
    );

    cpu_halt();
}
