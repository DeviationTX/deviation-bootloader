/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2013 Andrzej Surowiec <emeryth@gmail.com>
 * Copyright (C) 2013 Pavol Rusnak <stick@gk2.sk>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include "ramdisk.h"
#include "common.h"

#define ROM_ADDRESS 0x0800f000
#define ROM_SIZE_IN_KB 196      // Must be a multiple of cluster size
#define FLASH_PAGE_SIZE 2048 // <=128kB: 1024,  <=512kB: 2048

#define ROM_END     (((ROM_SIZE_IN_KB) * 1024) + ROM_ADDRESS)

#define WBVAL(x) ((x) & 0xFF), (((x) >> 8) & 0xFF)
#define QBVAL(x) ((x) & 0xFF), (((x) >> 8) & 0xFF),\
		 (((x) >> 16) & 0xFF), (((x) >> 24) & 0xFF)

// filesystem size is 512kB (1024 * SECTOR_SIZE)
#define SECTOR_SIZE		512
#define BYTES_PER_SECTOR	SECTOR_SIZE
#define SECTORS_PER_CLUSTER	8
#define RESERVED_SECTORS	1
#define FAT_COPIES		2
#define ROOT_ENTRY_LENGTH	32
#define CLUSTER_SIZE            (SECTOR_SIZE * SECTORS_PER_CLUSTER)
#define ROOT_ENTRIES		(SECTOR_SIZE / ROOT_ENTRY_LENGTH)
#define DEBUG_SIZE_IN_SECTORS   SECTORS_PER_CLUSTER
#define NUM_NONDATA_SECTORS     (RESERVED_SECTORS + FAT_COPIES + DEBUG_SIZE_IN_SECTORS + 1)    // Boot + 2* FAT + debug + root
#define SECTOR_COUNT		(2 * ((ROM_SIZE_IN_KB * 1024) / SECTOR_SIZE) + NUM_NONDATA_SECTORS)
#define ROM_SIZE_IN_CLUSTERS    (ROM_SIZE_IN_KB * 1024 / CLUSTER_SIZE)
#define ROM_SIZE_IN_SECTORS     (ROM_SIZE_IN_KB * 1024 / SECTOR_SIZE)
#define ROM_START_SECTOR        (RESERVED_SECTORS + FAT_COPIES + 1) // Boot + 2*FAT + Root

unsigned erased = 0;
unsigned debug_ptr = 0;

uint8_t BootSector[] = {
	0xEB, 0x3C, 0x90,					// code to jump to the bootstrap code
	'm', 'k', 'd', 'o', 's', 'f', 's', 0x00,		// OEM ID
	WBVAL(BYTES_PER_SECTOR),				// bytes per sector
	SECTORS_PER_CLUSTER,					// sectors per cluster
	WBVAL(RESERVED_SECTORS),				// # of reserved sectors (1 boot sector)
	FAT_COPIES,						// FAT copies (2)
	WBVAL(ROOT_ENTRIES),					// root entries (512)
	WBVAL(SECTOR_COUNT),					// total number of sectors
	0xF8,							// media descriptor (0xF8 = Fixed disk)
	0x01, 0x00,						// sectors per FAT (1)
	0x20, 0x00,						// sectors per track (32)
	0x40, 0x00,						// number of heads (64)
	0x00, 0x00, 0x00, 0x00,					// hidden sectors (0)
	0x00, 0x00, 0x00, 0x00,					// large number of sectors (0)
	0x00,							// drive number (0)
	0x00,							// reserved
	0x29,							// extended boot signature
	0x69, 0x17, 0xAD, 0x53,					// volume serial number
	'R', 'A', 'M', 'D', 'I', 'S', 'K', ' ', ' ', ' ', ' ',	// volume label
	'F', 'A', 'T', '1', '2', ' ', ' ', ' '			// filesystem type
};

