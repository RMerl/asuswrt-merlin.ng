// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2021 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include "containers.h"
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <iphlpapi.h>
#include <initguid.h>
#include <devguid.h>
#include <ddk/ndisguid.h>
#include <wireguard.h>
#include <hashtable.h>

#define IPC_SUPPORTS_KERNEL_INTERFACE

static bool have_cached_kernel_interfaces;
static struct hashtable cached_kernel_interfaces;
static const DEVPROPKEY devpkey_name = DEVPKEY_WG_NAME;
extern bool is_win7;

static int kernel_get_wireguard_interfaces(struct string_list *list)
{
	HDEVINFO dev_info = SetupDiGetClassDevsExW(&GUID_DEVCLASS_NET, is_win7 ? L"ROOT\\WIREGUARD" : L"SWD\\WireGuard", NULL, DIGCF_PRESENT, NULL, NULL, NULL);
	bool will_have_cached_kernel_interfaces = true;

	if (dev_info == INVALID_HANDLE_VALUE) {
		errno = EACCES;
		return -errno;
	}

	for (DWORD i = 0;; ++i) {
		DWORD buf_len;
		WCHAR adapter_name[MAX_ADAPTER_NAME];
		SP_DEVINFO_DATA dev_info_data = { .cbSize = sizeof(SP_DEVINFO_DATA) };
		DEVPROPTYPE prop_type;
		ULONG status, problem_code;
		char *interface_name;
		struct hashtable_entry *entry;

		if (!SetupDiEnumDeviceInfo(dev_info, i, &dev_info_data)) {
			if (GetLastError() == ERROR_NO_MORE_ITEMS)
				break;
			continue;
		}

		if (!SetupDiGetDevicePropertyW(dev_info, &dev_info_data, &devpkey_name,
					       &prop_type, (PBYTE)adapter_name,
					       sizeof(adapter_name), NULL, 0) ||
				prop_type != DEVPROP_TYPE_STRING)
			continue;
		adapter_name[_countof(adapter_name) - 1] = L'0';
		if (!adapter_name[0])
			continue;
		buf_len = WideCharToMultiByte(CP_UTF8, 0, adapter_name, -1, NULL, 0, NULL, NULL);
		if (!buf_len)
			continue;
		interface_name = malloc(buf_len);
		if (!interface_name)
			continue;
		buf_len = WideCharToMultiByte(CP_UTF8, 0, adapter_name, -1, interface_name, buf_len, NULL, NULL);
		if (!buf_len) {
			free(interface_name);
			continue;
		}

		if (CM_Get_DevNode_Status(&status, &problem_code, dev_info_data.DevInst, 0) == CR_SUCCESS &&
		    (status & (DN_DRIVER_LOADED | DN_STARTED)) == (DN_DRIVER_LOADED | DN_STARTED))
			string_list_add(list, interface_name);

		entry = hashtable_find_or_insert_entry(&cached_kernel_interfaces, interface_name);
		free(interface_name);
		if (!entry)
			continue;

		if (SetupDiGetDeviceInstanceIdW(dev_info, &dev_info_data, NULL, 0, &buf_len) || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			continue;
		entry->value = calloc(sizeof(WCHAR), buf_len);
		if (!entry->value)
			continue;
		if (!SetupDiGetDeviceInstanceIdW(dev_info, &dev_info_data, entry->value, buf_len, &buf_len)) {
			free(entry->value);
			entry->value = NULL;
			continue;
		}

		will_have_cached_kernel_interfaces = true;
	}
	SetupDiDestroyDeviceInfoList(dev_info);
	have_cached_kernel_interfaces = will_have_cached_kernel_interfaces;
	return 0;
}

static HANDLE kernel_interface_handle(const char *iface)
{
	HDEVINFO dev_info;
	WCHAR *interfaces = NULL;
	HANDLE handle;

	if (have_cached_kernel_interfaces) {
		struct hashtable_entry *entry = hashtable_find_entry(&cached_kernel_interfaces, iface);
		if (entry) {
			DWORD buf_len;
			if (CM_Get_Device_Interface_List_SizeW(
				&buf_len, (GUID *)&GUID_DEVINTERFACE_NET, (DEVINSTID_W)entry->value,
				CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS)
				goto err_hash;
			interfaces = calloc(buf_len, sizeof(*interfaces));
			if (!interfaces)
				goto err_hash;
			if (CM_Get_Device_Interface_ListW(
				(GUID *)&GUID_DEVINTERFACE_NET, (DEVINSTID_W)entry->value, interfaces, buf_len,
				CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS || !interfaces[0]) {
				free(interfaces);
				interfaces = NULL;
				goto err_hash;
			}
			handle = CreateFileW(interfaces, GENERIC_READ | GENERIC_WRITE,
					     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
					     OPEN_EXISTING, 0, NULL);
			free(interfaces);
			if (handle == INVALID_HANDLE_VALUE)
				goto err_hash;
			return handle;
err_hash:
			errno = EACCES;
			return NULL;
		}
	}

	dev_info = SetupDiGetClassDevsExW(&GUID_DEVCLASS_NET, is_win7 ? L"ROOT\\WIREGUARD" : L"SWD\\WireGuard", NULL, DIGCF_PRESENT, NULL, NULL, NULL);
	if (dev_info == INVALID_HANDLE_VALUE)
		return NULL;

	for (DWORD i = 0; !interfaces; ++i) {
		bool found;
		DWORD buf_len;
		WCHAR *buf, adapter_name[MAX_ADAPTER_NAME];
		SP_DEVINFO_DATA dev_info_data = { .cbSize = sizeof(SP_DEVINFO_DATA) };
		DEVPROPTYPE prop_type;
		char *interface_name;

		if (!SetupDiEnumDeviceInfo(dev_info, i, &dev_info_data)) {
			if (GetLastError() == ERROR_NO_MORE_ITEMS)
				break;
			continue;
		}

		if (!SetupDiGetDevicePropertyW(dev_info, &dev_info_data, &devpkey_name,
					       &prop_type, (PBYTE)adapter_name,
					       sizeof(adapter_name), NULL, 0) ||
				prop_type != DEVPROP_TYPE_STRING)
			continue;
		adapter_name[_countof(adapter_name) - 1] = L'0';
		if (!adapter_name[0])
			continue;
		buf_len = WideCharToMultiByte(CP_UTF8, 0, adapter_name, -1, NULL, 0, NULL, NULL);
		if (!buf_len)
			continue;
		interface_name = malloc(buf_len);
		if (!interface_name)
			continue;
		buf_len = WideCharToMultiByte(CP_UTF8, 0, adapter_name, -1, interface_name, buf_len, NULL, NULL);
		if (!buf_len) {
			free(interface_name);
			continue;
		}
		found = !strcmp(interface_name, iface);
		free(interface_name);
		if (!found)
			continue;

		if (SetupDiGetDeviceInstanceIdW(dev_info, &dev_info_data, NULL, 0, &buf_len) || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			continue;
		buf = calloc(sizeof(*buf), buf_len);
		if (!buf)
			continue;
		if (!SetupDiGetDeviceInstanceIdW(dev_info, &dev_info_data, buf, buf_len, &buf_len))
			goto cleanup_instance_id;
		if (CM_Get_Device_Interface_List_SizeW(
			&buf_len, (GUID *)&GUID_DEVINTERFACE_NET, (DEVINSTID_W)buf,
			CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS)
			goto cleanup_instance_id;
		interfaces = calloc(buf_len, sizeof(*interfaces));
		if (!interfaces)
			goto cleanup_instance_id;
		if (CM_Get_Device_Interface_ListW(
			(GUID *)&GUID_DEVINTERFACE_NET, (DEVINSTID_W)buf, interfaces, buf_len,
			CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS || !interfaces[0]) {
			free(interfaces);
			interfaces = NULL;
			goto cleanup_instance_id;
		}
cleanup_instance_id:
		free(buf);
	}
	SetupDiDestroyDeviceInfoList(dev_info);
	if (!interfaces) {
		errno = ENOENT;
		return NULL;
	}
	handle = CreateFileW(interfaces, GENERIC_READ | GENERIC_WRITE,
			     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
			     OPEN_EXISTING, 0, NULL);
	free(interfaces);
	if (handle == INVALID_HANDLE_VALUE) {
		errno = EACCES;
		return NULL;
	}
	return handle;
}

static int kernel_get_device(struct wgdevice **device, const char *iface)
{
	WG_IOCTL_INTERFACE *wg_iface;
	WG_IOCTL_PEER *wg_peer;
	WG_IOCTL_ALLOWED_IP *wg_aip;
	void *buf = NULL;
	DWORD buf_len = 0;
	HANDLE handle = kernel_interface_handle(iface);
	struct wgdevice *dev;
	struct wgpeer *peer;
	struct wgallowedip *aip;
	int ret;

	*device = NULL;

	if (!handle)
		return -errno;

	while (!DeviceIoControl(handle, WG_IOCTL_GET, NULL, 0, buf, buf_len, &buf_len, NULL)) {
		free(buf);
		if (GetLastError() != ERROR_MORE_DATA) {
			errno = EACCES;
			return -errno;
		}
		buf = malloc(buf_len);
		if (!buf)
			return -errno;
	}

	wg_iface = (WG_IOCTL_INTERFACE *)buf;
	dev = calloc(1, sizeof(*dev));
	if (!dev)
		goto out;
	strncpy(dev->name, iface, sizeof(dev->name));
	dev->name[sizeof(dev->name) - 1] = '\0';

	if (wg_iface->Flags & WG_IOCTL_INTERFACE_HAS_LISTEN_PORT) {
		dev->listen_port = wg_iface->ListenPort;
		dev->flags |= WGDEVICE_HAS_LISTEN_PORT;
	}

	if (wg_iface->Flags & WG_IOCTL_INTERFACE_HAS_PUBLIC_KEY) {
		memcpy(dev->public_key, wg_iface->PublicKey, sizeof(dev->public_key));
		dev->flags |= WGDEVICE_HAS_PUBLIC_KEY;
	}

	if (wg_iface->Flags & WG_IOCTL_INTERFACE_HAS_PRIVATE_KEY) {
		memcpy(dev->private_key, wg_iface->PrivateKey, sizeof(dev->private_key));
		dev->flags |= WGDEVICE_HAS_PRIVATE_KEY;
	}

	wg_peer = buf + sizeof(WG_IOCTL_INTERFACE);
	for (ULONG i = 0; i < wg_iface->PeersCount; ++i) {
		peer = calloc(1, sizeof(*peer));
		if (!peer)
			goto out;

		if (dev->first_peer == NULL)
			dev->first_peer = peer;
		else
			dev->last_peer->next_peer = peer;
		dev->last_peer = peer;

		if (wg_peer->Flags & WG_IOCTL_PEER_HAS_PUBLIC_KEY) {
			memcpy(peer->public_key, wg_peer->PublicKey, sizeof(peer->public_key));
			peer->flags |= WGPEER_HAS_PUBLIC_KEY;
		}

		if (wg_peer->Flags & WG_IOCTL_PEER_HAS_PRESHARED_KEY) {
			memcpy(peer->preshared_key, wg_peer->PresharedKey, sizeof(peer->preshared_key));
			if (!key_is_zero(peer->preshared_key))
				peer->flags |= WGPEER_HAS_PRESHARED_KEY;
		}

		if (wg_peer->Flags & WG_IOCTL_PEER_HAS_PERSISTENT_KEEPALIVE) {
			peer->persistent_keepalive_interval = wg_peer->PersistentKeepalive;
			peer->flags |= WGPEER_HAS_PERSISTENT_KEEPALIVE_INTERVAL;
		}

		if (wg_peer->Flags & WG_IOCTL_PEER_HAS_ENDPOINT) {
			if (wg_peer->Endpoint.si_family == AF_INET)
				peer->endpoint.addr4 = wg_peer->Endpoint.Ipv4;
			else if (wg_peer->Endpoint.si_family == AF_INET6)
				peer->endpoint.addr6 = wg_peer->Endpoint.Ipv6;
		}

		peer->rx_bytes = wg_peer->RxBytes;
		peer->tx_bytes = wg_peer->TxBytes;

		if (wg_peer->LastHandshake) {
			peer->last_handshake_time.tv_sec = wg_peer->LastHandshake / 10000000 - 11644473600LL;
			peer->last_handshake_time.tv_nsec = wg_peer->LastHandshake % 10000000 * 100;
		}

		wg_aip = (void *)wg_peer + sizeof(WG_IOCTL_PEER);
		for (ULONG j = 0; j < wg_peer->AllowedIPsCount; ++j) {
			aip = calloc(1, sizeof(*aip));
			if (!aip)
				goto out;

			if (peer->first_allowedip == NULL)
				peer->first_allowedip = aip;
			else
				peer->last_allowedip->next_allowedip = aip;
			peer->last_allowedip = aip;

			aip->family = wg_aip->AddressFamily;
			if (wg_aip->AddressFamily == AF_INET) {
				memcpy(&aip->ip4, &wg_aip->Address.V4, sizeof(aip->ip4));
				aip->cidr = wg_aip->Cidr;
			} else if (wg_aip->AddressFamily == AF_INET6) {
				memcpy(&aip->ip6, &wg_aip->Address.V6, sizeof(aip->ip6));
				aip->cidr = wg_aip->Cidr;
			}
			++wg_aip;
		}
		wg_peer = (WG_IOCTL_PEER *)wg_aip;
	}
	*device = dev;
	errno = 0;
out:
	ret = -errno;
	free(buf);
	CloseHandle(handle);
	return ret;
}

static int kernel_set_device(struct wgdevice *dev)
{
	WG_IOCTL_INTERFACE *wg_iface = NULL;
	WG_IOCTL_PEER *wg_peer;
	WG_IOCTL_ALLOWED_IP *wg_aip;
	DWORD buf_len = sizeof(WG_IOCTL_INTERFACE);
	HANDLE handle = kernel_interface_handle(dev->name);
	struct wgpeer *peer;
	struct wgallowedip *aip;
	size_t peer_count, aip_count;
	int ret = 0;

	if (!handle)
		return -errno;

	for_each_wgpeer(dev, peer) {
		if (DWORD_MAX - buf_len < sizeof(WG_IOCTL_PEER)) {
			errno = EOVERFLOW;
			goto out;
		}
		buf_len += sizeof(WG_IOCTL_PEER);
		for_each_wgallowedip(peer, aip) {
			if (DWORD_MAX - buf_len < sizeof(WG_IOCTL_ALLOWED_IP)) {
				errno = EOVERFLOW;
				goto out;
			}
			buf_len += sizeof(WG_IOCTL_ALLOWED_IP);
		}
	}
	wg_iface = calloc(1, buf_len);
	if (!wg_iface)
		goto out;

	if (dev->flags & WGDEVICE_HAS_PRIVATE_KEY) {
		memcpy(wg_iface->PrivateKey, dev->private_key, sizeof(wg_iface->PrivateKey));
		wg_iface->Flags |= WG_IOCTL_INTERFACE_HAS_PRIVATE_KEY;
	}

	if (dev->flags & WGDEVICE_HAS_LISTEN_PORT) {
		wg_iface->ListenPort = dev->listen_port;
		wg_iface->Flags |= WG_IOCTL_INTERFACE_HAS_LISTEN_PORT;
	}

	if (dev->flags & WGDEVICE_REPLACE_PEERS)
		wg_iface->Flags |= WG_IOCTL_INTERFACE_REPLACE_PEERS;

	peer_count = 0;
	wg_peer = (void *)wg_iface + sizeof(WG_IOCTL_INTERFACE);
	for_each_wgpeer(dev, peer) {
		wg_peer->Flags = WG_IOCTL_PEER_HAS_PUBLIC_KEY;
		memcpy(wg_peer->PublicKey, peer->public_key, sizeof(wg_peer->PublicKey));

		if (peer->flags & WGPEER_HAS_PRESHARED_KEY) {
			memcpy(wg_peer->PresharedKey, peer->preshared_key, sizeof(wg_peer->PresharedKey));
			wg_peer->Flags |= WG_IOCTL_PEER_HAS_PRESHARED_KEY;
		}

		if (peer->flags & WGPEER_HAS_PERSISTENT_KEEPALIVE_INTERVAL) {
			wg_peer->PersistentKeepalive = peer->persistent_keepalive_interval;
			wg_peer->Flags |= WG_IOCTL_PEER_HAS_PERSISTENT_KEEPALIVE;
		}

		if (peer->endpoint.addr.sa_family == AF_INET) {
			wg_peer->Endpoint.Ipv4 = peer->endpoint.addr4;
			wg_peer->Flags |= WG_IOCTL_PEER_HAS_ENDPOINT;
		} else if (peer->endpoint.addr.sa_family == AF_INET6) {
			wg_peer->Endpoint.Ipv6 = peer->endpoint.addr6;
			wg_peer->Flags |= WG_IOCTL_PEER_HAS_ENDPOINT;
		}

		if (peer->flags & WGPEER_REPLACE_ALLOWEDIPS)
			wg_peer->Flags |= WG_IOCTL_PEER_REPLACE_ALLOWED_IPS;

		if (peer->flags & WGPEER_REMOVE_ME)
			wg_peer->Flags |= WG_IOCTL_PEER_REMOVE;

		aip_count = 0;
		wg_aip = (void *)wg_peer + sizeof(WG_IOCTL_PEER);
		for_each_wgallowedip(peer, aip) {
			wg_aip->AddressFamily = aip->family;
			wg_aip->Cidr = aip->cidr;

			if (aip->family == AF_INET)
				wg_aip->Address.V4 = aip->ip4;
			else if (aip->family == AF_INET6)
				wg_aip->Address.V6 = aip->ip6;
			else
				continue;
			++aip_count;
			++wg_aip;
		}
		wg_peer->AllowedIPsCount = aip_count;
		++peer_count;
		wg_peer = (WG_IOCTL_PEER *)wg_aip;
	}
	wg_iface->PeersCount = peer_count;

	if (!DeviceIoControl(handle, WG_IOCTL_SET, NULL, 0, wg_iface, buf_len, &buf_len, NULL)) {
		errno = EACCES;
		goto out;
	}
	errno = 0;

out:
	ret = -errno;
	free(wg_iface);
	CloseHandle(handle);
	return ret;
}
