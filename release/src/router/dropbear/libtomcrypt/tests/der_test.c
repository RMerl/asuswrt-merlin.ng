/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <tomcrypt_test.h>
#if defined(GMP_LTC_DESC) || defined(USE_GMP)
#include <gmp.h>
#endif

#if !defined(LTC_DER) || !defined(LTC_TEST_MPI)

int der_test(void)
{
   return CRYPT_NOP;
}

#else

static const unsigned char _der_tests_stinky_root_cert[] =
   "MIIFETCCA/mgAwIBAgIQbv53JNmv518t5lkCHE272jANBgkqhkiG9w0BAQUFADCB"
   "lTELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAlVUMRcwFQYDVQQHEw5TYWx0IExha2Ug"
   "Q2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMSEwHwYDVQQLExho"
   "dHRwOi8vd3d3LnVzZXJ0cnVzdC5jb20xHTAbBgNVBAMTFFVUTi1VU0VSRmlyc3Qt"
   "T2JqZWN0MB4XDTA4MDQyOTAwMDAwMFoXDTEwMDQyOTIzNTk1OVowgbUxCzAJBgNV"
   "BAYTAlVTMQ4wDAYDVQQRDAU0NDE0MzELMAkGA1UECAwCT0gxGTAXBgNVBAcMEE1h"
   "eWZpZWxkIFZpbGxhZ2UxEDAOBgNVBAkMB1N1aXRlIEExFDASBgNVBAkMCzc2NyBC"
   "ZXRhIERyMSIwIAYDVQQKDBlQcmVlbXB0aXZlIFNvbHV0aW9ucywgTExDMSIwIAYD"
   "VQQDDBlQcmVlbXB0aXZlIFNvbHV0aW9ucywgTExDMIIBIjANBgkqhkiG9w0BAQEF"
   "AAOCAQ8AMIIBCgKCAQEAzH7ZBkMcBuHx8d2f10RGTHAf7gzzVteGbOihJGH2BwlS"
   "ZvNp6WEE4DfL+s1vp0wzk1XeLN5tRjg2qum9YqyCk7okh7pXGy46f5mWbLQiefGA"
   "j5UXRcr6WJ3xeACdbXxKrYMV0REia+4Jb2UbFA8S81PjhRon6vcRz76ziUWwt8NC"
   "igX+4ZC0skhhKzKszel6KGL7bJCtLG7ukw9DZCrvPCRcKFeM/GwQ6ACMgP88CSCL"
   "t1fbIXDH1vd/x2XM3QlaSDN6hYDbef8m1T+9TCkXVKeqG1GYjSUrHzYnCZUmTRrR"
   "38jgC3qXxiIpDKW105uM0nlXe2XF9c+ot2MdWvV4TwIDAQABo4IBOTCCATUwHwYD"
   "VR0jBBgwFoAU2u1kdBScFDyr3ZmpvVsoTYs8ydgwHQYDVR0OBBYEFK+1HzZE4i28"
   "oLIzuqlFR9SspiCIMA4GA1UdDwEB/wQEAwIHgDAMBgNVHRMBAf8EAjAAMBMGA1Ud"
   "JQQMMAoGCCsGAQUFBwMDMBEGCWCGSAGG+EIBAQQEAwIEEDBGBgNVHSAEPzA9MDsG"
   "DCsGAQQBsjEBAgEDAjArMCkGCCsGAQUFBwIBFh1odHRwczovL3NlY3VyZS5jb21v"
   "ZG8ubmV0L0NQUzBCBgNVHR8EOzA5MDegNaAzhjFodHRwOi8vY3JsLnVzZXJ0cnVz"
   "dC5jb20vVVROLVVTRVJGaXJzdC1PYmplY3QuY3JsMCEGA1UdEQQaMBiBFnN1cHBv"
   "cnRAcHJlZW1wdGl2ZS5jb20wDQYJKoZIhvcNAQEFBQADggEBAC+JM26Dokvonudl"
   "JXe/Yun7IBhimkagZUjbk9l/GQWN6i+v1o95UJ1wGJtBdm2+MxbSaPoNTDZR4B+2"
   "lYL9MW57UVmePrnfUPXQKZZG+8gTRDz8+7ol/CEAKmS3MLKCRcH5oe+J5345sGxi"
   "FC/KWNKedTNraW95xlg8NTlL2yRP7TMsjvBxgLmkbaFUoXzPTbQWmtovIagIT8GC"
   "JeXwdFaRjbamiz3Irl+u7x/mhxdza6RvgBYylXRFMudANpeGsV7gDXlnfzpFDKHQ"
   "niVwB7P5sbPFIlmIc+4/xRItkLIRjCVXaepgN9KYu3VOgiSDI6wXiTwP44/LUXQM"
   "hetwa7s=";