uint8_t FatSector[SECTOR_SIZE];
uint8_t DirSector[SECTOR_SIZE];
static uint8_t debug[CLUSTER_SIZE/8][8];

static const u8 dir_oldfw[32] = {
        'F',  'I',  'R',  'M',  'W',  'A',  'R',  'E',
        'O',  'L',  'D',
	0x01,							// attribute byte
	0x00,							// reserved for Windows NT
	0x00,							// creation millisecond
	0xCE, 0x01,						// creation time
	0x86, 0x41,						// creation date
	0x86, 0x41,						// last access date
	0x00, 0x00,						// reserved for FAT32
	0xCE, 0x01,						// last write time
	0x86, 0x41,						// last write date
	WBVAL(2 + ROM_SIZE_IN_CLUSTERS),			// start cluster
	QBVAL(ROM_SIZE_IN_KB * 1024)				// file size in bytes
  };
static const u8 dir_debug[32] = {
        'D',  'E',  'B',  'U',  'G',  ' ',  ' ',  ' ',
        ' ',  ' ',  ' ',
	0x01,							// attribute byte
	0x00,							// reserved for Windows NT
	0x00,							// creation millisecond
	0xCE, 0x01,						// creation time
	0x86, 0x41,						// creation date
	0x86, 0x41,						// last access date
	0x00, 0x00,						// reserved for FAT32
	0xCE, 0x01,						// last write time
	0x86, 0x41,						// last write date
	WBVAL(2 + 2 * ROM_SIZE_IN_CLUSTERS),			// start cluster
	QBVAL(sizeof(debug))					// file size in bytes
  };


static char hex_nibble(u8 val)
{
    if (val < 10)
        return '0' + val;
    return 'A' + val - 10;
}


static void debugmsg(char *msg, unsigned value)
{
    debug[debug_ptr][0] = msg[0];
    debug[debug_ptr][1] = msg[1];
    debug[debug_ptr][2] = ' ';
    debug[debug_ptr][3] = hex_nibble(0x0F & (value >> 12));
    debug[debug_ptr][4] = hex_nibble(0x0F & (value >> 8));
    debug[debug_ptr][5] = hex_nibble(0x0F & (value >> 4));
    debug[debug_ptr][6] = hex_nibble(0x0F & (value >> 0));
    debug[debug_ptr][7] = '\n';
    if (debug_ptr < CLUSTER_SIZE / 8)
        debug_ptr++;
}
static void link_fat_cluster(unsigned idx, unsigned next_clstr)
{
    unsigned offset = idx * 3 / 2;
    if (idx & 0x01) {
        next_clstr <<= 4;
    }
    FatSector[offset] |= next_clstr & 0xff;
    FatSector[offset+1] |= (next_clstr >> 8) & 0xff;
}

static void build_fat()
{
    memset(FatSector, 0, sizeof(FatSector));
    link_fat_cluster(0, 0xff8);
    link_fat_cluster(1, 0xfff);

    // data can start at cluster 2
    unsigned cluster = 2 + ROM_SIZE_IN_CLUSTERS;
    unsigned end_cluster = cluster + ROM_SIZE_IN_CLUSTERS;
     
    while (cluster < end_cluster - 1) {
        link_fat_cluster(cluster, cluster + 1);
        cluster++;
    }
    link_fat_cluster(cluster, 0xfff);
    link_fat_cluster(cluster+1, 0xfff); //Debug
}

int ramdisk_init(void)
{
        memset(DirSector, 0, sizeof(DirSector));
        memcpy(DirSector, dir_oldfw, sizeof(dir_oldfw));
        memcpy(DirSector + 32, dir_debug, sizeof(dir_debug));
        memset(debug, 0, sizeof(debug));
	flash_unlock();
	return 0;
}

