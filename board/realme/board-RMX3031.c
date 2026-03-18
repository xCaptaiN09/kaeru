//
// SPDX-FileCopyrightText: 2025 xCaptaiN09
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <board_ops.h>

#define VOLUME_UP 17
#define VOLUME_DOWN 1

// Spoof flag stored in unused RAM area near bootmode
#define BLDR_SPOOF_MAGIC 0xB1D05F00
#define BLDR_SPOOF_ADDR  (CONFIG_BOOTMODE_ADDRESS + 0x4)

static void cmd_bldr_spoof(const char *arg, void *data, unsigned size) {
    if (!arg || arg[0] == 0) {
        uint32_t val = READ32(BLDR_SPOOF_ADDR);
        if (val == BLDR_SPOOF_MAGIC) {
            fastboot_info("bldr_spoof: enabled");
        } else {
            fastboot_info("bldr_spoof: disabled");
        }
        fastboot_okay("");
        return;
    }

    if (arg[0] == 'o' && arg[1] == 'n') {
        WRITE32(BLDR_SPOOF_ADDR, BLDR_SPOOF_MAGIC);
        fastboot_info("bldr_spoof: enabled (reboot to apply)");
        fastboot_okay("");
    } else if (arg[0] == 'o' && arg[1] == 'f') {
        WRITE32(BLDR_SPOOF_ADDR, 0x0);
        fastboot_info("bldr_spoof: disabled (reboot to apply)");
        fastboot_okay("");
    } else {
        fastboot_info("usage: fastboot oem bldr_spoof [on|off]");
        fastboot_okay("");
    }
}

void board_early_init(void) {
    printf("Entering early init for Realme X7 Max (RMX3031)\n");

    uint32_t addr = 0;

    // Fix fastboot unlock verify
    addr = SEARCH_PATTERN(LK_START, LK_END, 0xB508, 0xF7C6, 0xFB0B, 0xF7C6, 0xFAFD);
    if (addr) {
        printf("Found fastboot_unlock_verify at 0x%08X\n", addr);
        FORCE_RETURN(addr, 0);
    }

    // Spoof verified boot state to GREEN if bldr_spoof is enabled
    if (READ32(BLDR_SPOOF_ADDR) == BLDR_SPOOF_MAGIC) {
        addr = SEARCH_PATTERN(LK_START, LK_END, 0x4B09, 0x447B, 0x685A, 0xB10A);
        if (addr) {
            printf("Found get_verified_boot_state at 0x%08X\n", addr);
            FORCE_RETURN(addr, 0);
        }
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

    // Register bldr_spoof fastboot command
    fastboot_register("oem bldr_spoof", cmd_bldr_spoof, 1);

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
