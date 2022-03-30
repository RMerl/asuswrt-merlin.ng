// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI Unicode collation protocol
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <charset.h>
#include <cp1250.h>
#include <cp437.h>
#include <efi_loader.h>

/* Characters that may not be used in FAT 8.3 file names */
static const char illegal[] = "+,<=>:;\"/\\|?*[]\x7f";

/*
 * EDK2 assumes codepage 1250 when creating FAT 8.3 file names.
 * Linux defaults to codepage 437 for FAT 8.3 file names.
 */
#if CONFIG_FAT_DEFAULT_CODEPAGE == 1250
/* Unicode code points for code page 1250 characters 0x80 - 0xff */
static const u16 codepage[] = CP1250;
#else
/* Unicode code points for code page 437 characters 0x80 - 0xff */
static const u16 codepage[] = CP437;
#endif

/* GUID of the EFI_UNICODE_COLLATION_PROTOCOL2 */
const efi_guid_t efi_guid_unicode_collation_protocol2 =
	EFI_UNICODE_COLLATION_PROTOCOL2_GUID;

/**
 * efi_stri_coll() - compare utf-16 strings case-insenitively
 *
 * @this:	unicode collation protocol instance
 * @s1:		first string
 * @s2:		second string
 *
 * This function implements the StriColl() service of the
 * EFI_UNICODE_COLLATION_PROTOCOL.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * TODO:
 * The implementation does not follow the Unicode collation algorithm.
 * For ASCII characters it results in the same sort order as EDK2.
 * We could use table UNICODE_CAPITALIZATION_TABLE for better results.
 *
 * Return:	0: s1 == s2, > 0: s1 > s2, < 0: s1 < s2
 */
static efi_intn_t EFIAPI efi_stri_coll(
		struct efi_unicode_collation_protocol *this, u16 *s1, u16 *s2)
{
	s32 c1, c2;
	efi_intn_t ret = 0;

	EFI_ENTRY("%p, %ls, %ls", this, s1, s2);
	for (; *s1 | *s2; ++s1, ++s2) {
		c1 = utf_to_upper(*s1);
		c2 = utf_to_upper(*s2);
		if (c1 < c2) {
			ret = -1;
			goto out;
		} else if (c1 > c2) {
			ret = 1;
			goto out;
		}
	}
out:
	EFI_EXIT(EFI_SUCCESS);
	return ret;
}

/**
 * next_lower() - get next codepoint converted to lower case
 *
 * @string:	pointer to u16 string, on return advanced by one codepoint
 * Return:	first codepoint of string converted to lower case
 */
static s32 next_lower(const u16 **string)
{
	return utf_to_lower(utf16_get(string));
}

/**
 * metai_match() - compare utf-16 string with a pattern string case-insenitively
 *
 * @string:	string to compare
 * @pattern:	pattern string
 *
 * The pattern string may use these:
 *	- * matches >= 0 characters
 *	- ? matches 1 character
 *	- [<char1><char2>...<charN>] match any character in the set
 *	- [<char1>-<char2>] matches any character in the range
 *
 * This function is called my efi_metai_match().
 *
 * For '*' pattern searches this function calls itself recursively.
 * Performance-wise this is suboptimal, especially for multiple '*' wildcards.
 * But it results in simple code.
 *
 * Return:	true if the string is matched.
 */
static bool metai_match(const u16 *string, const u16 *pattern)
{
	s32 first, s, p;

	for (; *string && *pattern;) {
		const u16 *string_old = string;

		s = next_lower(&string);
		p = next_lower(&pattern);

		switch (p) {
		case '*':
			/* Match 0 or more characters */
			for (;; s = next_lower(&string)) {
				if (metai_match(string_old, pattern))
					return true;
				if (!s)
					return false;
				string_old = string;
			}
		case '?':
			/* Match any one character */
			break;
		case '[':
			/* Match any character in the set */
			p = next_lower(&pattern);
			first = p;
			if (first == ']')
				/* Empty set */
				return false;
			p = next_lower(&pattern);
			if (p == '-') {
				/* Range */
				p = next_lower(&pattern);
				if (s < first || s > p)
					return false;
				p = next_lower(&pattern);
				if (p != ']')
					return false;
			} else {
				/* Set */
				bool hit = false;

				if (s == first)
					hit = true;
				for (; p && p != ']';
				     p = next_lower(&pattern)) {
					if (p == s)
						hit = true;
				}
				if (!hit || p != ']')
					return false;
			}
			break;
		default:
			/* Match one character */
			if (p != s)
				return false;
		}
	}
	if (!*pattern && !*string)
		return true;
	return false;
}

/**
 * efi_metai_match() - compare utf-16 string with a pattern string
 *		       case-insenitively
 *
 * @this:	unicode collation protocol instance
 * @s:		string to compare
 * @p:		pattern string
 *
 * The pattern string may use these:
 *	- * matches >= 0 characters
 *	- ? matches 1 character
 *	- [<char1><char2>...<charN>] match any character in the set
 *	- [<char1>-<char2>] matches any character in the range
 *
 * This function implements the MetaMatch() service of the
 * EFI_UNICODE_COLLATION_PROTOCOL.
 *
 * Return:	true if the string is matched.
 */
static bool EFIAPI efi_metai_match(struct efi_unicode_collation_protocol *this,
				   const u16 *string, const u16 *pattern)
{
	bool ret;

	EFI_ENTRY("%p, %ls, %ls", this, string, pattern);
	ret =  metai_match(string, pattern);
	EFI_EXIT(EFI_SUCCESS);
	return ret;
}

