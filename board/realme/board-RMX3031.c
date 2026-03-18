//
// SPDX-FileCopyrightText: 2025 xCaptaiN09
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <board_ops.h>

#define VOLUME_UP 17
#define VOLUME_DOWN 1

void board_early_init(void) {
    printf("Entering early init for Realme X7 Max (RMX3031)\n");

    uint32_t addr = 0;

    // Fix fastboot unlock verify
    addr = SEARCH_PATTERN(LK_START, LK_END, 0xB508, 0xF7C6, 0xFB0B, 0xF7C6, 0xFAFD);
    if (addr) {
        printf("Found fastboot_unlock_verify at 0x%08X\n", addr);
        FORCE_RETURN(addr, 0);
    }

    // Volume key boot mode selection
    if (mtk_detect_key(VOLUME_UP)) {
        set_bootmode(BOOTMODE_RECOVERY);
    } else if (mtk_detect_key(VOLUME_DOWN)) {
        set_bootmode(BOOTMODE_FASTBOOT);
    }
}

void board_late_init(void) {
    printf("Entering late init for Realme X7 Max (RMX3031)\n");

    uint32_t addr = 0;

    // Remove orange state warning + 5s delay
    addr = SEARCH_PATTERN(LK_START, LK_END, 0xB508, 0x4B1C, 0x447B, 0x681B, 0x681B);
    if (addr) {
        printf("Found orange_state_warning at 0x%08X\n", addr);
        FORCE_RETURN(addr, 0);
    }

    // Remove dm-verity corruption warning
    addr = SEARCH_PATTERN(LK_START, LK_END, 0xB530, 0xB083, 0xAB02, 0x2200, 0x4604);
    if (addr) {
        printf("Found dm_verity_corruption at 0x%08X\n", addr);
        FORCE_RETURN(addr, 0);
    }

    if (get_bootmode() != BOOTMODE_RECOVERY
        && get_bootmode() != BOOTMODE_NORMAL) {
        show_bootmode(get_bootmode());
    }
}
