// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016-2020 Toradex
 * Copyright (c) 2024 Savoir-Faire Linux
 */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CONFIG_TDX_CFG_BLOCK_IS_IN_EEPROM
#define ARCH_DMA_MINALIGN 4
#define CONFIG_SYS_CBSIZE 255

#define SERIAL_STR_LEN 8
#define MODULE_VER_STR_LEN 4 // V1.1
#define MODULE_REV_STR_LEN 3 // [A-Z] or #[26-99]

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define BITS_PER_LONG 32
#define GENMASK(h, l) \
	(((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define CMD_RET_SUCCESS 0
#define CMD_RET_FAILURE 1
#define CMD_RET_USAGE 2

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

struct toradex_hw {
	u16 ver_major;
	u16 ver_minor;
	u16 ver_assembly;
	u16 prodid;
};

struct toradex_eth_addr {
	u32 oui:24;
	u32 nic:24;
} __attribute__((__packed__));

struct toradex_som {
	const char *name;
	int is_enabled;
};

#define TAG_VALID	0xcf01
#define TAG_MAC		0x0000
#define TAG_CAR_SERIAL	0x0021
#define TAG_HW		0x0008
#define TAG_INVALID	0xffff

#define TAG_FLAG_VALID	0x1

#define TDX_EEPROM_ID_MODULE		0
#define TDX_EEPROM_ID_CARRIER		1

#if defined(CONFIG_TDX_CFG_BLOCK_IS_IN_MMC)
#define TDX_CFG_BLOCK_MAX_SIZE 512
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NAND)
#define TDX_CFG_BLOCK_MAX_SIZE 64
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NOR)
#define TDX_CFG_BLOCK_MAX_SIZE 64
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_EEPROM)
#define TDX_CFG_BLOCK_MAX_SIZE 64
#else
#error Toradex config block location not set
#endif

#define TDX_CFG_BLOCK_EXTRA_MAX_SIZE 64

struct toradex_tag {
	u32 len:14;
	u32 flags:2;
	u32 id:16;
};

struct pid4list {
	int pid4;
	char * const name;
};

struct toradex_hw tdx_hw_tag;
struct toradex_eth_addr tdx_eth_addr;
u32 tdx_serial;

u32 tdx_car_serial;
struct toradex_hw tdx_car_hw_tag;

char console_buffer[255];

#include "tdx-cfgdata.h"

static unsigned long dectoul(const char *cp, char **endp)
{
	return atol(cp);
};

static int cli_readline(const char *const prompt)
{
	printf("%s",prompt);

	if (fgets(console_buffer, sizeof(console_buffer), stdin) == NULL)
		return 0;

	return strlen(console_buffer);
}

static const char *get_board_assembly(u16 ver_assembly)
{
	static char ver_name[MODULE_REV_STR_LEN + 1];

	if (ver_assembly < 26) {
		ver_name[0] = (char)ver_assembly + 'A';
		ver_name[1] = '\0';
	} else {
		snprintf(ver_name, sizeof(ver_name),
			 "#%u", ver_assembly);
	}

	return ver_name;
}

static const char * const get_toradex_carrier_boards(int pid4)
{
	int i, index = 0;

	for (i = 1; i < ARRAY_SIZE(toradex_carrier_boards); i++) {
		if (pid4 == toradex_carrier_boards[i].pid4) {
			index = i;
			break;
		}
	}
	return toradex_carrier_boards[index].name;
}

static const char * const get_toradex_display_adapters(int pid4)
{
	int i, index = 0;

	for (i = 1; i < ARRAY_SIZE(toradex_display_adapters); i++) {
		if (pid4 == toradex_display_adapters[i].pid4) {
			index = i;
			break;
		}
	}
	return toradex_display_adapters[index].name;
}

static u32 get_serial_from_mac(struct toradex_eth_addr *eth_addr)
{
	int i;
	u32 oui = ntohl(eth_addr->oui) >> 8;
	u32 nic = ntohl(eth_addr->nic) >> 8;

	for (i = 0; i < ARRAY_SIZE(toradex_ouis); i++) {
		if (toradex_ouis[i] == oui)
			break;
	}

	return (u32)((i << 24) + nic);
}

