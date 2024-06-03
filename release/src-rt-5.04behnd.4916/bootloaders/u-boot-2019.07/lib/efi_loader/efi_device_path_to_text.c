// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI device path interface
 *
 *  Copyright (c) 2017 Heinrich Schuchardt
 */

#include <common.h>
#include <efi_loader.h>

#define MAC_OUTPUT_LEN 22
#define UNKNOWN_OUTPUT_LEN 23

#define MAX_NODE_LEN 512
#define MAX_PATH_LEN 1024

const efi_guid_t efi_guid_device_path_to_text_protocol =
		EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;

/**
 * efi_str_to_u16() - convert ASCII string to UTF-16
 *
 * A u16 buffer is allocated from pool. The ASCII string is copied to the u16
 * buffer.
 *
 * @str:	ASCII string
 * Return:	UTF-16 string. NULL if out of memory.
 */
static u16 *efi_str_to_u16(char *str)
{
	efi_uintn_t len;
	u16 *out;
	efi_status_t ret;

	len = strlen(str) + 1;
	ret = efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES, len * sizeof(u16),
				(void **)&out);
	if (ret != EFI_SUCCESS)
		return NULL;
	ascii2unicode(out, str);
	return out;
}

static char *dp_unknown(char *s, struct efi_device_path *dp)
{
	s += sprintf(s, "UNKNOWN(%04x,%04x)", dp->type, dp->sub_type);
	return s;
}

static char *dp_hardware(char *s, struct efi_device_path *dp)
{
	switch (dp->sub_type) {
	case DEVICE_PATH_SUB_TYPE_MEMORY: {
		struct efi_device_path_memory *mdp =
			(struct efi_device_path_memory *)dp;
		s += sprintf(s, "MemoryMapped(0x%x,0x%llx,0x%llx)",
			     mdp->memory_type,
			     mdp->start_address,
			     mdp->end_address);
		break;
	}
	case DEVICE_PATH_SUB_TYPE_VENDOR: {
		struct efi_device_path_vendor *vdp =
			(struct efi_device_path_vendor *)dp;
		s += sprintf(s, "VenHw(%pUl)", &vdp->guid);
		break;
	}
	default:
		s = dp_unknown(s, dp);
		break;
	}
	return s;
}

static char *dp_acpi(char *s, struct efi_device_path *dp)
{
	switch (dp->sub_type) {
	case DEVICE_PATH_SUB_TYPE_ACPI_DEVICE: {
		struct efi_device_path_acpi_path *adp =
			(struct efi_device_path_acpi_path *)dp;

		s += sprintf(s, "Acpi(PNP%04X,%d)", EISA_PNP_NUM(adp->hid),
			     adp->uid);
		break;
	}
	default:
		s = dp_unknown(s, dp);
		break;
	}
	return s;
}