const unsigned char _der_tests_cacert_root_cert[] =
   "MIIHPTCCBSWgAwIBAgIBADANBgkqhkiG9w0BAQQFADB5MRAwDgYDVQQKEwdSb290"
   "IENBMR4wHAYDVQQLExVodHRwOi8vd3d3LmNhY2VydC5vcmcxIjAgBgNVBAMTGUNB"
   "IENlcnQgU2lnbmluZyBBdXRob3JpdHkxITAfBgkqhkiG9w0BCQEWEnN1cHBvcnRA"
   "Y2FjZXJ0Lm9yZzAeFw0wMzAzMzAxMjI5NDlaFw0zMzAzMjkxMjI5NDlaMHkxEDAO"
   "BgNVBAoTB1Jvb3QgQ0ExHjAcBgNVBAsTFWh0dHA6Ly93d3cuY2FjZXJ0Lm9yZzEi"
   "MCAGA1UEAxMZQ0EgQ2VydCBTaWduaW5nIEF1dGhvcml0eTEhMB8GCSqGSIb3DQEJ"
   "ARYSc3VwcG9ydEBjYWNlcnQub3JnMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIIC"
   "CgKCAgEAziLA4kZ97DYoB1CW8qAzQIxL8TtmPzHlawI229Z89vGIj053NgVBlfkJ"
   "8BLPRoZzYLdufujAWGSuzbCtRRcMY/pnCujW0r8+55jE8Ez64AO7NV1sId6eINm6"
   "zWYyN3L69wj1x81YyY7nDl7qPv4coRQKFWyGhFtkZip6qUtTefWIonvuLwphK42y"
   "fk1WpRPs6tqSnqxEQR5YYGUFZvjARL3LlPdCfgv3ZWiYUQXw8wWRBB0bF4LsyFe7"
   "w2t6iPGwcswlWyCR7BYCEo8y6RcYSNDHBS4CMEK4JZwFaz+qOqfrU0j36NK2B5jc"
   "G8Y0f3/JHIJ6BVgrCFvzOKKrF11myZjXnhCLotLddJr3cQxyYN/Nb5gznZY0dj4k"
   "epKwDpUeb+agRThHqtdB7Uq3EvbXG4OKDy7YCbZZ16oE/9KTfWgu3YtLq1i6L43q"
   "laegw1SJpfvbi1EinbLDvhG+LJGGi5Z4rSDTii8aP8bQUWWHIbEZAWV/RRyH9XzQ"
   "QUxPKZgh/TMfdQwEUfoZd9vUFBzugcMd9Zi3aQaRIt0AUMyBMawSB3s42mhb5ivU"
   "fslfrejrckzzAeVLIL+aplfKkQABi6F1ITe1Yw1nPkZPcCBnzsXWWdsC4PDSy826"
   "YreQQejdIOQpvGQpQsgi3Hia/0PsmBsJUUtaWsJx8cTLc6nloQsCAwEAAaOCAc4w"
   "ggHKMB0GA1UdDgQWBBQWtTIb1Mfz4OaO873SsDrusjkY0TCBowYDVR0jBIGbMIGY"
   "gBQWtTIb1Mfz4OaO873SsDrusjkY0aF9pHsweTEQMA4GA1UEChMHUm9vdCBDQTEe"
   "MBwGA1UECxMVaHR0cDovL3d3dy5jYWNlcnQub3JnMSIwIAYDVQQDExlDQSBDZXJ0"
   "IFNpZ25pbmcgQXV0aG9yaXR5MSEwHwYJKoZIhvcNAQkBFhJzdXBwb3J0QGNhY2Vy"
   "dC5vcmeCAQAwDwYDVR0TAQH/BAUwAwEB/zAyBgNVHR8EKzApMCegJaAjhiFodHRw"
   "czovL3d3dy5jYWNlcnQub3JnL3Jldm9rZS5jcmwwMAYJYIZIAYb4QgEEBCMWIWh0"
   "dHBzOi8vd3d3LmNhY2VydC5vcmcvcmV2b2tlLmNybDA0BglghkgBhvhCAQgEJxYl"
   "aHR0cDovL3d3dy5jYWNlcnQub3JnL2luZGV4LnBocD9pZD0xMDBWBglghkgBhvhC"
   "AQ0ESRZHVG8gZ2V0IHlvdXIgb3duIGNlcnRpZmljYXRlIGZvciBGUkVFIGhlYWQg"
   "b3ZlciB0byBodHRwOi8vd3d3LmNhY2VydC5vcmcwDQYJKoZIhvcNAQEEBQADggIB"
   "ACjH7pyCArpcgBLKNQodgW+JapnM8mgPf6fhjViVPr3yBsOQWqy1YPaZQwGjiHCc"
   "nWKdpIevZ1gNMDY75q1I08t0AoZxPuIrA2jxNGJARjtT6ij0rPtmlVOKTV39O9lg"
   "18p5aTuxZZKmxoGCXJzN600BiqXfEVWqFcofN8CCmHBh22p8lqOOLlQ+TyGpkO/c"
   "gr/c6EWtTZBzCDyUZbAEmXZ/4rzCahWqlwQ3JNgelE5tDlG+1sSPypZt90Pf6DBl"
   "Jzt7u0NDY8RD97LsaMzhGY4i+5jhe1o+ATc7iwiwovOVThrLm82asduycPAtStvY"
   "sONvRUgzEv/+PDIqVPfE94rwiCPCR/5kenHA0R6mY7AHfqQv0wGP3J8rtsYIqQ+T"
   "SCX8Ev2fQtzzxD72V7DX3WnRBnc0CkvSyqD/HMaMyRa+xMwyN2hzXwj7UfdJUzYF"
   "CpUCTPJ5GhD22Dp1nPMd8aINcGeGG7MW9S/lpOt5hvk9C8JzC6WZrG/8Z7jlLwum"
   "GCSNe9FINSkYQKyTYOGWhlC0elnYjyELn8+CkcY7v2vcB5G5l1YjqrZslMZIBjzk"
   "zk6q5PYvCdxTby78dOs6Y5nCpqyJvKeyRKANihDjbPIky/qbn3BHLt4Ui9SyIAmW"
   "omTxJBzcoTWcFbLUvFUufQb1nA5V9FrWk9p2rSVzTMVD";
const unsigned long _der_tests_cacert_root_cert_size = sizeof(_der_tests_cacert_root_cert);

/*
SEQUENCE(3 elem)
    SEQUENCE(8 elem)
        [0](1)
            INTEGER  2
        INTEGER  0
        SEQUENCE(2 elem)
            OBJECT IDENTIFIER 1.2.840.113549.1.1.4
            NULL
        SEQUENCE(4 elem)
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.4.10
                    PrintableString  Root CA
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.4.11
                    PrintableString  http://www.cacert.org
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.4.3
                    PrintableString  CA Cert Signing Authority
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 1.2.840.113549.1.9.1
                    IA5String support@cacert.org
        SEQUENCE(2 elem)
            UTCTime03-03-30 12:29:49 UTC
            UTCTime33-03-29 12:29:49 UTC
        SEQUENCE(4 elem)
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.4.10
                    PrintableString Root CA
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.4.11
                    PrintableString http://www.cacert.org
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.4.3
                    PrintableString CA Cert Signing Authority
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 1.2.840.113549.1.9.1
                    IA5String support@cacert.org
        SEQUENCE(2 elem)
            SEQUENCE(2 elem)
                OBJECT IDENTIFIER 1.2.840.113549.1.1.1
                NULL
            BIT STRING(1 elem)
        SEQUENCE(2 elem)
            INTEGER (4096 bit)
            INTEGER 65537
        [3](1)
            SEQUENCE(7 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.29.14
                    OCTET STRING(1 elem)
                        OCTET STRING(20 byte) 16B5321BD4C7F3E0E68EF3BDD2B03AEEB23918D1
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.29.35
                    OCTET STRING(1 elem)
                        SEQUENCE(3 elem)
                            [0]
                            [1](1)
                                [4](1)
                                SEQUENCE(4 elem)
                                    SET(1 elem)
                                        SEQUENCE(2 elem)
                                            OBJECT IDENTIFIER 2.5.4.10
                                            PrintableString Root CA
                                    SET(1 elem)
                                        SEQUENCE(2 elem)
                                            OBJECT IDENTIFIER 2.5.4.11
                                            PrintableString http://www.cacert.org
                                    SET(1 elem)
                                        SEQUENCE(2 elem)
                                            OBJECT IDENTIFIER 2.5.4.3
                                            PrintableString CA Cert Signing Authority
                                    SET(1 elem)
                                        SEQUENCE(2 elem)
                                            OBJECT IDENTIFIER 1.2.840.113549.1.9.1
                                            IA5String support@cacert.org
                            [2]
                SEQUENCE(3 elem)
                    OBJECT IDENTIFIER 2.5.29.19
                    BOOLEAN true
                    OCTET STRING(1 elem)
                        SEQUENCE(1 elem)
                            BOOLEAN true
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.29.31
                    OCTET STRING(1 elem)
                        SEQUENCE(1 elem)
                            SEQUENCE(1 elem)
                                [0](1)
                                    [0](1)
                                        [6]
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.16.840.1.113730.1.4
                    OCTET STRING(1 elem)
                        IA5String https://www.cacert.org/revoke.crl
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.16.840.1.113730.1.8
                    OCTET STRING(1 elem)
                        IA5String http://www.cacert.org/index.php?id=10
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.16.840.1.113730.1.13
                    OCTET STRING(1 elem)
                        IA5String To get your own certificate for FREE head over to http://www.cacert.org
    SEQUENCE(2 elem)
        OBJECT IDENTIFIER 1.2.840.113549.1.1.4
        NULL
    BIT STRING(4096 bit)
 */

#define __ASN1_FMTSTRING_FMT "line: %d, type=%d, size=%lu, data=%p, self=%p, next=%p, prev=%p, parent=%p, child=%p"
#define __ASN1_FMTSTRING_VAL(l)  __LINE__, (l)->type, (l)->size, (l)->data, (l), (l)->next, (l)->prev, (l)->parent, (l)->child

#define __ASN1_ERR(l) fprintf(stderr, __ASN1_FMTSTRING_FMT "\n", __ASN1_FMTSTRING_VAL(l)); \
    exit(EXIT_FAILURE)

#define __CHECK_ASN1_HAS(l, w) do { if ((l)->w == NULL) { \
    __ASN1_ERR(l);\
} } while(0)

#define __CHECK_ASN1_HAS_NO(l, w) do { if ((l)->w != NULL) { \
    __ASN1_ERR(l);\
} } while(0)



