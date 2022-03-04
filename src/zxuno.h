/*
    ZEsarUX  ZX Second-Emulator And Released for UniX
    Copyright (C) 2013 Cesar Hernandez Bano

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef ZXUNO_H
#define ZXUNO_H


#include "cpu.h"

//#define ZXUNO_ROM_NAME "zxuno.rom"
#define ZXUNO_SPI_FLASH_NAME "zxuno.flash"

//extern z80_bit zxuno_enabled;

extern z80_byte zxuno_ports[];
extern z80_byte zxuno_read_port(z80_int puerto);
extern void zxuno_write_port(z80_int puerto, z80_byte value);
extern void hard_reset_cpu_zxuno(void);

//extern z80_byte *zxuno_sram_mem_table[];
//extern z80_byte *zxuno_bootm_memory_paged[];
//extern z80_byte *zxuno_no_bootm_memory_paged[];
//extern void mem_set_normal_pages_zxuno(void);

extern z80_byte *zxuno_sram_mem_table_new[];
extern z80_byte *zxuno_memory_paged_brandnew[];
extern void zxuno_set_memory_pages(void);
extern void zxuno_init_memory_tables(void);

extern z80_byte zxuno_dmareg[5][2];
extern z80_byte zxuno_index_nibble_dma_write[];
extern z80_byte zxuno_index_nibble_dma_read[];

extern void zxuno_handle_dma(void);

extern z80_int zxuno_dma_current_src;
extern z80_int zxuno_dma_current_dst;
extern z80_int zxuno_dma_current_len;

extern char *zxuno_dma_types[];
extern char *zxuno_dma_modes[];

extern z80_bit zxuno_dma_disabled;

extern z80_byte last_port_FC3B;

extern char zxuno_initial_64k_file[];

#define ZXUNO_BOOTM_DISABLED ( (zxuno_ports[0]&1)==0 )
#define ZXUNO_BOOTM_ENABLED ( (zxuno_ports[0]&1)==1 )
#define MACHINE_IS_ZXUNO_BOOTM_DISABLED (MACHINE_IS_ZXUNO && ZXUNO_BOOTM_DISABLED )
#define MACHINE_IS_ZXUNO_BOOTM_ENABLED  (MACHINE_IS_ZXUNO && ZXUNO_BOOTM_ENABLED )

#define ZXUNO_DIVEN_DISABLED ( (zxuno_ports[0]&2)==0 )
#define ZXUNO_DIVEN_ENABLED ( (zxuno_ports[0]&2)==1 )
#define MACHINE_IS_ZXUNO_DIVEN_DISABLED ( MACHINE_IS_ZXUNO && ZXUNO_DIVEN_DISABLED )

//Tamanyos en KB
#define ZXUNO_ROM_SIZE 16
#define ZXUNO_SRAM_SIZE 512
#define ZXUNO_SRAM_PAGES (ZXUNO_SRAM_SIZE/16)
#define ZXUNO_SPI_SIZE 4096
#define ZXUNO_SPI_SIZE_BYTES (ZXUNO_SPI_SIZE*1024)
#define ZXUNO_SPI_SIZE_BYTES_MASK (ZXUNO_SPI_SIZE_BYTES-1)

extern void zxuno_set_emulator_setting_diven(void);
extern void zxuno_set_emulator_setting_disd(void);
extern void zxuno_set_emulator_setting_i2kb(void);
extern void zxuno_set_emulator_setting_timing(void);
extern void zxuno_set_emulator_setting_contend(void);
extern void zxuno_set_emulator_setting_devcontrol_diay(void);
extern void zxuno_set_emulator_setting_devcontrol_ditay(void);
extern void zxuno_load_spi_flash(void);
extern void zxuno_p2a_write_page_port(z80_int puerto, z80_byte value);
extern z80_bit zxuno_flash_persistent_writes;
extern void zxuno_flush_flash_to_disk(void);
extern void zxuno_set_timing_48k(void);
//extern int zxuno_flash_operating_counter;

extern int zxuno_flash_must_flush_to_disk;

extern char zxuno_flash_spi_name[];

extern void zxuno_footer_print_flash_operating(void);

extern void zxuno_spi_clear_write_enable(void);

extern void delete_zxuno_flash_text(void);

//bit de Write enable de la SPI
#define ZXUNO_SPI_WEL 2


//extern void zxuno_page_ram(z80_byte bank);

extern z80_byte zxuno_spi_bus[];
extern int last_spi_write_address;
extern int last_spi_read_address;
extern z80_byte next_spi_read_byte;
extern z80_byte zxuno_spi_status_register;
extern z80_byte zxuno_spi_bus_index;

//extern void zxuno_mem_page_ram_p2a(void);
//extern void zxuno_mem_page_rom_p2a(void);

extern void zxuno_handle_raster_interrupts();
//extern z80_bit zxuno_disparada_raster;
extern int zxuno_core_id_indice;
extern void zxuno_set_emulator_setting_scandblctrl(void);

extern void zxuno_set_emulator_setting_ditimex(void);
extern void zxuno_set_emulator_setting_diulaplus(void);

extern z80_bit zxuno_deny_turbo_bios_boot;

extern z80_int zxuno_radasoffset;

extern z80_bit zxuno_radasoffset_high_byte;

extern z80_bit zxuno_flash_write_protection;

extern z80_byte zxuno_get_radaspalbank_offset(void);

extern void zxuno_set_emulador_settings(void);

extern int is_zxuno_chloe_mmu(void);

extern int zxuno_is_chloe_mmu(void);


#define ZXUNO_UART_DATA_REG 0xc6
#define ZXUNO_UART_STAT_REG 0xc7

extern void zxuno_uartbridge_enable(void);
extern void zxuno_uartbridge_disable(void);
extern z80_byte zxuno_uartbridge_readdata(void);
extern void zxuno_uartbridge_writedata(z80_byte value);
extern z80_byte zxuno_uartbridge_readstatus(void);

#define ZXUNO_UART_BYTE_RECEIVED_BIT 0x80
#define ZXUNO_UART_BYTE_TRANSMITTING_BIT 0x40

extern z80_byte zxuno_get_devcontrol_di7ffd(void);
extern z80_byte zxuno_get_devcontrol_di1ffd(void);


extern z80_byte *zxuno_get_vram_address(int vram);
extern int zxuno_is_prism_mode_enabled(void);
extern int zxuno_get_vram_mapped(void);
extern z80_byte *zxuno_get_vram_mapped_address(void);
extern void zxuno_prism_set_default_palette(void);
extern void zxuno_prism_set_color_palette(void);

//Componente de cada color de paleta. Por una parte, valor 24 bits tal cual la establece el usuario
//Por otra parte, indice a color de paleta de 15bits (paleta de tsconf)
//En principio de momento el valor de 24 bits se guarda ahi pero nadie hace uso de el
struct s_zxuno_prism_palette_item 
{
    z80_byte rgb[3]; //0=r, 1=g, 2=b
    int index_palette_15bit;
};

extern struct s_zxuno_prism_palette_item zxuno_prism_current_palette[];

extern int zxuno_prism_get_border_color(void);

extern void zxuno_prism_screen_store_scanline_rainbow(void);

extern int zxuno_is_prism_mapping_enabled(void);

extern void zxuno_prism_mode_splash(void);

extern void zxuno_load_additional_64k_block(void);

#endif