void get_mac_from_serial(u32 tdx_serial, struct toradex_eth_addr *eth_addr)
{
	u8 oui_index = tdx_serial >> 24;
	u32 nic = tdx_serial & GENMASK(23, 0);
	u32 oui;

	if (oui_index >= ARRAY_SIZE(toradex_ouis)) {
		puts("Can't find OUI for this serial#\n");
		oui_index = 0;
	}

	oui = toradex_ouis[oui_index];

	eth_addr->oui = htonl(oui << 8);
	eth_addr->nic = htonl(nic << 8);
}

struct non_volatile_device {
	int type;
	const char* path;
	int offset;
};

const struct non_volatile_device nv_devs[] = {
	{TDX_EEPROM_ID_MODULE, "/dev/mmcblk2boot0", 0x3ffe00},
	{TDX_EEPROM_ID_CARRIER, "/sys/bus/nvmem/devices/3-00573/nvmem", 0},
	{TDX_EEPROM_ID_CARRIER, "/sys/bus/nvmem/devices/3-00513/nvmem", 0},
};

static const struct non_volatile_device* first_valid_nv_dev(u32 type)
{
	int fd;

	for (int i=0; i<ARRAY_SIZE(nv_devs); ++i) {
		const struct non_volatile_device* nv_dev = &nv_devs[i];
		if (nv_dev->type == type) {
			fd = open(nv_dev->path, O_RDONLY);
			if (fd != -1) {
				close(fd);
				return nv_dev;
			}
		}
	}
}