#define CHECK_ASN1_TYPE(l, t) do { if ((l)->type != (t)) { \
    __ASN1_ERR(l);\
} } while(0)

#define CHECK_ASN1_HAS_CHILD(l) __CHECK_ASN1_HAS(l, child)
#define CHECK_ASN1_HAS_NO_CHILD(l) __CHECK_ASN1_HAS_NO(l, child)
#define CHECK_ASN1_HAS_NEXT(l) __CHECK_ASN1_HAS(l, next)
#define CHECK_ASN1_HAS_NO_NEXT(l) __CHECK_ASN1_HAS_NO(l, next)
#define CHECK_ASN1_HAS_DATA(l) __CHECK_ASN1_HAS(l, data)
#define CHECK_ASN1_HAS_NO_DATA(l) __CHECK_ASN1_HAS_NO(l, data)

#ifdef LTC_DER_TESTS_PRINT_FLEXI
static void _der_tests_print_flexi(ltc_asn1_list* l, unsigned int level)
{
  char buf[1024];
  const char* name = NULL;
  const char* text = NULL;
  ltc_asn1_list* ostring = NULL;
  unsigned int n;

  switch (l->type)
    {
  case LTC_ASN1_EOL:
    name = "EOL";
    snprintf(buf, sizeof(buf),__ASN1_FMTSTRING_FMT "\n", __ASN1_FMTSTRING_VAL(l));
    text = buf;
    break;
  case LTC_ASN1_BOOLEAN:
    name = "BOOLEAN";
    {
      if (*(int*)l->data)
        text = "true";
      else
        text = "false";
    }
    break;
  case LTC_ASN1_INTEGER:
    name = "INTEGER";
    break;
  case LTC_ASN1_SHORT_INTEGER:
    name = "SHORT INTEGER";
    break;
  case LTC_ASN1_BIT_STRING:
    name = "BIT STRING";
    break;
  case LTC_ASN1_OCTET_STRING:
    name = "OCTET STRING";
    {
      unsigned long ostring_l = l->size;
      /* sometimes there's another sequence in an octet string...
       * try to decode that... if it fails print out the octet string
       */
      if (der_decode_sequence_flexi(l->data, &ostring_l, &ostring) == CRYPT_OK) {
          text = "";
      }
      else {
          int r;
          char* s = buf;
          int sz = sizeof(buf);
          for (n = 0; n < l->size; ++n) {
              r = snprintf(s, sz, "%02X", ((unsigned char*)l->data)[n]);
              if (r < 0 || r >= sz) {
                  fprintf(stderr, "%s boom\n", name);
                  exit(EXIT_FAILURE);
              }
              s += r;
              sz -= r;
          }
          text = buf;
      }
    }
    break;
  case LTC_ASN1_NULL:
    name = "NULL";
    text = "";
    break;
  case LTC_ASN1_OBJECT_IDENTIFIER:
    name = "OBJECT IDENTIFIER";
    {
      unsigned long i;
      int r;
      char* s = buf;
      int sz = sizeof(buf);
      for (i = 0; i < l->size; ++i) {
        r = snprintf(s, sz, "%lu.", ((unsigned long*)l->data)[i]);
        if (r < 0 || r >= sz) {
            fprintf(stderr, "%s boom\n", name);
            exit(EXIT_FAILURE);
        }
        s += r;
        sz -= r;
      }
      /* replace the last . with a \0 */
      *(s - 1) = '\0';
      text = buf;
    }
    break;
  case LTC_ASN1_IA5_STRING:
    name = "IA5 STRING";
    text = l->data;
    break;
  case LTC_ASN1_PRINTABLE_STRING:
    name = "PRINTABLE STRING";
    text = l->data;
    break;
  case LTC_ASN1_UTF8_STRING:
    name = "UTF8 STRING";
    break;
  case LTC_ASN1_UTCTIME:
    name = "UTCTIME";
    {
      ltc_utctime* ut = l->data;
      snprintf(buf, sizeof(buf), "%02d-%02d-%02d %02d:%02d:%02d %c%02d:%02d",
          ut->YY, ut->MM, ut->DD, ut->hh, ut->mm, ut->ss,
          ut->off_dir ? '-' : '+', ut->off_hh, ut->off_mm);
      text = buf;
    }
    break;
  case LTC_ASN1_GENERALIZEDTIME:
    name = "GENERALIZED TIME";
    {
      ltc_generalizedtime* gt = l->data;
      if(gt->fs)
         snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%02dZ",
          gt->YYYY, gt->MM, gt->DD, gt->hh, gt->mm, gt->ss, gt->fs);
      else
         snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02dZ",
          gt->YYYY, gt->MM, gt->DD, gt->hh, gt->mm, gt->ss);
      text = buf;
    }
    break;
  case LTC_ASN1_CHOICE:
    name = "CHOICE";
    break;
  case LTC_ASN1_SEQUENCE:
    name = "SEQUENCE";
    text = "";
    break;
  case LTC_ASN1_SET:
    name = "SET";
    text = "";
    break;
  case LTC_ASN1_SETOF:
    name = "SETOF";
    text = "";
    break;
  case LTC_ASN1_RAW_BIT_STRING:
    name = "RAW BIT STRING";
    break;
  case LTC_ASN1_TELETEX_STRING:
    name = "TELETEX STRING";
    text = l->data;
    break;
  case LTC_ASN1_CONSTRUCTED:
    if (l->used & 0x80)
      name = "CONTEXT SPECIFIC";
    else
      name = "CONSTRUCTED";
    snprintf(buf, sizeof(buf), "[%d]", l->used & 0x1f);
    text = buf;
    break;
  case LTC_ASN1_CONTEXT_SPECIFIC:
    name = "CONTEXT SPECIFIC";
    {
      int r;
      char* s = buf;
      int sz = sizeof(buf);
      r = snprintf(s, sz, "[%d] ", l->used & 0x1f);
      if (r < 0 || r >= sz) {
          printf("Context Specific boom");
          exit(EXIT_FAILURE);
      }
      s += r;
      sz -= r;
      for (n = 0; n < l->size; ++n) {
          r = snprintf(s, sz, "%02X", ((unsigned char*)l->data)[n]);
          if (r < 0 || r >= sz) {
              printf("Context Specific boom");
              exit(EXIT_FAILURE);
          }
          s += r;
          sz -= r;
      }
      text = buf;
    }
    break;
  }

  for (n = 0; n < level; ++n) {
     fprintf(stderr, "    ");
  }
  if (name) {
      if (text)
         fprintf(stderr, "%s %s\n", name, text);
      else
         fprintf(stderr, "%s <missing decoding>\n", name);
  }
  else
     fprintf(stderr, "WTF type=%i\n", l->type);

  if (ostring) {
      _der_tests_print_flexi(ostring, level + 1);
      der_free_sequence_flexi(ostring);
  }

  if (l->child)
    _der_tests_print_flexi(l->child, level + 1);

  if (l->next)
    _der_tests_print_flexi(l->next, level);
}
#endif

