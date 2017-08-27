/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <winsock2.h>
#include <windows.h>
#include <winhttp.h>

#include "winhttp_fetcher.h"

#include <library.h>

/**
 * Timeout for DNS resolution, in ms
 */
#define RESOLVE_TIMEOUT 5000

/**
 * Timeout for TCP connect, in ms
 */
#define CONNECT_TIMEOUT 10000

typedef struct private_winhttp_fetcher_t private_winhttp_fetcher_t;

/**
 * Private data of a winhttp_fetcher_t.
 */
struct private_winhttp_fetcher_t {

	/**
	 * Public interface
	 */
	winhttp_fetcher_t public;

	/**
	 * WinHTTP session handle
	 */
	HINTERNET session;

	/**
	 * POST request data
	 */
	chunk_t request;

	/**
	 * HTTP version string to use
	 */
	LPWSTR version;

	/**
	 * Optional HTTP headers, as allocated LPWSTR
	 */
	linked_list_t *headers;

	/**
	 * Callback function
	 */
	fetcher_callback_t cb;

	/**
	 * Timeout for operations, in ms
	 */
	u_long timeout;

	/**
	 * User pointer to store HTTP status code to
	 */
	u_int *result;
};

/**
 * Configure and send the HTTP request
 */
static bool send_request(private_winhttp_fetcher_t *this, HINTERNET request)
{
	WCHAR headers[512] = L"";
	LPWSTR hdr;

	/* Set timeout. By default, send/receive does not time out */
	if (!WinHttpSetTimeouts(request, RESOLVE_TIMEOUT, CONNECT_TIMEOUT,
							this->timeout, this->timeout))
	{
		DBG1(DBG_LIB, "opening HTTP request failed: %u", GetLastError());
		return FALSE;
	}
	while (this->headers->remove_first(this->headers, (void**)&hdr) == SUCCESS)
	{
		wcsncat(headers, hdr, countof(headers) - wcslen(headers) - 1);
		if (this->headers->get_count(this->headers))
		{
			wcsncat(headers, L"\r\n", countof(headers) - wcslen(headers) - 1);
		}
		free(hdr);
	}
	if (!WinHttpSendRequest(request, headers, wcslen(headers),
				this->request.ptr, this->request.len, this->request.len, 0))
	{
		DBG1(DBG_LIB, "sending HTTP request failed: %u", GetLastError());
		return FALSE;
	}
	return TRUE;
}

/**
 * Read back result and invoke receive callback
 */