/**
 * efi_str_lwr() - convert to lower case
 *
 * @this:	unicode collation protocol instance
 * @string:	string to convert
 * @p:		pattern string
 *
 * The conversion is done in place. As long as upper and lower letters use the
 * same number of words this does not pose a problem.
 *
 * This function implements the StrLwr() service of the
 * EFI_UNICODE_COLLATION_PROTOCOL.
 */
static void EFIAPI efi_str_lwr(struct efi_unicode_collation_protocol *this,
			       u16 *string)
{
	EFI_ENTRY("%p, %ls", this, string);
	for (; *string; ++string)
		*string = utf_to_lower(*string);
	EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_str_upr() - convert to upper case
 *
 * @this:	unicode collation protocol instance
 * @string:	string to convert
 * @p:		pattern string
 *
 * The conversion is done in place. As long as upper and lower letters use the
 * same number of words this does not pose a problem.
 *
 * This function implements the StrUpr() service of the
 * EFI_UNICODE_COLLATION_PROTOCOL.
 */
static void EFIAPI efi_str_upr(struct efi_unicode_collation_protocol *this,
			       u16 *string)
{
	EFI_ENTRY("%p, %ls", this, string);
	for (; *string; ++string)
		*string = utf_to_upper(*string);
	EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_fat_to_str() - convert an 8.3 file name from an OEM codepage to Unicode
 *
 * @this:	unicode collation protocol instance
 * @fat_size:	size of the string to convert
 * @fat:	string to convert
 * @string:	converted string
 *
 * This function implements the FatToStr() service of the
 * EFI_UNICODE_COLLATION_PROTOCOL.
 */
static void EFIAPI efi_fat_to_str(struct efi_unicode_collation_protocol *this,
				  efi_uintn_t fat_size, char *fat, u16 *string)
{
	efi_uintn_t i;
	u16 c;

	EFI_ENTRY("%p, %zu, %s, %p", this, fat_size, fat, string);
	for (i = 0; i < fat_size; ++i) {
		c = (unsigned char)fat[i];
		if (c > 0x80)
			c = codepage[i - 0x80];
		string[i] = c;
		if (!c)
			break;
	}
	string[i] = 0;
	EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_fat_to_str() - convert a utf-16 string to legal characters for a FAT
 *                    file name in an OEM code page
 *
 * @this:	unicode collation protocol instance
 * @string:	Unicode string to convert
 * @fat_size:	size of the target buffer
 * @fat:	converted string
 *
 * This function implements the StrToFat() service of the
 * EFI_UNICODE_COLLATION_PROTOCOL.
 *
 * Return:	true if an illegal character was substituted by '_'.
 */
static bool EFIAPI efi_str_to_fat(struct efi_unicode_collation_protocol *this,
				  const u16 *string, efi_uintn_t fat_size,
				  char *fat)
{
	efi_uintn_t i;
	s32 c;
	bool ret = false;

	EFI_ENTRY("%p, %ls, %zu, %p", this, string, fat_size, fat);
	for (i = 0; i < fat_size;) {
		c = utf16_get(&string);
		switch (c) {
		/* Ignore period and space */
		case '.':
		case ' ':
			continue;
		case 0:
			break;
		}
		c = utf_to_upper(c);
		if (c >= 0x80) {
			int j;

			/* Look for codepage translation */
			for (j = 0; j < 0x80; ++j) {
				if (c == codepage[j]) {
					c = j + 0x80;
					break;
				}
			}
			if (j >= 0x80) {
				c = '_';
				ret = true;
			}
		} else if (c && (c < 0x20 || strchr(illegal, c))) {
			c = '_';
			ret = true;
		}

		fat[i] = c;
		if (!c)
			break;
		++i;
	}
	EFI_EXIT(EFI_SUCCESS);
	return ret;
}

const struct efi_unicode_collation_protocol efi_unicode_collation_protocol2 = {
	.stri_coll = efi_stri_coll,
	.metai_match = efi_metai_match,
	.str_lwr = efi_str_lwr,
	.str_upr = efi_str_upr,
	.fat_to_str = efi_fat_to_str,
	.str_to_fat = efi_str_to_fat,
	.supported_languages = "en",
};

/*
 * In EFI 1.10 a version of the Unicode collation protocol using ISO 639-2
 * language codes existed. This protocol is not part of the UEFI specification
 * any longer. Unfortunately it is required to run the UEFI Self Certification
 * Test (SCT) II, version 2.6, 2017. So we implement it here for the sole
 * purpose of running the SCT. It can be removed when a compliant SCT is
 * available.
 */
#if CONFIG_IS_ENABLED(EFI_UNICODE_COLLATION_PROTOCOL)

/* GUID of the EFI_UNICODE_COLLATION_PROTOCOL */
const efi_guid_t efi_guid_unicode_collation_protocol =
	EFI_UNICODE_COLLATION_PROTOCOL_GUID;

const struct efi_unicode_collation_protocol efi_unicode_collation_protocol = {
	.stri_coll = efi_stri_coll,
	.metai_match = efi_metai_match,
	.str_lwr = efi_str_lwr,
	.str_upr = efi_str_upr,
	.fat_to_str = efi_fat_to_str,
	.str_to_fat = efi_str_to_fat,
	/* ISO 639-2 language code */
	.supported_languages = "eng",
};

#endif