static void der_cacert_test(void)
{
  unsigned char buf[sizeof(_der_tests_cacert_root_cert)];
  unsigned long len1 = sizeof(buf), len2;

  ltc_asn1_list *decoded_list, *l, *l1, *l2;

  DO(base64_decode(_der_tests_stinky_root_cert, sizeof(_der_tests_stinky_root_cert), buf, &len1));
  len2 = len1;
  DO(der_decode_sequence_flexi(buf, &len2, &decoded_list));
  der_free_sequence_flexi(decoded_list);

  len1 = sizeof(buf);
  DO(base64_decode(_der_tests_cacert_root_cert, sizeof(_der_tests_cacert_root_cert), buf, &len1));
  len2 = len1;
  DO(der_decode_sequence_flexi(buf, &len2, &decoded_list));
  CHECK_ASN1_TYPE(decoded_list, LTC_ASN1_SEQUENCE);
  CHECK_ASN1_HAS_DATA(decoded_list);

  der_sequence_shrink(decoded_list);

  CHECK_ASN1_TYPE(decoded_list, LTC_ASN1_SEQUENCE);
  CHECK_ASN1_HAS_NO_DATA(decoded_list);

#ifdef LTC_DER_TESTS_PRINT_FLEXI
  printf("\n\n--- test print start ---\n\n");
  _der_tests_print_flexi(decoded_list, 0);
  printf("\n\n--- test print end ---\n\n");
#endif

  l = decoded_list;

  /*
SEQUENCE(3 elem)
    SEQUENCE(8 elem)
   */

  CHECK_ASN1_TYPE(l, LTC_ASN1_SEQUENCE);
  CHECK_ASN1_HAS_CHILD(l);
  CHECK_ASN1_HAS_NO_NEXT(l);

  l = l->child;

  CHECK_ASN1_TYPE(l, LTC_ASN1_SEQUENCE);
  CHECK_ASN1_HAS_CHILD(l);
  CHECK_ASN1_HAS_NEXT(l);

  l1 = l->child;

  /*
        [0](1)
            INTEGER  2
   */

  CHECK_ASN1_TYPE(l1, LTC_ASN1_CONSTRUCTED);
  CHECK_ASN1_HAS_CHILD(l1);
  CHECK_ASN1_HAS_NEXT(l1);

  l2 = l1->child;

  CHECK_ASN1_TYPE(l2, LTC_ASN1_INTEGER);
  CHECK_ASN1_HAS_NO_CHILD(l2);
  CHECK_ASN1_HAS_NO_NEXT(l2);

  l1 = l1->next;

  /*
        INTEGER  0
   */

  CHECK_ASN1_TYPE(l1, LTC_ASN1_INTEGER);
  CHECK_ASN1_HAS_NO_CHILD(l1);
  CHECK_ASN1_HAS_NEXT(l1);

  l1 = l1->next;

  /*
        SEQUENCE(2 elem)
            OBJECT IDENTIFIER 1.2.840.113549.1.1.4
            NULL
   */

  CHECK_ASN1_TYPE(l1, LTC_ASN1_SEQUENCE);
  CHECK_ASN1_HAS_CHILD(l1);
  CHECK_ASN1_HAS_NEXT(l1);

  l2 = l1->child;

  CHECK_ASN1_TYPE(l2, LTC_ASN1_OBJECT_IDENTIFIER);
  CHECK_ASN1_HAS_NO_CHILD(l2);
  CHECK_ASN1_HAS_NEXT(l2);

  l2 = l2->next;

  CHECK_ASN1_TYPE(l2, LTC_ASN1_NULL);
  CHECK_ASN1_HAS_NO_CHILD(l2);
  CHECK_ASN1_HAS_NO_NEXT(l2);

  /*
        SEQUENCE(4 elem)
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.4.10
                    PrintableString  Root CA
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.4.11
                    PrintableString  http://www.cacert.org
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 2.5.4.3
                    PrintableString  CA Cert Signing Authority
            SET(1 elem)
                SEQUENCE(2 elem)
                    OBJECT IDENTIFIER 1.2.840.113549.1.9.1
                    IA5String support@cacert.org
   */

  l = l->next;

  /*
    SEQUENCE(2 elem)
        OBJECT IDENTIFIER 1.2.840.113549.1.1.4
        NULL
   */

  CHECK_ASN1_TYPE(l, LTC_ASN1_SEQUENCE);
  CHECK_ASN1_HAS_CHILD(l);
  CHECK_ASN1_HAS_NEXT(l);

  l1 = l->child;

  CHECK_ASN1_TYPE(l1, LTC_ASN1_OBJECT_IDENTIFIER);
  CHECK_ASN1_HAS_NO_CHILD(l1);
  CHECK_ASN1_HAS_NEXT(l1);

  l1 = l1->next;

  CHECK_ASN1_TYPE(l1, LTC_ASN1_NULL);
  CHECK_ASN1_HAS_NO_CHILD(l1);
  CHECK_ASN1_HAS_NO_NEXT(l1);

  l = l->next;

  /*
    BIT STRING(4096 bit)
   */

  CHECK_ASN1_TYPE(l, LTC_ASN1_BIT_STRING);
  CHECK_ASN1_HAS_NO_CHILD(l);
  CHECK_ASN1_HAS_NO_NEXT(l);

  der_free_sequence_flexi(decoded_list);
}

static void der_set_test(void)
{
   ltc_asn1_list list[10];
   static const unsigned char oct_str[] = { 1, 2, 3, 4 };
   static const unsigned char bin_str[] = { 1, 0, 0, 1 };
   static const unsigned long int_val   = 12345678UL;

   unsigned char strs[10][10], outbuf[128];
   unsigned long x, val, outlen;

   /* make structure and encode it */
   LTC_SET_ASN1(list, 0, LTC_ASN1_OCTET_STRING,  oct_str, sizeof(oct_str));
   LTC_SET_ASN1(list, 1, LTC_ASN1_BIT_STRING,    bin_str, sizeof(bin_str));
   LTC_SET_ASN1(list, 2, LTC_ASN1_SHORT_INTEGER, &int_val, 1);

   /* encode it */
   outlen = sizeof(outbuf);
   DO(der_encode_set(list, 3, outbuf, &outlen));

   /* first let's test the set_decoder out of order to see what happens, we should get all the fields we expect even though they're in a diff order */
   LTC_SET_ASN1(list, 0, LTC_ASN1_BIT_STRING,    strs[1], sizeof(strs[1]));
   LTC_SET_ASN1(list, 1, LTC_ASN1_SHORT_INTEGER, &val, 1);
   LTC_SET_ASN1(list, 2, LTC_ASN1_OCTET_STRING,  strs[0], sizeof(strs[0]));

   DO(der_decode_set(outbuf, outlen, list, 3));

   /* now compare the items */
   if (memcmp(strs[0], oct_str, sizeof(oct_str))) {
      fprintf(stderr, "error decoding set using der_decode_set (oct_str is wrong):\n");
      exit(EXIT_FAILURE);
   }

   if (memcmp(strs[1], bin_str, sizeof(bin_str))) {
      fprintf(stderr, "error decoding set using der_decode_set (bin_str is wrong):\n");
      exit(EXIT_FAILURE);
   }

   if (val != int_val) {
      fprintf(stderr, "error decoding set using der_decode_set (int_val is wrong):\n");
      exit(EXIT_FAILURE);
   }

   strcpy((char*)strs[0], "one");
   strcpy((char*)strs[1], "one2");
   strcpy((char*)strs[2], "two");
   strcpy((char*)strs[3], "aaa");
   strcpy((char*)strs[4], "aaaa");
   strcpy((char*)strs[5], "aab");
   strcpy((char*)strs[6], "aaab");
   strcpy((char*)strs[7], "bbb");
   strcpy((char*)strs[8], "bbba");
   strcpy((char*)strs[9], "bbbb");

   for (x = 0; x < 10; x++) {
       LTC_SET_ASN1(list, x, LTC_ASN1_PRINTABLE_STRING, strs[x], strlen((char*)strs[x]));
   }

   outlen = sizeof(outbuf);
   DO(der_encode_setof(list, 10, outbuf, &outlen));

   for (x = 0; x < 10; x++) {
       LTC_SET_ASN1(list, x, LTC_ASN1_PRINTABLE_STRING, strs[x], sizeof(strs[x]) - 1);
   }
   XMEMSET(strs, 0, sizeof(strs));

   DO(der_decode_set(outbuf, outlen, list, 10));

   /* now compare */
   for (x = 1; x < 10; x++) {
      if (!(strlen((char*)strs[x-1]) <= strlen((char*)strs[x])) && strcmp((char*)strs[x-1], (char*)strs[x]) >= 0) {
         fprintf(stderr, "error SET OF order at %lu is wrong\n", x);
         exit(EXIT_FAILURE);
      }
   }

}