static int read_nv_device_data(const struct non_volatile_device* nv_dev,
	int offset, uint8_t *buf, int size)
{
	int fd;

	fd = open(nv_dev->path, O_RDONLY);
	if (fd == -1) {
		printf("error: cannot open '%s'.\n", nv_dev->path);
		return -1;
	}

	offset += nv_dev->offset;

	if (lseek(fd, offset, SEEK_SET) == -1) {
		printf("error: cannot seek to %i.\n", offset);
		close(fd);
		return -1;
	}

	if (read(fd, buf, size) != size) {
		printf("error: could not read %i bytes.\n", size);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

static int write_nv_device_data(const struct non_volatile_device* nv_dev,
	int offset, uint8_t *buf, int size)
{
	int fd;

	fd = open(nv_dev->path, O_RDWR);
	if (fd == -1) {
		printf("error: cannot open '%s'.\n", nv_dev->path);
		return -1;
	}

	offset += nv_dev->offset;

	if (lseek(fd, offset, SEEK_SET) == -1) {
		printf("error: cannot seek to %i.\n", offset);
		close(fd);
		return -1;
	}

	if (write(fd, buf, size) != size) {
		printf("error: could not write %i bytes.\n", size);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

static int read_tdx_cfg_block(const struct non_volatile_device* nv_dev)
{
	int ret = 0;
	u8 *config_block = NULL;
	struct toradex_tag *tag;
	size_t size = TDX_CFG_BLOCK_MAX_SIZE;
	int offset;

	/* Allocate RAM area for config block */
	config_block = memalign(ARCH_DMA_MINALIGN, size);
	if (!config_block) {
		printf("Not enough malloc space available!\n");
		return -ENOMEM;
	}

	memset(config_block, 0, size);

	ret = read_nv_device_data(nv_dev, 0x0, config_block,
				    TDX_CFG_BLOCK_MAX_SIZE);
	if (ret)
		goto out;

	/* Expect a valid tag first */
	tag = (struct toradex_tag *)config_block;
	if (tag->flags != TAG_FLAG_VALID || tag->id != TAG_VALID) {
		ret = -EINVAL;
		goto out;
	}
	offset = 4;

	/*
	 * check if there is enough space for storing tag and value of the
	 * biggest element
	 */
	while (offset + sizeof(struct toradex_tag) +
	       sizeof(struct toradex_hw) < TDX_CFG_BLOCK_MAX_SIZE) {
		tag = (struct toradex_tag *)(config_block + offset);
		offset += 4;
		if (tag->id == TAG_INVALID)
			break;

		if (tag->flags == TAG_FLAG_VALID) {
			switch (tag->id) {
			case TAG_MAC:
				memcpy(&tdx_eth_addr, config_block + offset,
				       6);

				tdx_serial = get_serial_from_mac(&tdx_eth_addr);
				break;
			case TAG_HW:
				memcpy(&tdx_hw_tag, config_block + offset, 8);
				break;
			}
		}

		/* Get to next tag according to current tags length */
		offset += tag->len * 4;
	}

	/* Cap product id to avoid issues with a yet unknown one */
	if (tdx_hw_tag.prodid >= ARRAY_SIZE(toradex_modules))
		tdx_hw_tag.prodid = 0;

out:
	free(config_block);
	return ret;
}

static int parse_assembly_string(char *string_to_parse, u16 *assembly)
{
	if (string_to_parse[3] >= 'A' && string_to_parse[3] <= 'Z')
		*assembly = string_to_parse[3] - 'A';
	else if (string_to_parse[3] == '#')
		*assembly = dectoul(&string_to_parse[4], NULL);
	else
		return -EINVAL;

	return 0;
}

static int get_cfgblock_interactive(void)
{
	char message[CONFIG_SYS_CBSIZE];
	int len = 0;
	int ret = 0;
	unsigned int prodid;
	int i;

	printf("Enabled modules:\n");
	for (i = 0; i < ARRAY_SIZE(toradex_modules); i++) {
		if (toradex_modules[i].is_enabled)
			printf(" %04d %s\n", i, toradex_modules[i].name);
	}

	sprintf(message, "Enter the module ID: ");
	len = cli_readline(message);

	prodid = dectoul(console_buffer, NULL);
	if (prodid >= ARRAY_SIZE(toradex_modules) || !toradex_modules[prodid].is_enabled) {
		printf("Parsing module id failed\n");
		return -1;
	}
	tdx_hw_tag.prodid = prodid;

	len = 0;
	while (len < 4) {
		sprintf(message, "Enter the module version (e.g. V1.1B or V1.1#26): V");
		len = cli_readline(message);
	}

	tdx_hw_tag.ver_major = console_buffer[0] - '0';
	tdx_hw_tag.ver_minor = console_buffer[2] - '0';

	ret = parse_assembly_string(console_buffer, &tdx_hw_tag.ver_assembly);
	if (ret) {
		printf("Parsing module version failed\n");
		return ret;
	}

	while (len < 8) {
		sprintf(message, "Enter module serial number: ");
		len = cli_readline(message);
	}

	tdx_serial = dectoul(console_buffer, NULL);

	return 0;
}

static int get_cfgblock_barcode(char *barcode, struct toradex_hw *tag,
				u32 *serial)
{
	char revision[3] = {barcode[6], barcode[7], '\0'};

	if (strlen(barcode) < 16) {
		printf("Argument too short, barcode is 16 chars long\n");
		return -1;
	}

	/* Get hardware information from the first 8 digits */
	tag->ver_major = barcode[4] - '0';
	tag->ver_minor = barcode[5] - '0';
	tag->ver_assembly = dectoul(revision, NULL);

	barcode[4] = '\0';
	tag->prodid = dectoul(barcode, NULL);

	/* Parse second part of the barcode (serial number */
	barcode += 8;
	*serial = dectoul(barcode, NULL);

	return 0;
}

static int write_tag(u8 *config_block, int *offset, int tag_id,
		     u8 *tag_data, size_t tag_data_size)
{
	struct toradex_tag *tag;

	if (!offset || !config_block)
		return -EINVAL;

	tag = (struct toradex_tag *)(config_block + *offset);
	tag->id = tag_id;
	tag->flags = TAG_FLAG_VALID;
	/* len is provided as number of 32bit values after the tag */
	tag->len = (tag_data_size + sizeof(u32) - 1) / sizeof(u32);
	*offset += sizeof(struct toradex_tag);
	if (tag_data && tag_data_size) {
		memcpy(config_block + *offset, tag_data,
		       tag_data_size);
		*offset += tag_data_size;
	}

	return 0;
}

int read_tdx_cfg_block_carrier(const struct non_volatile_device* nv_dev)
{
	int ret = 0;
	u8 *config_block = NULL;
	struct toradex_tag *tag;
	size_t size = TDX_CFG_BLOCK_EXTRA_MAX_SIZE;
	int offset;

	/* Allocate RAM area for carrier config block */
	config_block = memalign(ARCH_DMA_MINALIGN, size);
	if (!config_block) {
		printf("Not enough malloc space available!\n");
		return -ENOMEM;
	}

	memset(config_block, 0, size);

	ret = read_nv_device_data(nv_dev, 0x0, config_block,
				   size);
	if (ret)
		return ret;

	/* Expect a valid tag first */
	tag = (struct toradex_tag *)config_block;
	if (tag->flags != TAG_FLAG_VALID || tag->id != TAG_VALID) {
		ret = -EINVAL;
		goto out;
	}
	offset = 4;

	while (offset + sizeof(struct toradex_tag) +
	       sizeof(struct toradex_hw) < TDX_CFG_BLOCK_MAX_SIZE) {
		tag = (struct toradex_tag *)(config_block + offset);
		offset += 4;
		if (tag->id == TAG_INVALID)
			break;

		if (tag->flags == TAG_FLAG_VALID) {
			switch (tag->id) {
			case TAG_CAR_SERIAL:
				memcpy(&tdx_car_serial, config_block + offset,
				       sizeof(tdx_car_serial));
				break;
			case TAG_HW:
				memcpy(&tdx_car_hw_tag, config_block +
				       offset, 8);
				break;
			}
		}

		/* Get to next tag according to current tags length */
		offset += tag->len * 4;
	}
out:
	free(config_block);
	return ret;
}

static int get_cfgblock_carrier_interactive(void)
{
	char message[CONFIG_SYS_CBSIZE];
	int len;
	int ret = 0;

	printf("Supported carrier boards:\n");
	printf("%30s\t[ID]\n", "CARRIER BOARD NAME");
	for (int i = 0; i < ARRAY_SIZE(toradex_carrier_boards); i++)
		printf("%30s\t[%d]\n",
		       toradex_carrier_boards[i].name,
		       toradex_carrier_boards[i].pid4);

	sprintf(message, "Choose your carrier board (provide ID): ");
	len = cli_readline(message);
	tdx_car_hw_tag.prodid = dectoul(console_buffer, NULL);

	do {
		sprintf(message, "Enter carrier board version (e.g. V1.1B or V1.1#26): V");
		len = cli_readline(message);
	} while (len < 4);

	tdx_car_hw_tag.ver_major = console_buffer[0] - '0';
	tdx_car_hw_tag.ver_minor = console_buffer[2] - '0';

	ret = parse_assembly_string(console_buffer, &tdx_car_hw_tag.ver_assembly);
	if (ret) {
		printf("Parsing module version failed\n");
		return ret;
	}

	while (len < 8) {
		sprintf(message, "Enter carrier board serial number: ");
		len = cli_readline(message);
	}

	tdx_car_serial = dectoul(console_buffer, NULL);

	return 0;
}

static int do_cfgblock_carrier_create(const struct non_volatile_device* nv_dev,
	int force_overwrite, char *barcode)
{
	u8 *config_block;
	size_t size = TDX_CFG_BLOCK_EXTRA_MAX_SIZE;
	int offset = 0;
	int ret = CMD_RET_SUCCESS;
	int err;

	/* Allocate RAM area for config block */
	config_block = memalign(ARCH_DMA_MINALIGN, size);
	if (!config_block) {
		printf("Not enough malloc space available!\n");
		return CMD_RET_FAILURE;
	}

	memset(config_block, 0xff, size);
	err = read_tdx_cfg_block_carrier(nv_dev);
	if ((err == 0) && !force_overwrite) {
		char message[CONFIG_SYS_CBSIZE];

		sprintf(message, "A valid Toradex Carrier config block is present, still recreate? [y/N] ");

		if (!cli_readline(message))
			goto out;

		if (console_buffer[0] != 'y' &&
		    console_buffer[0] != 'Y')
			goto out;
	}

	if (!barcode) {
		err = get_cfgblock_carrier_interactive();
	} else {
		err = get_cfgblock_barcode(barcode, &tdx_car_hw_tag, &tdx_car_serial);
	}

	if (err) {
		ret = CMD_RET_FAILURE;
		goto out;
	}

	/* Valid Tag */
	write_tag(config_block, &offset, TAG_VALID, NULL, 0);

	/* Product Tag */
	write_tag(config_block, &offset, TAG_HW, (u8 *)&tdx_car_hw_tag,
		  sizeof(tdx_car_hw_tag));

	/* Serial Tag */
	write_tag(config_block, &offset, TAG_CAR_SERIAL, (u8 *)&tdx_car_serial,
		  sizeof(tdx_car_serial));

	memset(config_block + offset, 0, 32 - offset);
	err = write_nv_device_data(nv_dev, 0x0, config_block, size);
	if (err) {
		printf("Failed to write Toradex Extra config block: %d\n",
		       ret);
		ret = CMD_RET_FAILURE;
		goto out;
	}

	printf("Toradex Extra config block successfully written\n");

out:
	free(config_block);
	return ret;
}

static int do_cfgblock_create(const struct non_volatile_device* nv_dev,
	int force_overwrite, char *barcode)
{
	u8 *config_block;
	size_t size = TDX_CFG_BLOCK_MAX_SIZE;
	int offset = 0;
	int ret = CMD_RET_SUCCESS;
	int err;

	/* Allocate RAM area for config block */
	config_block = memalign(ARCH_DMA_MINALIGN, size);
	if (!config_block) {
		printf("Not enough malloc space available!\n");
		return CMD_RET_FAILURE;
	}

	memset(config_block, 0xff, size);

	err = read_tdx_cfg_block(nv_dev);
	if (err == 0) {
#if defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NAND)
		/*
		 * On NAND devices, recreation is only allowed if the page is
		 * empty (config block invalid...)
		 */
		printf("NAND erase block %d need to be erased before creating a Toradex config block\n",
		       CONFIG_TDX_CFG_BLOCK_OFFSET /
		       get_nand_dev_by_index(0)->erasesize);
		goto out;
#elif defined(CONFIG_TDX_CFG_BLOCK_IS_IN_NOR)
		/*
		 * On NOR devices, recreation is only allowed if the sector is
		 * empty and write protection is off (config block invalid...)
		 */
		printf("NOR sector at offset 0x%02x need to be erased and unprotected before creating a Toradex config block\n",
		       CONFIG_TDX_CFG_BLOCK_OFFSET);
		goto out;
#else
		if (!force_overwrite) {
			char message[CONFIG_SYS_CBSIZE];

			sprintf(message,
				"A valid Toradex config block is present, still recreate? [y/N] ");

			if (!cli_readline(message))
				goto out;

			if (console_buffer[0] != 'y' &&
			    console_buffer[0] != 'Y')
				goto out;
		}
#endif
	}

	/* Parse new Toradex config block data... */
	if (!barcode) {
		err = get_cfgblock_interactive();
	} else {
		err = get_cfgblock_barcode(barcode, &tdx_hw_tag, &tdx_serial);
	}
	if (err) {
		ret = CMD_RET_FAILURE;
		goto out;
	}

	/* Convert serial number to MAC address (the storage format) */
	get_mac_from_serial(tdx_serial, &tdx_eth_addr);

	/* Valid Tag */
	write_tag(config_block, &offset, TAG_VALID, NULL, 0);

	/* Product Tag */
	write_tag(config_block, &offset, TAG_HW, (u8 *)&tdx_hw_tag,
		  sizeof(tdx_hw_tag));

	/* MAC Tag */
	write_tag(config_block, &offset, TAG_MAC, (u8 *)&tdx_eth_addr,
		  sizeof(tdx_eth_addr));

	memset(config_block + offset, 0, 32 - offset);

	err = write_nv_device_data(nv_dev, 0x0, config_block,
				     TDX_CFG_BLOCK_MAX_SIZE);
	if (err) {
		printf("Failed to write Toradex config block: %d\n", ret);
		ret = CMD_RET_FAILURE;
		goto out;
	}

	printf("Toradex config block successfully written\n");

out:
	free(config_block);
	return ret;
}

static int do_cfgblock_carrier_print(const struct non_volatile_device* nv_dev)
{
	char tdx_car_serial_str[SERIAL_STR_LEN + 1];
	char tdx_car_rev_str[MODULE_VER_STR_LEN + MODULE_REV_STR_LEN + 1];
	const char *tdx_carrier_board_name;

	int ret = read_tdx_cfg_block_carrier(nv_dev);
	if (ret) {
		printf("Failed to load Toradex carrier config block: %d\n",
				ret);
		return CMD_RET_FAILURE;
	}

	tdx_carrier_board_name =
		get_toradex_carrier_boards(tdx_car_hw_tag.prodid);

	snprintf(tdx_car_serial_str, sizeof(tdx_car_serial_str),
			"%08u", tdx_car_serial);
	snprintf(tdx_car_rev_str, sizeof(tdx_car_rev_str),
			"V%1d.%1d%s",
			tdx_car_hw_tag.ver_major,
			tdx_car_hw_tag.ver_minor,
			get_board_assembly(tdx_car_hw_tag.ver_assembly));

	printf("carrier_prodid=\"%04d\"\n"
			"carrier_prodname=\"%s\"\n"
			"carrier_rev=\"%s\"\n"
			"carrier_serial=\"%s\"\n",
			tdx_car_hw_tag.prodid,
			tdx_carrier_board_name,
			tdx_car_rev_str,
			tdx_car_serial_str);

	return CMD_RET_SUCCESS;
}

static int do_cfgblock_print(const struct non_volatile_device* nv_dev)
{
	char tdx_serial_str[SERIAL_STR_LEN + 1];
	char tdx_board_rev_str[MODULE_VER_STR_LEN + MODULE_REV_STR_LEN + 1];

	int ret = read_tdx_cfg_block(nv_dev);
	if (ret) {
		printf("Failed to load Toradex config block: %d\n",
				ret);
		return CMD_RET_FAILURE;
	}

	snprintf(tdx_serial_str, sizeof(tdx_serial_str),
			"%08u", tdx_serial);
	snprintf(tdx_board_rev_str, sizeof(tdx_board_rev_str),
			"V%1d.%1d%s",
			tdx_hw_tag.ver_major,
			tdx_hw_tag.ver_minor,
			get_board_assembly(tdx_hw_tag.ver_assembly));

	printf("module_prodid=\"%04d\"\n"
			"module_prodname=\"%s\"\n"
			"module_rev=\"%s\"\n"
			"module_serial=\"%s\"\n",
			tdx_hw_tag.prodid,
			toradex_modules[tdx_hw_tag.prodid].name,
			tdx_board_rev_str,
			tdx_serial_str);

	return CMD_RET_SUCCESS;
}

static int do_cfgblock_carrier_list()
{
	for (int i = 0; i < ARRAY_SIZE(toradex_carrier_boards); i++)
		printf("%04d\t%s\n", toradex_carrier_boards[i].pid4,
							toradex_carrier_boards[i].name);

	return CMD_RET_SUCCESS;
}

static int do_cfgblock_list()
{
	for (int i = 0; i < ARRAY_SIZE(toradex_modules); i++) {
		if (toradex_modules[i].is_enabled)
			printf("%04d\t%s\n", i, toradex_modules[i].name);
	}

	return CMD_RET_SUCCESS;
}

static void usage()
{
	printf("Toradex config block handling commands\n"
	"create [-y] [barcode]         - (Re-)create Toradex config block\n"
	"create carrier [-y] [barcode] - (Re-)create Toradex Carrier config block\n"
	"print                         - Print Toradex config block in flash\n"
	"print carrier                 - Print Toradex Carrier config block in flash\n"
	"list                          - Print supported module IDs and name\n"
	"list carrier                  - Print supported carrier IDs and name\n");
}

int main(int argc, char *const argv[])
{
	int ret, i;
	int carrier = 0, force_overwrite = 0;
	char *barcode = NULL;
	const struct non_volatile_device* nv_dev;

	if (argc < 2) {
		usage();
		return CMD_RET_USAGE;
	}

	for (i=2; i<argc; ++i) {
		if (!strcmp(argv[i], "carrier")) {
			carrier = 1;
		} else if (!strcmp(argv[i], "-y")) {
			force_overwrite = 1;
		} else {
			barcode = argv[i];
		}
	}

	nv_dev = first_valid_nv_dev(carrier ? TDX_EEPROM_ID_CARRIER :
										TDX_EEPROM_ID_MODULE);
	if (!nv_dev)
		return -ENODEV;

	if (!strcmp(argv[1], "create")) {
		if (carrier) {
			return do_cfgblock_carrier_create(nv_dev, force_overwrite, barcode);
		} else {
			return do_cfgblock_create(nv_dev, force_overwrite, barcode);
		}
	} else if (!strcmp(argv[1], "print")) {
		if (carrier) {
			return do_cfgblock_carrier_print(nv_dev);
		} else {
			return do_cfgblock_print(nv_dev);
		}
	} else if (!strcmp(argv[1], "list")) {
		if (carrier) {
			return do_cfgblock_carrier_list();
		} else {
			return do_cfgblock_list();
		}
	}

	usage();
	return CMD_RET_USAGE;
}