int ramdisk_read(uint32_t lba, uint8_t *copy_to)
{
	debugmsg("r ", lba);
	memset(copy_to, 0, SECTOR_SIZE);
        erased = 0;
	switch (lba) {
		case 0: // sector 0 is the boot sector
			memcpy(copy_to, BootSector, sizeof(BootSector));
			copy_to[SECTOR_SIZE - 2] = 0x55;
			copy_to[SECTOR_SIZE - 1] = 0xAA;
			break;
		case 1: // sector 1 is FAT 1st copy
		case 2: // sector 2 is FAT 2nd copy
			memcpy(copy_to, FatSector, sizeof(FatSector));
			break;
		case 3: // sector 3 is the directory entry
			memcpy(copy_to, DirSector, sizeof(DirSector));
			break;
		default:
			// ignore reads outside of the data section
			if (lba >= ROM_START_SECTOR && lba < ROM_START_SECTOR + 2*ROM_SIZE_IN_SECTORS) {
                                if (lba > ROM_START_SECTOR + ROM_SIZE_IN_SECTORS) {
                                    //2 copies of ROM are provided
                                    lba -= ROM_SIZE_IN_SECTORS;
                                }
				uint32_t *memory_ptr= (uint32_t*)(ROM_ADDRESS + SECTOR_SIZE * (lba - ROM_START_SECTOR));
				uint8_t *data = copy_to;
				for (int i = 0; i < SECTOR_SIZE; i += 4) {
					*(uint32_t *)data = *memory_ptr++;
					data += 4;
				}
			}
			if (lba >= ROM_START_SECTOR + 2*ROM_SIZE_IN_SECTORS && lba < ROM_START_SECTOR + 2*ROM_SIZE_IN_SECTORS + DEBUG_SIZE_IN_SECTORS) {
                                u8 *ptr = (u8*)debug;
				memcpy(copy_to, ptr + (lba - ROM_START_SECTOR + 2*ROM_SIZE_IN_SECTORS) * SECTOR_SIZE, SECTOR_SIZE);
			}
			break;
	}
	return 0;
}

int ramdisk_write(uint32_t lba, const uint8_t *copy_from)
{
	debugmsg("w ", lba);
	switch (lba) {
		case 0: // sector 0 is the boot sector - READ-ONLY
			break;
		case 1: // sector 1 is FAT 1st copy
			build_fat();
		case 2: // sector 2 is FAT 2nd copy
			break;
		case 3: // sector 3 is the directory entry
			memcpy(DirSector, copy_from, sizeof(DirSector));
			break;
		default:
			// ignore reads outside of the data section
			if (lba >= ROM_START_SECTOR && lba < ROM_START_SECTOR + ROM_SIZE_IN_SECTORS) {
				uint32_t *memory_ptr= (uint32_t*)(ROM_ADDRESS + SECTOR_SIZE * (lba - ROM_START_SECTOR));
				if (! erased) {
					debugmsg(" e", lba);
					uint32_t *page_address = (uint32_t *)ROM_ADDRESS;
					while ((uint32_t)page_address < ROM_END) {
						flash_erase_page((uint32_t)page_address);
						uint32_t flash_status = flash_get_status_flags();
						if(flash_status != FLASH_SR_EOP)
							return 1;
						page_address += FLASH_PAGE_SIZE;
					}
					erased = 1;
				}
				for(unsigned i=0; i<SECTOR_SIZE; i += 4)
				{
					/*programming word data*/
					flash_program_word((uint32_t)memory_ptr, *((uint32_t*)(copy_from + i)));
					uint32_t flash_status = flash_get_status_flags();
					if(flash_status != FLASH_SR_EOP)
						return 1;

					/*verify if correct data is programmed*/
					if(*memory_ptr != *((uint32_t*)(copy_from + i)))
						return 1;
					memory_ptr++;
				}
			}
			break;

        return 0;
}
				
	(void)lba;
	(void)copy_from;
	// ignore writes
	return 0;
}

int ramdisk_blocks(void)
{
	return SECTOR_COUNT;
}