/* we are encoding

  SEQUENCE {
     PRINTABLE "printable"
     IA5       "ia5"
     SEQUENCE {
        INTEGER 12345678
        UTCTIME { 91, 5, 6, 16, 45, 40, 1, 7, 0 }
        GENERALIZEDTIME { 2017, 03, 21, 10, 21, 12, 4, 1, 2, 0 }
        SEQUENCE {
           OCTET STRING { 1, 2, 3, 4 }
           BIT STRING   { 1, 0, 0, 1 }
           SEQUENCE {
              OID       { 1, 2, 840, 113549 }
              NULL
              SET OF {
                 PRINTABLE "333"  -- WILL GET SORTED
                 PRINTABLE "222"
           }
        }
     }
  }

*/

static void der_flexi_test(void)
{
   static const char printable_str[]    = "printable";
   static const char set1_str[]         = "333";
   static const char set2_str[]         = "222";
   static const char ia5_str[]          = "ia5";
   static const unsigned long int_val   = 12345678UL;
   static const ltc_utctime   utctime   = { 91, 5, 6, 16, 45, 40, 1, 7, 0 };
   static const ltc_generalizedtime gtime = { 2017, 03, 21, 10, 21, 12, 421, 1, 2, 0 };
   static const unsigned char oct_str[] = { 1, 2, 3, 4 };
   static const unsigned char bit_str[] = { 1, 0, 0, 1 };
   static const unsigned long oid_str[] = { 1, 2, 840, 113549 };

   unsigned char encode_buf[192];
   unsigned long encode_buf_len, decode_len;

   ltc_asn1_list static_list[5][4], *decoded_list, *l;

   /* build list */
   LTC_SET_ASN1(static_list[0], 0, LTC_ASN1_PRINTABLE_STRING, (void *)printable_str, strlen(printable_str));
   LTC_SET_ASN1(static_list[0], 1, LTC_ASN1_IA5_STRING,       (void *)ia5_str,       strlen(ia5_str));
   LTC_SET_ASN1(static_list[0], 2, LTC_ASN1_SEQUENCE,         static_list[1],   4);

   LTC_SET_ASN1(static_list[1], 0, LTC_ASN1_SHORT_INTEGER,    (void *)&int_val,         1);
   LTC_SET_ASN1(static_list[1], 1, LTC_ASN1_UTCTIME,          (void *)&utctime,         1);
   LTC_SET_ASN1(static_list[1], 2, LTC_ASN1_GENERALIZEDTIME,  (void *)&gtime,           1);
   LTC_SET_ASN1(static_list[1], 3, LTC_ASN1_SEQUENCE,         static_list[2],   3);

   LTC_SET_ASN1(static_list[2], 0, LTC_ASN1_OCTET_STRING,     (void *)oct_str,          4);
   LTC_SET_ASN1(static_list[2], 1, LTC_ASN1_BIT_STRING,       (void *)bit_str,          4);
   LTC_SET_ASN1(static_list[2], 2, LTC_ASN1_SEQUENCE,         static_list[3],   3);

   LTC_SET_ASN1(static_list[3], 0, LTC_ASN1_OBJECT_IDENTIFIER,(void *)oid_str,          4);
   LTC_SET_ASN1(static_list[3], 1, LTC_ASN1_NULL,             NULL,             0);
   LTC_SET_ASN1(static_list[3], 2, LTC_ASN1_SETOF,            static_list[4],   2);

   LTC_SET_ASN1(static_list[4], 0, LTC_ASN1_PRINTABLE_STRING, set1_str, strlen(set1_str));
   LTC_SET_ASN1(static_list[4], 1, LTC_ASN1_PRINTABLE_STRING, set2_str, strlen(set2_str));

   /* encode it */
   encode_buf_len = sizeof(encode_buf);
   DO(der_encode_sequence(&static_list[0][0], 3, encode_buf, &encode_buf_len));

#if 0
   {
     FILE *f;
     f = fopen("t.bin", "wb");
     fwrite(encode_buf, 1, encode_buf_len, f);
     fclose(f);
   }
#endif

   /* decode with flexi */
   decode_len = encode_buf_len;
   DO(der_decode_sequence_flexi(encode_buf, &decode_len, &decoded_list));

   if (decode_len != encode_buf_len) {
      fprintf(stderr, "Decode len of %lu does not match encode len of %lu \n", decode_len, encode_buf_len);
      exit(EXIT_FAILURE);
   }

   /* we expect l->next to be NULL and l->child to not be */
   l = decoded_list;
   if (l->next != NULL || l->child == NULL) {
      fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
      exit(EXIT_FAILURE);
   }

   /* we expect a SEQUENCE */
      if (l->type != LTC_ASN1_SEQUENCE) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }
      l = l->child;

   /* PRINTABLE STRING */
      /* we expect printable_str */
      if (l->next == NULL || l->child != NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_PRINTABLE_STRING) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->size != strlen(printable_str) || memcmp(printable_str, l->data, l->size)) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      /* move to next */
      l = l->next;

   /* IA5 STRING */
      /* we expect ia5_str */
      if (l->next == NULL || l->child != NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_IA5_STRING) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->size != strlen(ia5_str) || memcmp(ia5_str, l->data, l->size)) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      /* move to next */
      l = l->next;

   /* expect child anve move down */

      if (l->next != NULL || l->child == NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_SEQUENCE) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }
      l = l->child;


   /* INTEGER */

      if (l->next == NULL || l->child != NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_INTEGER) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (mp_cmp_d(l->data, 12345678UL) != LTC_MP_EQ) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      /* move to next */
      l = l->next;

   /* UTCTIME */

      if (l->next == NULL || l->child != NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_UTCTIME) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (memcmp(l->data, &utctime, sizeof(utctime))) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      /* move to next */
      l = l->next;

   /* GeneralizedTime */

      if (l->next == NULL || l->child != NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_GENERALIZEDTIME) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (memcmp(l->data, &gtime, sizeof(gtime))) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      /* move to next */
      l = l->next;

   /* expect child anve move down */

      if (l->next != NULL || l->child == NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_SEQUENCE) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }
      l = l->child;


   /* OCTET STRING */
      /* we expect oct_str */
      if (l->next == NULL || l->child != NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_OCTET_STRING) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->size != sizeof(oct_str) || memcmp(oct_str, l->data, l->size)) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      /* move to next */
      l = l->next;

   /* BIT STRING */
      /* we expect oct_str */
      if (l->next == NULL || l->child != NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_BIT_STRING) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->size != sizeof(bit_str) || memcmp(bit_str, l->data, l->size)) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      /* move to next */
      l = l->next;

   /* expect child anve move down */

      if (l->next != NULL || l->child == NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_SEQUENCE) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }
      l = l->child;


   /* OID STRING */
      /* we expect oid_str */
      if (l->next == NULL || l->child != NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_OBJECT_IDENTIFIER) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->size != sizeof(oid_str)/sizeof(oid_str[0]) || memcmp(oid_str, l->data, l->size*sizeof(oid_str[0]))) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      /* move to next */
      l = l->next;

   /* NULL */
      if (l->type != LTC_ASN1_NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      /* move to next */
      l = l->next;

   /* expect child anve move down */
      if (l->next != NULL || l->child == NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_SET) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }
      l = l->child;

   /* PRINTABLE STRING */
      /* we expect printable_str */
      if (l->next == NULL || l->child != NULL) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->type != LTC_ASN1_PRINTABLE_STRING) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

