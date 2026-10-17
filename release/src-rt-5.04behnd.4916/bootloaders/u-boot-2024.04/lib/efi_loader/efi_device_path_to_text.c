// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI device path interface
 *
 *  Copyright (c) 2017 Heinrich Schuchardt
 */

#include <blk.h>
#include <efi_loader.h>
#include <malloc.h>

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
	u16 *out, *dst;

	len = sizeof(u16) * (utf8_utf16_strlen(str) + 1);
	out = efi_alloc(len);
	if (!out)
		return NULL;
	dst = out;
	utf8_utf16_strcpy(&dst, str);
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
		int i, n;
		struct efi_device_path_vendor *vdp =
			(struct efi_device_path_vendor *)dp;

		s += sprintf(s, "VenHw(%pUl", &vdp->guid);
		n = (int)vdp->dp.length - sizeof(struct efi_device_path_vendor);
		/* Node must fit into MAX_NODE_LEN) */
		if (n > 0 && n < MAX_NODE_LEN / 2 - 22) {
			s += sprintf(s, ",");
			for (i = 0; i < n; ++i)
				s += sprintf(s, "%02x", vdp->vendor_data[i]);
		}
		s += sprintf(s, ")");
		break;
	}
	case DEVICE_PATH_SUB_TYPE_CONTROLLER: {
		struct efi_device_path_controller *cdp =
			(struct efi_device_path_controller *)dp;

		s += sprintf(s, "Ctrl(0x%0x)", cdp->controller_number);
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
	case DEVICE_PATH_SUB_TYPE_MSG_UART: {
		struct efi_device_path_uart *uart =
			(struct efi_device_path_uart *)dp;
		const char parity_str[6] = {'D', 'N', 'E', 'O', 'M', 'S'};
		const char *stop_bits_str[4] = { "D", "1", "1.5", "2" };

		s += sprintf(s, "Uart(%lld,%d,", uart->baud_rate,
			     uart->data_bits);

		/*
		 * Parity and stop bits can either both use keywords or both use
		 * numbers but numbers and keywords should not be mixed. Let's
		 * go for keywords as this is what EDK II does. For illegal
		 * values fall back to numbers.
		 */
		if (uart->parity < 6)
			s += sprintf(s, "%c,", parity_str[uart->parity]);
		else
			s += sprintf(s, "%d,", uart->parity);
		if (uart->stop_bits < 4)
			s += sprintf(s, "%s)", stop_bits_str[uart->stop_bits]);
		else
			s += sprintf(s, "%d)", uart->stop_bits);
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
		int i, n = sizeof(struct efi_mac_addr);
		struct efi_device_path_mac_addr *mdp =
			(struct efi_device_path_mac_addr *)dp;

		if (mdp->if_type <= 1)
			n = 6;
		s += sprintf(s, "MAC(");
		for (i = 0; i < n; ++i)
			s += sprintf(s, "%02x", mdp->mac.addr[i]);
		s += sprintf(s, ",%u)", mdp->if_type);

		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_USB_CLASS: {
		struct efi_device_path_usb_class *ucdp =
			(struct efi_device_path_usb_class *)dp;

		s += sprintf(s, "UsbClass(0x%x,0x%x,0x%x,0x%x,0x%x)",
			ucdp->vendor_id, ucdp->product_id,
			ucdp->device_class, ucdp->device_subclass,
			ucdp->device_protocol);

		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_SATA: {
		struct efi_device_path_sata *sdp =
			(struct efi_device_path_sata *) dp;

		s += sprintf(s, "Sata(0x%x,0x%x,0x%x)",
			     sdp->hba_port,
			     sdp->port_multiplier_port,
			     sdp->logical_unit_number);
		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_NVME: {
		struct efi_device_path_nvme *ndp =
			(struct efi_device_path_nvme *)dp;
		u32 ns_id;

		memcpy(&ns_id, &ndp->ns_id, sizeof(ns_id));
		s += sprintf(s, "NVMe(0x%x,", ns_id);

		/* Display byte 7 first, byte 0 last */
		for (int i = 0; i < 8; ++i)
			s += sprintf(s, "%s%02x", i ? "-" : "",
				     ndp->eui64[i ^ 7]);
		s += sprintf(s, ")");

		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_URI: {
		struct efi_device_path_uri *udp =
			(struct efi_device_path_uri *)dp;
		int n;

		n = (int)udp->dp.length - sizeof(struct efi_device_path_uri);

		s += sprintf(s, "Uri(");
		if (n > 0 && n < MAX_NODE_LEN - 6)
			s += snprintf(s, n, "%s", (char *)udp->uri);
		s += sprintf(s, ")");
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
 * Return:	next unused buffer address
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
		s += sprintf(s, "CDROM(%u,0x%llx,0x%llx)", cddp->boot_entry,
			     cddp->partition_start, cddp->partition_size);
		break;
	}
	case DEVICE_PATH_SUB_TYPE_VENDOR_PATH: {
		int i, n;
		struct efi_device_path_vendor *vdp =
			(struct efi_device_path_vendor *)dp;

		s += sprintf(s, "VenMedia(%pUl", &vdp->guid);
		n = (int)vdp->dp.length - sizeof(struct efi_device_path_vendor);
		/* Node must fit into MAX_NODE_LEN) */
		if (n > 0 && n < MAX_NODE_LEN / 2 - 24) {
			s += sprintf(s, ",");
			for (i = 0; i < n; ++i)
				s += sprintf(s, "%02x", vdp->vendor_data[i]);
		}
		s += sprintf(s, ")");
		break;
	}
	case DEVICE_PATH_SUB_TYPE_FILE_PATH: {
		struct efi_device_path_file_path *fp =
			(struct efi_device_path_file_path *)dp;
		u16 *buffer;
		int slen = dp->length - sizeof(*dp);

		/* two bytes for \0, extra byte if dp->length is odd */
		buffer = calloc(1, slen + 3);
		if (!buffer) {
			log_err("Out of memory\n");
			return s;
		}
		memcpy(buffer, fp->str, dp->length - sizeof(*dp));
		s += snprintf(s, MAX_NODE_LEN - 1, "%ls", buffer);
		free(buffer);
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
 * Return:		end of string
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
 * Return:		text representation of the device path
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
 * Return:		text representation of the device path
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
	while (device_path && str + MAX_NODE_LEN < buffer + MAX_PATH_LEN) {
		if (device_path->type == DEVICE_PATH_TYPE_END) {
			if (device_path->sub_type !=
			    DEVICE_PATH_SUB_TYPE_INSTANCE_END)
				break;
			*str++ = ',';
		} else {
			*str++ = '/';
			str = efi_convert_single_device_node_to_text(
							str, device_path);
		}
		*(u8 **)&device_path += device_path->length;
	}

	*str = 0;
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
