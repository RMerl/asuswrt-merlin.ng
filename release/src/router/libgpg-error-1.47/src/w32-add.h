## w32-add.h - Snippet to be be included into gpg-error.h.
## Comments are indicated by a double hash mark.  Due to a
## peculiarity of the script the first used line must not
## start with a hash mark.

/* Decide whether to use the format_arg attribute.  */
#if _GPG_ERR_GCC_VERSION > 20800
# define _GPG_ERR_ATTR_FORMAT_ARG(a)  __attribute__ ((__format_arg__ (a)))
#else
# define _GPG_ERR_ATTR_FORMAT_ARG(a)
#endif

/* A lean gettext implementation based on GNU style mo files which are
   required to be encoded in UTF-8.  There is a limit on 65534 entries
   to save some RAM.  Only Germanic plural rules are supported.  */
const char *_gpg_w32_bindtextdomain (const char *domainname,
                                     const char *dirname);
const char *_gpg_w32_textdomain (const char *domainname);
const char *_gpg_w32_gettext (const char *msgid)
            _GPG_ERR_ATTR_FORMAT_ARG (1);
const char *_gpg_w32_dgettext (const char *domainname, const char *msgid)
            _GPG_ERR_ATTR_FORMAT_ARG (2);
const char *_gpg_w32_dngettext (const char *domainname, const char *msgid1,
                                const char *msgid2, unsigned long int n)
            _GPG_ERR_ATTR_FORMAT_ARG (2) _GPG_ERR_ATTR_FORMAT_ARG (3);
const char *_gpg_w32_gettext_localename (void);
int _gpg_w32_gettext_use_utf8 (int value);

#ifdef GPG_ERR_ENABLE_GETTEXT_MACROS
# define bindtextdomain(a,b) _gpg_w32_bindtextdomain ((a), (b))
# define textdomain(a)       _gpg_w32_textdomain ((a))
# define gettext(a)          _gpg_w32_gettext ((a))
# define dgettext(a,b)       _gpg_w32_dgettext ((a), (b))
# define ngettext(a,b,c)     _gpg_w32_dngettext (NULL, (a), (b), (c))
# define dngettext(a,b,c,d)  _gpg_w32_dngettext ((a), (b), (c), (d))
# define gettext_localename() _gpg_w32_gettext_localename ()
# define gettext_use_utf8(a) _gpg_w32_gettext_use_utf8 (a)
#endif /*GPG_ERR_ENABLE_GETTEXT_MACROS*/

/* Force the use of the locale NAME or if NAME is NULL the one derived
 * from LANGID.  This function must be used early and is not thread-safe. */
void gpgrt_w32_override_locale (const char *name, unsigned short langid);


/* A simple iconv implementation w/o the need for an extra DLL.  */
struct _gpgrt_w32_iconv_s;
typedef struct _gpgrt_w32_iconv_s *gpgrt_w32_iconv_t;

gpgrt_w32_iconv_t gpgrt_w32_iconv_open (const char *tocode,
                                        const char *fromcode);
int     gpgrt_w32_iconv_close (gpgrt_w32_iconv_t cd);
size_t  gpgrt_w32_iconv (gpgrt_w32_iconv_t cd,
                         const char **inbuf, size_t *inbytesleft,
                         char **outbuf, size_t *outbytesleft);

#ifdef GPGRT_ENABLE_W32_ICONV_MACROS
# define ICONV_CONST const
# define iconv_t gpgrt_w32_iconv_t
# define iconv_open(a,b)  gpgrt_w32_iconv_open ((a), (b))
# define iconv_close(a)   gpgrt_w32_iconv_close ((a))
# define iconv(a,b,c,d,e) gpgrt_w32_iconv ((a),(b),(c),(d),(e))
#endif /*GPGRT_ENABLE_W32_ICONV_MACROS*/

/* Release a wchar_t * buffer.  */
void gpgrt_free_wchar (wchar_t *wstring);

/* Convert an UTF-8 encoded file name to wchar.
 * Prepend a '\\?\' prefix if needed.  */
wchar_t *gpgrt_fname_to_wchar (const char *fname);

/* Convert an UTF8 string to a WCHAR string.  Caller should use
 * gpgrt_free_wchar to release the result.
 * Returns NULL on error and sets ERRNO. */
wchar_t *gpgrt_utf8_to_wchar (const char *string);

/* Convert a WCHAR string to UTF-8.  Caller should use gpgrt_free to
 * release the result.   Returns NULL on error and sets ERRNO.  */
char *gpgrt_wchar_to_utf8 (const wchar_t *wstring);

/* Query a string in the registry.  */
char *gpgrt_w32_reg_query_string (const char *root,
                                  const char *dir,
                                  const char *name);