static char *dp_msging(char *s, struct efi_device_path *dp)
{
	switch (dp->sub_type) {
	case DEVICE_PATH_SUB_TYPE_MSG_ATAPI: {
		struct efi_device_path_atapi *ide =
			(struct efi_device_path_atapi *)dp;
		s += sprintf(s, "Ata(%d,%d,%d)", ide->primary_secondary,
			     ide->slave_master, ide->logical_unit_number);
		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_SCSI: {
		struct efi_device_path_scsi *ide =
			(struct efi_device_path_scsi *)dp;
		s += sprintf(s, "Scsi(%u,%u)", ide->target_id,
			     ide->logical_unit_number);
		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_USB: {
		struct efi_device_path_usb *udp =
			(struct efi_device_path_usb *)dp;
		s += sprintf(s, "USB(0x%x,0x%x)", udp->parent_port_number,
			     udp->usb_interface);
		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_MAC_ADDR: {
		struct efi_device_path_mac_addr *mdp =
			(struct efi_device_path_mac_addr *)dp;

		if (mdp->if_type != 0 && mdp->if_type != 1)
			break;

		s += sprintf(s, "MAC(%02x%02x%02x%02x%02x%02x,0x%1x)",
			mdp->mac.addr[0], mdp->mac.addr[1],
			mdp->mac.addr[2], mdp->mac.addr[3],
			mdp->mac.addr[4], mdp->mac.addr[5],
			mdp->if_type);

		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_USB_CLASS: {
		struct efi_device_path_usb_class *ucdp =
			(struct efi_device_path_usb_class *)dp;

		s += sprintf(s, "USBClass(%x,%x,%x,%x,%x)",
			ucdp->vendor_id, ucdp->product_id,
			ucdp->device_class, ucdp->device_subclass,
			ucdp->device_protocol);

		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_SD:
	case DEVICE_PATH_SUB_TYPE_MSG_MMC: {
		const char *typename =
			(dp->sub_type == DEVICE_PATH_SUB_TYPE_MSG_SD) ?
					"SD" : "eMMC";
		struct efi_device_path_sd_mmc_path *sddp =
			(struct efi_device_path_sd_mmc_path *)dp;
		s += sprintf(s, "%s(%u)", typename, sddp->slot_number);
		break;
	}
	default:
		s = dp_unknown(s, dp);
		break;
	}
	return s;
}

/*
 * Convert a media device path node to text.
 *
 * @s		output buffer
 * @dp		device path node
 * @return	next unused buffer address
 */
static char *dp_media(char *s, struct efi_device_path *dp)
{
	switch (dp->sub_type) {
	case DEVICE_PATH_SUB_TYPE_HARD_DRIVE_PATH: {
		struct efi_device_path_hard_drive_path *hddp =
			(struct efi_device_path_hard_drive_path *)dp;
		void *sig = hddp->partition_signature;
		u64 start;
		u64 end;

		/* Copy from packed structure to aligned memory */
		memcpy(&start, &hddp->partition_start, sizeof(start));
		memcpy(&end, &hddp->partition_end, sizeof(end));

		switch (hddp->signature_type) {
		case SIG_TYPE_MBR: {
			u32 signature;

			memcpy(&signature, sig, sizeof(signature));
			s += sprintf(
				s, "HD(%d,MBR,0x%08x,0x%llx,0x%llx)",
				hddp->partition_number, signature, start, end);
			break;
			}
		case SIG_TYPE_GUID:
			s += sprintf(
				s, "HD(%d,GPT,%pUl,0x%llx,0x%llx)",
				hddp->partition_number, sig, start, end);
			break;
		default:
			s += sprintf(
				s, "HD(%d,0x%02x,0,0x%llx,0x%llx)",
				hddp->partition_number, hddp->partmap_type,
				start, end);
			break;
		}

		break;
	}
	case DEVICE_PATH_SUB_TYPE_CDROM_PATH: {
		struct efi_device_path_cdrom_path *cddp =
			(struct efi_device_path_cdrom_path *)dp;
		s += sprintf(s, "CDROM(0x%x)", cddp->boot_entry);
		break;
	}
	case DEVICE_PATH_SUB_TYPE_FILE_PATH: {
		struct efi_device_path_file_path *fp =
			(struct efi_device_path_file_path *)dp;
		int slen = (dp->length - sizeof(*dp)) / 2;
		if (slen > MAX_NODE_LEN - 2)
			slen = MAX_NODE_LEN - 2;
		s += sprintf(s, "%-.*ls", slen, fp->str);
		break;
	}
	default:
		s = dp_unknown(s, dp);
		break;
	}
	return s;
}

/*
 * Converts a single node to a char string.
 *
 * @buffer		output buffer
 * @dp			device path or node
 * @return		end of string
 */
static char *efi_convert_single_device_node_to_text(
		char *buffer,
		struct efi_device_path *dp)
{
	char *str = buffer;

	switch (dp->type) {
	case DEVICE_PATH_TYPE_HARDWARE_DEVICE:
		str = dp_hardware(str, dp);
		break;
	case DEVICE_PATH_TYPE_ACPI_DEVICE:
		str = dp_acpi(str, dp);
		break;
	case DEVICE_PATH_TYPE_MESSAGING_DEVICE:
		str = dp_msging(str, dp);
		break;
	case DEVICE_PATH_TYPE_MEDIA_DEVICE:
		str = dp_media(str, dp);
		break;
	case DEVICE_PATH_TYPE_END:
		break;
	default:
		str = dp_unknown(str, dp);
	}

	*str = '\0';
	return str;
}

/*
 * This function implements the ConvertDeviceNodeToText service of the
 * EFI_DEVICE_PATH_TO_TEXT_PROTOCOL.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * device_node		device node to be converted
 * display_only		true if the shorter text representation shall be used
 * allow_shortcuts	true if shortcut forms may be used
 * @return		text representation of the device path
 *			NULL if out of memory of device_path is NULL
 */
static uint16_t EFIAPI *efi_convert_device_node_to_text(
		struct efi_device_path *device_node,
		bool display_only,
		bool allow_shortcuts)
{
	char str[MAX_NODE_LEN];
	uint16_t *text = NULL;

	EFI_ENTRY("%p, %d, %d", device_node, display_only, allow_shortcuts);

	if (!device_node)
		goto out;
	efi_convert_single_device_node_to_text(str, device_node);

	text = efi_str_to_u16(str);

out:
	EFI_EXIT(EFI_SUCCESS);
	return text;
}

/*
 * This function implements the ConvertDevicePathToText service of the
 * EFI_DEVICE_PATH_TO_TEXT_PROTOCOL.
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * device_path		device path to be converted
 * display_only		true if the shorter text representation shall be used
 * allow_shortcuts	true if shortcut forms may be used
 * @return		text representation of the device path
 *			NULL if out of memory of device_path is NULL
 */
static uint16_t EFIAPI *efi_convert_device_path_to_text(
		struct efi_device_path *device_path,
		bool display_only,
		bool allow_shortcuts)
{
	uint16_t *text = NULL;
	char buffer[MAX_PATH_LEN];
	char *str = buffer;

	EFI_ENTRY("%p, %d, %d", device_path, display_only, allow_shortcuts);

	if (!device_path)
		goto out;
	while (device_path &&
	       str + MAX_NODE_LEN < buffer + MAX_PATH_LEN) {
		*str++ = '/';
		str = efi_convert_single_device_node_to_text(str, device_path);
		device_path = efi_dp_next(device_path);
	}

	text = efi_str_to_u16(buffer);

out:
	EFI_EXIT(EFI_SUCCESS);
	return text;
}

/* helper for debug prints.. efi_free_pool() the result. */
uint16_t *efi_dp_str(struct efi_device_path *dp)
{
	return EFI_CALL(efi_convert_device_path_to_text(dp, true, true));
}

const struct efi_device_path_to_text_protocol efi_device_path_to_text = {
	.convert_device_node_to_text = efi_convert_device_node_to_text,
	.convert_device_path_to_text = efi_convert_device_path_to_text,
};