/* note we compare set2_str FIRST because the SET OF is sorted and "222" comes before "333" */
      if (l->size != strlen(set2_str) || memcmp(set2_str, l->data, l->size)) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      /* move to next */
      l = l->next;

   /* PRINTABLE STRING */
      /* we expect printable_str */
      if (l->type != LTC_ASN1_PRINTABLE_STRING) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }

      if (l->size != strlen(set1_str) || memcmp(set1_str, l->data, l->size)) {
         fprintf(stderr, "(%d), %d, %lu, next=%p, prev=%p, parent=%p, child=%p\n", __LINE__, l->type, l->size, l->next, l->prev, l->parent, l->child);
         exit(EXIT_FAILURE);
      }


   der_sequence_free(l);

}

static int der_choice_test(void)
{
   ltc_asn1_list types[7], host[1];
   unsigned char bitbuf[10], octetbuf[10], ia5buf[10], printbuf[10], outbuf[256], x, y;
   unsigned long integer, oidbuf[10], outlen, inlen;
   void          *mpinteger;
   ltc_utctime   utctime = { 91, 5, 6, 16, 45, 40, 1, 7, 0 };
   ltc_generalizedtime gtime = { 2038, 01, 19, 3, 14, 8, 0, 0, 0, 0 };

   /* setup variables */
   for (x = 0; x < sizeof(bitbuf); x++)   { bitbuf[x]   = x & 1; }
   for (x = 0; x < sizeof(octetbuf); x++) { octetbuf[x] = x;     }
   for (x = 0; x < sizeof(ia5buf); x++)   { ia5buf[x]   = 'a';   }
   for (x = 0; x < sizeof(printbuf); x++) { printbuf[x] = 'a';   }
   integer = 1;
   for (x = 0; x < sizeof(oidbuf)/sizeof(oidbuf[0]); x++)   { oidbuf[x] = x + 1;   }
   DO(mp_init(&mpinteger));

   for (x = 0; x < 14; x++) {
       /* setup list */
       LTC_SET_ASN1(types, 0, LTC_ASN1_PRINTABLE_STRING, printbuf, sizeof(printbuf));
       LTC_SET_ASN1(types, 1, LTC_ASN1_BIT_STRING, bitbuf, sizeof(bitbuf));
       LTC_SET_ASN1(types, 2, LTC_ASN1_OCTET_STRING, octetbuf, sizeof(octetbuf));
       LTC_SET_ASN1(types, 3, LTC_ASN1_IA5_STRING, ia5buf, sizeof(ia5buf));
       if (x > 7) {
          LTC_SET_ASN1(types, 4, LTC_ASN1_SHORT_INTEGER, &integer, 1);
       } else {
          LTC_SET_ASN1(types, 4, LTC_ASN1_INTEGER, mpinteger, 1);
       }
       LTC_SET_ASN1(types, 5, LTC_ASN1_OBJECT_IDENTIFIER, oidbuf, sizeof(oidbuf)/sizeof(oidbuf[0]));
       if (x > 7) {
          LTC_SET_ASN1(types, 6, LTC_ASN1_UTCTIME, &utctime, 1);
       } else {
          LTC_SET_ASN1(types, 6, LTC_ASN1_GENERALIZEDTIME, &gtime, 1);
       }

       LTC_SET_ASN1(host, 0, LTC_ASN1_CHOICE, types, 7);


       /* encode */
       outlen = sizeof(outbuf);
       DO(der_encode_sequence(&types[x>6?x-7:x], 1, outbuf, &outlen));

       /* decode it */
       inlen = outlen;
       DO(der_decode_sequence(outbuf, inlen, &host[0], 1));

       for (y = 0; y < 7; y++) {
           if (types[y].used && y != (x>6?x-7:x)) {
               fprintf(stderr, "CHOICE, flag %u in trial %u was incorrectly set to one\n", y, x);
               return 1;
           }
           if (!types[y].used && y == (x>6?x-7:x)) {
               fprintf(stderr, "CHOICE, flag %u in trial %u was incorrectly set to zero\n", y, x);
               return 1;
           }
      }
  }
  mp_clear(mpinteger);
  return 0;
}


static void _der_recursion_limit(void)
{
   int failed = 0;
   unsigned int n;
   unsigned long integer = 123, s;
   ltc_asn1_list seqs[LTC_DER_MAX_RECURSION + 2], dummy[1], *flexi;
   unsigned char buf[2048];
   LTC_SET_ASN1(dummy, 0, LTC_ASN1_SHORT_INTEGER, &integer, 1);
   LTC_SET_ASN1(seqs, LTC_DER_MAX_RECURSION + 1, LTC_ASN1_SEQUENCE, dummy, 1);
   for (n = 0; n < LTC_DER_MAX_RECURSION + 1; ++n) {
      LTC_SET_ASN1(seqs, LTC_DER_MAX_RECURSION - n, LTC_ASN1_SEQUENCE, &seqs[LTC_DER_MAX_RECURSION - n + 1], 1);
   }
   s = sizeof(buf);
   DO(der_encode_sequence(seqs, 1, buf, &s));
   DO(der_decode_sequence(buf, s, seqs, 1));
   SHOULD_FAIL(der_decode_sequence_flexi(buf, &s, &flexi));
   if (failed) exit(EXIT_FAILURE);
}

