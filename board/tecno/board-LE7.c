//
// SPDX-FileCopyrightText: 2025 xCaptaiN09
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <board_ops.h>

#define VOLUME_UP 17
#define VOLUME_DOWN 1

#define BLDR_SPOOF_MAGIC 0xB1D05F00
#define BLDR_SPOOF_ADDR  (CONFIG_BOOTMODE_ADDRESS + 0x4)

static void cmd_bldr_spoof(const char *arg, void *data, unsigned size) {
    if (!arg || arg[0] == 0) {
        uint32_t val = READ32(BLDR_SPOOF_ADDR);
        fastboot_info(val == BLDR_SPOOF_MAGIC ? "bldr_spoof: enabled" : "bldr_spoof: disabled");
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
    printf("Entering early init for Tecno Pova 2 (LE7)\n");

    uint32_t addr = 0;

    if (READ32(BLDR_SPOOF_ADDR) == BLDR_SPOOF_MAGIC) {
        addr = SEARCH_PATTERN(LK_START, LK_END, 0x4B09, 0x447B, 0x685A, 0xB10A);
        if (addr) {
            printf("Found get_verified_boot_state at 0x%08X\n", addr);
            FORCE_RETURN(addr, 0);
        }
    }

    if (mtk_detect_key(VOLUME_UP)) {
        set_bootmode(BOOTMODE_RECOVERY);
    } else if (mtk_detect_key(VOLUME_DOWN)) {
        set_bootmode(BOOTMODE_FASTBOOT);
    }
}

void board_late_init(void) {
    printf("Entering late init for Tecno Pova 2 (LE7)\n");

    uint32_t addr = 0;

    fastboot_register("oem bldr_spoof", cmd_bldr_spoof, 1);

    addr = SEARCH_PATTERN(LK_START, LK_END, 0xB508, 0x4B1C, 0x447B, 0x681B, 0x681B);
    if (addr) {
        printf("Found orange_state_warning at 0x%08X\n", addr);
        FORCE_RETURN(addr, 0);
    }

    addr = SEARCH_PATTERN(LK_START, LK_END, 0x2802, 0xD000, 0x4770, 0xB538, 0x2432);
    if (addr) {
        printf("Found dm_verity_corruption at 0x%08X\n", addr);
        FORCE_RETURN(addr, 0);
    }

    if (get_bootmode() != BOOTMODE_RECOVERY
        && get_bootmode() != BOOTMODE_NORMAL) {
        show_bootmode(get_bootmode());
    }
}