static bool read_result(private_winhttp_fetcher_t *this, HINTERNET request,
						void *user)
{
	DWORD received;
	char buf[1024];
	u_int32_t code;
	DWORD codelen = sizeof(code);

	if (!WinHttpReceiveResponse(request, NULL))
	{
		DBG1(DBG_LIB, "reading HTTP response header failed: %u", GetLastError());
		return FALSE;
	}
	if (!WinHttpQueryHeaders(request,
				WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
				NULL, &code, &codelen, NULL))
	{
		DBG1(DBG_LIB, "reading HTTP status code failed: %u", GetLastError());
		return FALSE;
	}
	if (this->result)
	{
		*this->result = code;
	}
	if (code < 200 || code >= 300)
	{	/* non-successful HTTP status code */
		if (!this->result)
		{
			DBG1(DBG_LIB, "HTTP request failed with status %u", code);
		}
		return FALSE;
	}
	if (this->cb == fetcher_default_callback)
	{
		*(chunk_t*)user = chunk_empty;
	}
	while (TRUE)
	{
		if (!WinHttpReadData(request, buf, sizeof(buf), &received))
		{
			DBG1(DBG_LIB, "reading HTTP response failed: %u", GetLastError());
			return FALSE;
		}
		if (received == 0)
		{
			/* end of response */
			break;
		}
		if (!this->cb(user, chunk_create(buf, received)))
		{
			DBG1(DBG_LIB, "processing response failed or cancelled");
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Parse an uri to wide string host and path, optionally set flags and port
 */
static bool parse_uri(private_winhttp_fetcher_t *this, char *uri,
					  LPWSTR host, int hostlen, LPWSTR path, int pathlen,
					  LPWSTR user, int userlen, LPWSTR pass, int passlen,
					  DWORD *flags, INTERNET_PORT *port)
{
	WCHAR wuri[512], extra[256];
	URL_COMPONENTS comps = {
		.dwStructSize = sizeof(URL_COMPONENTS),
		.lpszHostName = host,
		.dwHostNameLength = hostlen,
		.lpszUrlPath = path,
		.dwUrlPathLength = pathlen,
		.lpszUserName = user,
		.dwUserNameLength = userlen,
		.lpszPassword = pass,
		.dwPasswordLength = passlen,
		.lpszExtraInfo = extra,
		.dwExtraInfoLength = countof(extra),
	};

	if (!MultiByteToWideChar(CP_THREAD_ACP, 0, uri, -1, wuri, countof(wuri)))
	{
		DBG1(DBG_LIB, "converting URI failed: %u", GetLastError());
		return FALSE;
	}
	if (!WinHttpCrackUrl(wuri, 0, ICU_ESCAPE, &comps))
	{
		DBG1(DBG_LIB, "cracking URI failed: %u", GetLastError());
		return FALSE;
	}
	if (comps.nScheme == INTERNET_SCHEME_HTTPS)
	{
		*flags |= WINHTTP_FLAG_SECURE;
	}
	if (comps.dwExtraInfoLength)
	{
		wcsncat(path, extra, pathlen - comps.dwUrlPathLength - 1);
	}
	if (comps.nPort)
	{
		*port = comps.nPort;
	}
	return TRUE;
}

/**
 * Set credentials for basic authentication, if given
 */
static bool set_credentials(private_winhttp_fetcher_t *this,
							HINTERNET *request, LPWSTR user, LPWSTR pass)
{
	if (!wcslen(user) && !wcslen(pass))
	{	/* skip */
		return TRUE;
	}
	return WinHttpSetCredentials(request, WINHTTP_AUTH_TARGET_SERVER,
								 WINHTTP_AUTH_SCHEME_BASIC, user, pass, NULL);
}

METHOD(fetcher_t, fetch, status_t,
	private_winhttp_fetcher_t *this, char *uri, void *userdata)
{
	INTERNET_PORT port = INTERNET_DEFAULT_PORT;
	status_t status = FAILED;
	DWORD flags = 0;
	HINTERNET connection, request;
	WCHAR host[256], path[512], user[256], pass[256], *method;

	if (this->request.len)
	{
		method = L"POST";
	}
	else
	{
		method = L"GET";
	}

	if (this->result)
	{	/* zero-initialize for early failures */
		*this->result = 0;
	}

	if (parse_uri(this, uri, host, countof(host), path, countof(path),
				  user, countof(user), pass, countof(pass), &flags, &port))
	{
		connection = WinHttpConnect(this->session, host, port, 0);
		if (connection)
		{
			request = WinHttpOpenRequest(connection, method, path, this->version,
										 WINHTTP_NO_REFERER,
										 WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
			if (request)
			{
				if (set_credentials(this, request, user, pass) &&
					send_request(this, request) &&
					read_result(this, request, userdata))
				{
					status = SUCCESS;
				}
				WinHttpCloseHandle(request);
			}
			else
			{
				DBG1(DBG_LIB, "opening request failed: %u", GetLastError());
			}
			WinHttpCloseHandle(connection);
		}
		else
		{
			DBG1(DBG_LIB, "connection failed: %u", GetLastError());
		}
	}
	return status;
}

/**
 * Append an header as wide string
 */
static bool append_header(private_winhttp_fetcher_t *this, char *name)
{
	int len;
	LPWSTR buf;

	len = MultiByteToWideChar(CP_THREAD_ACP, 0, name, -1, NULL, 0);
	if (!len)
	{
		return FALSE;
	}
	buf = calloc(len, sizeof(WCHAR));
	if (!MultiByteToWideChar(CP_THREAD_ACP, 0, name, -1, buf, len))
	{
		free(buf);
		return FALSE;
	}
	this->headers->insert_last(this->headers, buf);
	return TRUE;
}

METHOD(fetcher_t, set_option, bool,
	private_winhttp_fetcher_t *this, fetcher_option_t option, ...)
{
	bool supported = TRUE;
	char buf[128];
	va_list args;

	va_start(args, option);
	switch (option)
	{
		case FETCH_REQUEST_DATA:
			this->request = va_arg(args, chunk_t);
			break;
		case FETCH_REQUEST_TYPE:
			snprintf(buf, sizeof(buf), "Content-Type: %s", va_arg(args, char*));
			supported = append_header(this, buf);
			break;
		case FETCH_REQUEST_HEADER:
			supported = append_header(this, va_arg(args, char*));
			break;
		case FETCH_HTTP_VERSION_1_0:
			this->version = L"HTTP/1.0";
			break;
		case FETCH_TIMEOUT:
			this->timeout = va_arg(args, u_int) * 1000;
			break;
		case FETCH_CALLBACK:
			this->cb = va_arg(args, fetcher_callback_t);
			break;
		case FETCH_RESPONSE_CODE:
			this->result = va_arg(args, u_int*);
			break;
		case FETCH_SOURCEIP:
			/* not supported, FALL */
		default:
			supported = FALSE;
			break;
	}
	va_end(args);
	return supported;
}

METHOD(fetcher_t, destroy, void,
	private_winhttp_fetcher_t *this)
{
	WinHttpCloseHandle(this->session);
	this->headers->destroy_function(this->headers, free);
	free(this);
}
/*
 * Described in header.
 */
winhttp_fetcher_t *winhttp_fetcher_create()
{
	private_winhttp_fetcher_t *this;

	INIT(this,
		.public = {
			.interface = {
				.fetch = _fetch,
				.set_option = _set_option,
				.destroy = _destroy,
			},
		},
		.version = L"HTTP/1.1",
		.cb = fetcher_default_callback,
		.headers = linked_list_create(),
		.session = WinHttpOpen(L"strongSwan WinHTTP fetcher",
							WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
							WINHTTP_NO_PROXY_NAME,
							WINHTTP_NO_PROXY_BYPASS, 0),
	);

	if (!this->session)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}