int der_test(void)
{
   unsigned long x, y, z, zz, oid[2][32];
   unsigned char buf[3][2048];
   void *a, *b, *c, *d, *e, *f, *g;

   static const unsigned char rsa_oid_der[] = { 0x06, 0x06, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d };
   static const unsigned long rsa_oid[]     = { 1, 2, 840, 113549 };

   static const unsigned char rsa_ia5[]     = "test1@rsa.com";
   static const unsigned char rsa_ia5_der[] = { 0x16, 0x0d, 0x74, 0x65, 0x73, 0x74, 0x31,
                                                0x40, 0x72, 0x73, 0x61, 0x2e, 0x63, 0x6f, 0x6d };

   static const unsigned char rsa_printable[] = "Test User 1";
   static const unsigned char rsa_printable_der[] = { 0x13, 0x0b, 0x54, 0x65, 0x73, 0x74, 0x20, 0x55,
                                                      0x73, 0x65, 0x72, 0x20, 0x31 };

   static const ltc_utctime   rsa_time1 = { 91, 5, 6, 16, 45, 40, 1, 7, 0 };
   static const ltc_utctime   rsa_time2 = { 91, 5, 6, 23, 45, 40, 0, 0, 0 };
   ltc_utctime                tmp_time;

   static const unsigned char rsa_time1_der[] = { 0x17, 0x11, 0x39, 0x31, 0x30, 0x35, 0x30, 0x36, 0x31, 0x36, 0x34, 0x35, 0x34, 0x30, 0x2D, 0x30, 0x37, 0x30, 0x30 };
   static const unsigned char rsa_time2_der[] = { 0x17, 0x0d, 0x39, 0x31, 0x30, 0x35, 0x30, 0x36, 0x32, 0x33, 0x34, 0x35, 0x34, 0x30, 0x5a };

   static const wchar_t utf8_1[]           = { 0x0041, 0x2262, 0x0391, 0x002E };
   static const unsigned char utf8_1_der[] = { 0x0C, 0x07, 0x41, 0xE2, 0x89, 0xA2, 0xCE, 0x91, 0x2E };
   static const wchar_t utf8_2[]           = { 0xD55C, 0xAD6D, 0xC5B4 };
   static const unsigned char utf8_2_der[] = { 0x0C, 0x09, 0xED, 0x95, 0x9C, 0xEA, 0xB5, 0xAD, 0xEC, 0x96, 0xB4 };

   unsigned char utf8_buf[32];
   wchar_t utf8_out[32];


   _der_recursion_limit();
   der_cacert_test();

   DO(mp_init_multi(&a, &b, &c, &d, &e, &f, &g, NULL));
   for (zz = 0; zz < 16; zz++) {
#ifdef USE_TFM
      for (z = 0; z < 256; z++) {
#else
      for (z = 0; z < 1024; z++) {
#endif
         if (yarrow_read(buf[0], z, &yarrow_prng) != z) {
            fprintf(stderr, "Failed to read %lu bytes from yarrow\n", z);
            return 1;
         }
         DO(mp_read_unsigned_bin(a, buf[0], z));
/*          if (mp_iszero(a) == LTC_MP_NO) { a.sign = buf[0][0] & 1 ? LTC_MP_ZPOS : LTC_MP_NEG; } */
         x = sizeof(buf[0]);
         DO(der_encode_integer(a, buf[0], &x));
         DO(der_length_integer(a, &y));
         if (y != x) { fprintf(stderr, "DER INTEGER size mismatch\n"); return 1; }
         mp_set_int(b, 0);
         DO(der_decode_integer(buf[0], y, b));
         if (y != x || mp_cmp(a, b) != LTC_MP_EQ) {
            fprintf(stderr, "%lu: %lu vs %lu\n", z, x, y);
            mp_clear_multi(a, b, c, d, e, f, g, NULL);
            return 1;
         }
      }
   }

/* test short integer */
   for (zz = 0; zz < 256; zz++) {
      for (z = 1; z < 4; z++) {
         if (yarrow_read(buf[2], z, &yarrow_prng) != z) {
            fprintf(stderr, "Failed to read %lu bytes from yarrow\n", z);
            return 1;
         }
         /* encode with normal */
         DO(mp_read_unsigned_bin(a, buf[2], z));

         x = sizeof(buf[0]);
         DO(der_encode_integer(a, buf[0], &x));

         /* encode with short */
         y = sizeof(buf[1]);
         DO(der_encode_short_integer(mp_get_int(a), buf[1], &y));
         if (x != y || memcmp(buf[0], buf[1], x)) {
            fprintf(stderr, "DER INTEGER short encoding failed, %lu, %lu, 0x%lX\n", x, y, mp_get_int(a));
            for (zz = 0; zz < z; zz++) fprintf(stderr, "%02x ", buf[2][zz]);
            fprintf(stderr, "\n");
            for (z = 0; z < x; z++) fprintf(stderr, "%02x ", buf[0][z]);
            fprintf(stderr, "\n");
            for (z = 0; z < y; z++) fprintf(stderr, "%02x ", buf[1][z]);
            fprintf(stderr, "\n");
            mp_clear_multi(a, b, c, d, e, f, g, NULL);
            return 1;
         }

         /* decode it */
         x = 0;
         DO(der_decode_short_integer(buf[1], y, &x));
         if (x != mp_get_int(a)) {
            fprintf(stderr, "DER INTEGER short decoding failed, %lu, %lu\n", x, mp_get_int(a));
            mp_clear_multi(a, b, c, d, e, f, g, NULL);
            return 1;
         }
      }
   }
   mp_clear_multi(a, b, c, d, e, f, g, NULL);


/* Test bit string */
   for (zz = 1; zz < 1536; zz++) {
       yarrow_read(buf[0], zz, &yarrow_prng);
       for (z = 0; z < zz; z++) {
           buf[0][z] &= 0x01;
       }
       x = sizeof(buf[1]);
       DO(der_encode_bit_string(buf[0], zz, buf[1], &x));
       DO(der_length_bit_string(zz, &y));
       if (y != x) {
          fprintf(stderr, "\nDER BIT STRING length of encoded not match expected : %lu, %lu, %lu\n", z, x, y);
          return 1;
       }

       y = sizeof(buf[2]);
       DO(der_decode_bit_string(buf[1], x, buf[2], &y));
       if (y != zz || memcmp(buf[0], buf[2], zz)) {
          fprintf(stderr, "%lu, %lu, %d\n", y, zz, memcmp(buf[0], buf[2], zz));
          return 1;
       }
   }

/* Test octet string */
   for (zz = 1; zz < 1536; zz++) {
       yarrow_read(buf[0], zz, &yarrow_prng);
       x = sizeof(buf[1]);
       DO(der_encode_octet_string(buf[0], zz, buf[1], &x));
       DO(der_length_octet_string(zz, &y));
       if (y != x) {
          fprintf(stderr, "\nDER OCTET STRING length of encoded not match expected : %lu, %lu, %lu\n", z, x, y);
          return 1;
       }
       y = sizeof(buf[2]);
       DO(der_decode_octet_string(buf[1], x, buf[2], &y));
       if (y != zz || memcmp(buf[0], buf[2], zz)) {
          fprintf(stderr, "%lu, %lu, %d\n", y, zz, memcmp(buf[0], buf[2], zz));
          return 1;
       }
   }

/* test OID */
   x = sizeof(buf[0]);
   DO(der_encode_object_identifier((unsigned long*)rsa_oid, sizeof(rsa_oid)/sizeof(rsa_oid[0]), buf[0], &x));
   if (x != sizeof(rsa_oid_der) || memcmp(rsa_oid_der, buf[0], x)) {
      fprintf(stderr, "rsa_oid_der encode failed to match, %lu, ", x);
      for (y = 0; y < x; y++) fprintf(stderr, "%02x ", buf[0][y]);
      fprintf(stderr, "\n");
      return 1;
   }

   y = sizeof(oid[0])/sizeof(oid[0][0]);
   DO(der_decode_object_identifier(buf[0], x, oid[0], &y));
   if (y != sizeof(rsa_oid)/sizeof(rsa_oid[0]) || memcmp(rsa_oid, oid[0], sizeof(rsa_oid))) {
      fprintf(stderr, "rsa_oid_der decode failed to match, %lu, ", y);
      for (z = 0; z < y; z++) fprintf(stderr, "%lu ", oid[0][z]);
      fprintf(stderr, "\n");
      return 1;
   }

   /* do random strings */
   for (zz = 0; zz < 5000; zz++) {
       /* pick a random number of words */
       yarrow_read(buf[0], 4, &yarrow_prng);
       LOAD32L(z, buf[0]);
       z = 2 + (z % ((sizeof(oid[0])/sizeof(oid[0][0])) - 2));

       /* fill them in */
       oid[0][0] = buf[0][0] % 3;
       oid[0][1] = buf[0][1] % 40;

       for (y = 2; y < z; y++) {
          yarrow_read(buf[0], 4, &yarrow_prng);
          LOAD32L(oid[0][y], buf[0]);
       }

       /* encode it */
       x = sizeof(buf[0]);
       DO(der_encode_object_identifier(oid[0], z, buf[0], &x));
       DO(der_length_object_identifier(oid[0], z, &y));
       if (x != y) {
          fprintf(stderr, "Random OID %lu test failed, length mismatch: %lu, %lu\n", z, x, y);
          for (x = 0; x < z; x++) fprintf(stderr, "%lu\n", oid[0][x]);
          return 1;
       }

       /* decode it */
       y = sizeof(oid[0])/sizeof(oid[0][0]);
       DO(der_decode_object_identifier(buf[0], x, oid[1], &y));
       if (y != z) {
          fprintf(stderr, "Random OID %lu test failed, decode length mismatch: %lu, %lu\n", z, x, y);
          return 1;
       }
       if (memcmp(oid[0], oid[1], sizeof(oid[0][0]) * z)) {
          fprintf(stderr, "Random OID %lu test failed, decoded values wrong\n", z);
          for (x = 0; x < z; x++) fprintf(stderr, "%lu\n", oid[0][x]);
          fprintf(stderr, "\n\n Got \n\n");
          for (x = 0; x < z; x++) fprintf(stderr, "%lu\n", oid[1][x]);
          return 1;
       }
   }

/* IA5 string */
   x = sizeof(buf[0]);
   DO(der_encode_ia5_string(rsa_ia5, strlen((char*)rsa_ia5), buf[0], &x));
   if (x != sizeof(rsa_ia5_der) || memcmp(buf[0], rsa_ia5_der, x)) {
      fprintf(stderr, "IA5 encode failed: %lu, %lu\n", x, (unsigned long)sizeof(rsa_ia5_der));
      return 1;
   }
   DO(der_length_ia5_string(rsa_ia5, strlen((char*)rsa_ia5), &y));
   if (y != x) {
      fprintf(stderr, "IA5 length failed to match: %lu, %lu\n", x, y);
      return 1;
   }
   y = sizeof(buf[1]);
   DO(der_decode_ia5_string(buf[0], x, buf[1], &y));
   if (y != strlen((char*)rsa_ia5) || memcmp(buf[1], rsa_ia5, strlen((char*)rsa_ia5))) {
       fprintf(stderr, "DER IA5 failed test vector\n");
       return 1;
   }

/* Printable string */
   x = sizeof(buf[0]);
   DO(der_encode_printable_string(rsa_printable, strlen((char*)rsa_printable), buf[0], &x));
   if (x != sizeof(rsa_printable_der) || memcmp(buf[0], rsa_printable_der, x)) {
      fprintf(stderr, "PRINTABLE encode failed: %lu, %lu\n", x, (unsigned long)sizeof(rsa_printable_der));
      return 1;
   }
   DO(der_length_printable_string(rsa_printable, strlen((char*)rsa_printable), &y));
   if (y != x) {
      fprintf(stderr, "printable length failed to match: %lu, %lu\n", x, y);
      return 1;
   }
   y = sizeof(buf[1]);
   DO(der_decode_printable_string(buf[0], x, buf[1], &y));
   if (y != strlen((char*)rsa_printable) || memcmp(buf[1], rsa_printable, strlen((char*)rsa_printable))) {
       fprintf(stderr, "DER printable failed test vector\n");
       return 1;
   }

/* Test UTC time */
   x = sizeof(buf[0]);
   DO(der_encode_utctime((ltc_utctime*)&rsa_time1, buf[0], &x));
   if (x != sizeof(rsa_time1_der) || memcmp(buf[0], rsa_time1_der, x)) {
      fprintf(stderr, "UTCTIME encode of rsa_time1 failed: %lu, %lu\n", x, (unsigned long)sizeof(rsa_time1_der));
      fprintf(stderr, "\n\n");
      for (y = 0; y < x; y++) fprintf(stderr, "%02x ", buf[0][y]);
      fprintf(stderr, "\n");
      return 1;
   }
   DO(der_length_utctime((ltc_utctime*)&rsa_time1, &y));
   if (y != x) {
      fprintf(stderr, "UTCTIME length failed to match for rsa_time1: %lu, %lu\n", x, y);
      return 1;
   }
   DO(der_decode_utctime(buf[0], &y, &tmp_time));
   if (y != x || memcmp(&rsa_time1, &tmp_time, sizeof(ltc_utctime))) {
      fprintf(stderr, "UTCTIME decode failed for rsa_time1: %lu %lu\n", x, y);
fprintf(stderr, "\n\n%u %u %u %u %u %u %u %u %u\n\n",
tmp_time.YY,
tmp_time.MM,
tmp_time.DD,
tmp_time.hh,
tmp_time.mm,
tmp_time.ss,
tmp_time.off_dir,
tmp_time.off_mm,
tmp_time.off_hh);
      return 1;
   }

   x = sizeof(buf[0]);
   DO(der_encode_utctime((ltc_utctime*)&rsa_time2, buf[0], &x));
   if (x != sizeof(rsa_time2_der) || memcmp(buf[0], rsa_time2_der, x)) {
      fprintf(stderr, "UTCTIME encode of rsa_time2 failed: %lu, %lu\n", x, (unsigned long)sizeof(rsa_time1_der));
      fprintf(stderr, "\n\n");
      for (y = 0; y < x; y++) fprintf(stderr, "%02x ", buf[0][y]);
      fprintf(stderr, "\n");
      return 1;
   }
   DO(der_length_utctime((ltc_utctime*)&rsa_time2, &y));
   if (y != x) {
      fprintf(stderr, "UTCTIME length failed to match for rsa_time2: %lu, %lu\n", x, y);
      return 1;
   }
   DO(der_decode_utctime(buf[0], &y, &tmp_time));
   if (y != x || memcmp(&rsa_time2, &tmp_time, sizeof(ltc_utctime))) {
      fprintf(stderr, "UTCTIME decode failed for rsa_time2: %lu %lu\n", x, y);
fprintf(stderr, "\n\n%u %u %u %u %u %u %u %u %u\n\n",
tmp_time.YY,
tmp_time.MM,
tmp_time.DD,
tmp_time.hh,
tmp_time.mm,
tmp_time.ss,
tmp_time.off_dir,
tmp_time.off_mm,
tmp_time.off_hh);


      return 1;
   }

   /* UTF 8 */
     /* encode it */
     x = sizeof(utf8_buf);
     DO(der_encode_utf8_string(utf8_1, sizeof(utf8_1) / sizeof(utf8_1[0]), utf8_buf, &x));
     DO(der_length_utf8_string(utf8_1, sizeof(utf8_1) / sizeof(utf8_1[0]), &y));
     if (x != sizeof(utf8_1_der) || memcmp(utf8_buf, utf8_1_der, x) || x != y) {
        fprintf(stderr, "DER UTF8_1 encoded to %lu bytes\n", x);
        for (y = 0; y < x; y++) fprintf(stderr, "%02x ", (unsigned)utf8_buf[y]);
        fprintf(stderr, "\n");
        return 1;
     }
     /* decode it */
     y = sizeof(utf8_out) / sizeof(utf8_out[0]);
     DO(der_decode_utf8_string(utf8_buf, x, utf8_out, &y));
     if (y != (sizeof(utf8_1) / sizeof(utf8_1[0])) || memcmp(utf8_1, utf8_out, y * sizeof(wchar_t))) {
        fprintf(stderr, "DER UTF8_1 decoded to %lu wchar_t\n", y);
        for (x = 0; x < y; x++) fprintf(stderr, "%04lx ", (unsigned long)utf8_out[x]);
        fprintf(stderr, "\n");
        return 1;
     }

     /* encode it */
     x = sizeof(utf8_buf);
     DO(der_encode_utf8_string(utf8_2, sizeof(utf8_2) / sizeof(utf8_2[0]), utf8_buf, &x));
     if (x != sizeof(utf8_2_der) || memcmp(utf8_buf, utf8_2_der, x)) {
        fprintf(stderr, "DER UTF8_2 encoded to %lu bytes\n", x);
        for (y = 0; y < x; y++) fprintf(stderr, "%02x ", (unsigned)utf8_buf[y]);
        fprintf(stderr, "\n");
        return 1;
     }
     /* decode it */
     y = sizeof(utf8_out) / sizeof(utf8_out[0]);
     DO(der_decode_utf8_string(utf8_buf, x, utf8_out, &y));
     if (y != (sizeof(utf8_2) / sizeof(utf8_2[0])) || memcmp(utf8_2, utf8_out, y * sizeof(wchar_t))) {
        fprintf(stderr, "DER UTF8_2 decoded to %lu wchar_t\n", y);
        for (x = 0; x < y; x++) fprintf(stderr, "%04lx ", (unsigned long)utf8_out[x]);
        fprintf(stderr, "\n");
        return 1;
     }


   der_set_test();
   der_flexi_test();
   return der_choice_test();
}

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
