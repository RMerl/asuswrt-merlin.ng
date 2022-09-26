#include <string.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include "aws_sigv4.h"

#define AWS_SIGV4_AUTH_HEADER_NAME            "Authorization"
#define AWS_SIGV4_SIGNING_ALGORITHM           "AWS4-HMAC-SHA256"
#define AWS_SIGV4_HEX_SHA256_LENGTH           SHA256_DIGEST_LENGTH * 2
#define AWS_SIGV4_AUTH_HEADER_MAX_LEN         1024
#define AWS_SIGV4_CANONICAL_REQUEST_BUF_LEN   1024
#define AWS_SIGV4_STRING_TO_SIGN_BUF_LEN      1024
#define AWS_SIGV4_KEY_BUF_LEN                 33
#define AWS_SIGV4_MAX_NUM_QUERY_COMPONENTS    50

typedef int (*aws_sigv4_compare_func_t)(const void*, const void*);

static int aws_sigv4_kv_cmp(aws_sigv4_kv_t* p1,
                            aws_sigv4_kv_t* p2)
{
  size_t len = p1->key.len <= p2->key.len ? p1->key.len : p2->key.len;
  return strncmp((char*) p1->key.data, (char*) p2->key.data, len);
}

static unsigned char* construct_query_str(unsigned char*  dst_cstr,
                                          aws_sigv4_kv_t* query_params,
                                          size_t          query_num)
{

  int i;
  for (i = 0; i < query_num; i++)
  {
    /* here we assume args are percent-encoded */
    dst_cstr = aws_sigv4_sprintf(dst_cstr, "%V=%V",
                                 &query_params[i].key, &query_params[i].value);
    if (i != query_num - 1)
    {
      *(dst_cstr++) = '&';
    }
  }
  return dst_cstr;
}

static void parse_query_params(aws_sigv4_str_t* query_str,
                               aws_sigv4_kv_t*  query_params,
                               size_t*          arr_len)
{
  if (aws_sigv4_empty_str(query_str)
      || query_params == NULL)
  {
    arr_len = 0;
    return;
  }
  size_t idx = 0;
  unsigned char* c_ptr = query_str->data;
  query_params[0].key.data = c_ptr;
  /* here we assume query string are well-formed */
  while (c_ptr != query_str->data + query_str->len)
  {
    if (*c_ptr == '=')
    {
      query_params[idx].key.len     = c_ptr - query_params[idx].key.data;
      query_params[idx].value.data  = ++c_ptr;
    }
    else if (*c_ptr == '&')
    {
      query_params[idx].value.len   = c_ptr - query_params[idx].value.data;
      query_params[++idx].key.data  = ++c_ptr;
    }
    else
    {
      c_ptr++;
    }
  }
  query_params[idx].value.len = c_ptr - query_params[idx].value.data;
  *arr_len = idx + 1;
}

void get_hexdigest(aws_sigv4_str_t* str_in, aws_sigv4_str_t* hex_out)
{
  static const unsigned char digits[] = "0123456789abcdef";
  unsigned char* c_ptr = hex_out->data;
  int i;
  for (i = 0; i < str_in->len; i++)
  {
    *(c_ptr++) = digits[(str_in->data[i] & 0xf0) >> 4];
    *(c_ptr++) = digits[str_in->data[i] & 0x0f];
  }
  hex_out->len = str_in->len * 2;
}

void get_hex_sha256(aws_sigv4_str_t* str_in, aws_sigv4_str_t* hex_sha256_out)
{
  unsigned char sha256_buf[SHA256_DIGEST_LENGTH];
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, str_in->data, str_in->len);
  SHA256_Final(sha256_buf, &ctx);

  aws_sigv4_str_t sha256_str = { .data = sha256_buf, .len = SHA256_DIGEST_LENGTH };
  get_hexdigest(&sha256_str, hex_sha256_out);
}

void get_signing_key(aws_sigv4_params_t* sigv4_params, aws_sigv4_str_t* signing_key)
{
  unsigned char key_buf[AWS_SIGV4_KEY_BUF_LEN]  = { 0 };
  unsigned char msg_buf[AWS_SIGV4_KEY_BUF_LEN]  = { 0 };
  aws_sigv4_str_t key = { .data = key_buf };
  aws_sigv4_str_t msg = { .data = msg_buf };
  /* kDate = HMAC("AWS4" + kSecret, Date) */
  key.len = aws_sigv4_sprintf(key_buf, "AWS4%V", &sigv4_params->secret_access_key) - key_buf;
  /* data in YYYYMMDD format */
  msg.len = aws_sigv4_snprintf(msg_buf, 8, "%V", &sigv4_params->x_amz_date) - msg_buf;
  /* get HMAC SHA256 */
  HMAC(EVP_sha256(), key.data, key.len, msg.data, msg.len,
       signing_key->data, (unsigned int *) &signing_key->len);
  /* kRegion = HMAC(kDate, Region) */
  key.len = aws_sigv4_sprintf(key_buf, "%V", signing_key) - key_buf;
  // key.len = signing_key->len;
  msg.len = aws_sigv4_sprintf(msg_buf, "%V", &sigv4_params->region) - msg_buf;
  HMAC(EVP_sha256(), key.data, key.len, msg.data, msg.len,
       signing_key->data, (unsigned int *) &signing_key->len);
  /* kService = HMAC(kRegion, Service) */
  key.len = aws_sigv4_sprintf(key_buf, "%V", signing_key) - key_buf;
  // key.len = signing_key->len;
  msg.len = aws_sigv4_sprintf(msg_buf, "%V", &sigv4_params->service) - msg_buf;
  HMAC(EVP_sha256(), key.data, key.len, msg.data, msg.len,
       signing_key->data, (unsigned int *) &signing_key->len);
  /* kSigning = HMAC(kService, "aws4_request") */
  key.len = aws_sigv4_sprintf(key_buf, "%V", signing_key) - key_buf;
  // key.len = signing_key->len;
  msg.len = aws_sigv4_sprintf(msg_buf, "aws4_request") - msg_buf;
  HMAC(EVP_sha256(), key.data, key.len, msg.data, msg.len,
       signing_key->data, (unsigned int *) &signing_key->len);
}

void get_credential_scope(aws_sigv4_params_t* sigv4_params,
                          aws_sigv4_str_t* credential_scope)
{
  unsigned char* str = credential_scope->data;
  /* get date in yyyymmdd format */
  str = aws_sigv4_snprintf(str, 8, "%V", &sigv4_params->x_amz_date);
  str = aws_sigv4_sprintf(str, "/%V/%V/aws4_request",
                          &sigv4_params->region, &sigv4_params->service);
  credential_scope->len = str - credential_scope->data;
}

void get_signed_headers(aws_sigv4_params_t* sigv4_params,
                        aws_sigv4_str_t* signed_headers)
{
  /* TODO: Need to support additional headers and header sorting */
  signed_headers->len = aws_sigv4_sprintf(signed_headers->data, "host;x-amz-date")
                        - signed_headers->data;
}

void get_canonical_headers(aws_sigv4_params_t* sigv4_params,
                           aws_sigv4_str_t* canonical_headers)
{
  /* TODO: Add logic to remove leading and trailing spaces for header values */
  canonical_headers->len = aws_sigv4_sprintf(canonical_headers->data,
                                             "host:%V\nx-amz-date:%V\n",
                                             &sigv4_params->host,
                                             &sigv4_params->x_amz_date)
                           - canonical_headers->data;
}

void get_canonical_request(aws_sigv4_params_t* sigv4_params,
                           aws_sigv4_str_t* canonical_request)
{
  unsigned char* str = canonical_request->data;
  /* TODO: Here we assume the URI and query string have already been encoded.
   *       Add encoding logic in future.
   */
  str =  aws_sigv4_sprintf(str, "%V\n%V\n",
                           &sigv4_params->method,
                           &sigv4_params->uri);

  /* query string can be empty */
  if (!aws_sigv4_empty_str(&sigv4_params->query_str))
  {
    aws_sigv4_kv_t query_params[AWS_SIGV4_MAX_NUM_QUERY_COMPONENTS];
    size_t query_num = 0;
    parse_query_params(&sigv4_params->query_str, query_params, &query_num);
    qsort(query_params, query_num, sizeof(aws_sigv4_kv_t),
          (aws_sigv4_compare_func_t) aws_sigv4_kv_cmp);
    str = construct_query_str(str, query_params, query_num);
  }
  *(str++) = '\n';

  aws_sigv4_str_t canonical_headers = { .data = str };
  get_canonical_headers(sigv4_params, &canonical_headers);
  str += canonical_headers.len;
  *(str++) = '\n';

  aws_sigv4_str_t signed_headers = { .data = str };
  get_signed_headers(sigv4_params, &signed_headers);
  str += signed_headers.len;
  *(str++) = '\n';

  aws_sigv4_str_t hex_sha256 = { .data = str };
  get_hex_sha256(&sigv4_params->payload, &hex_sha256);
  str += hex_sha256.len;

  canonical_request->len = str - canonical_request->data;
}

void get_string_to_sign(aws_sigv4_str_t* request_date,
                        aws_sigv4_str_t* credential_scope,
                        aws_sigv4_str_t* canonical_request,
                        aws_sigv4_str_t* string_to_sign)
{
  unsigned char* str = string_to_sign->data;
  str =  aws_sigv4_sprintf(str, "AWS4-HMAC-SHA256\n%V\n%V\n",
                           request_date, credential_scope);

  aws_sigv4_str_t hex_sha256 = { .data = str };
  get_hex_sha256(canonical_request, &hex_sha256);
  str += hex_sha256.len;

  string_to_sign->len = str - string_to_sign->data;
}

int aws_sigv4_sign(aws_sigv4_params_t* sigv4_params, aws_sigv4_header_t* auth_header)
{
  int rc = AWS_SIGV4_OK;
  if (auth_header == NULL
      || sigv4_params == NULL
      || aws_sigv4_empty_str(&sigv4_params->secret_access_key)
      || aws_sigv4_empty_str(&sigv4_params->access_key_id)
      || aws_sigv4_empty_str(&sigv4_params->method)
      || aws_sigv4_empty_str(&sigv4_params->uri)
      || aws_sigv4_empty_str(&sigv4_params->host)
      || aws_sigv4_empty_str(&sigv4_params->x_amz_date)
      || aws_sigv4_empty_str(&sigv4_params->region)
      || aws_sigv4_empty_str(&sigv4_params->service))
  {
    rc = AWS_SIGV4_INVALID_INPUT_ERROR;
    goto err;
  }

  /* TODO: Support custom memory allocator */
  auth_header->value.data = calloc(AWS_SIGV4_AUTH_HEADER_MAX_LEN, sizeof(unsigned char));
  if (auth_header->value.data == NULL)
  {
    rc = AWS_SIGV4_MEMORY_ALLOCATION_ERROR;
    goto err;
  }

  auth_header->key.data  = (unsigned char*) AWS_SIGV4_AUTH_HEADER_NAME;
  auth_header->key.len   = strlen(AWS_SIGV4_AUTH_HEADER_NAME);

  /* AWS4-HMAC-SHA256 Credential=AKIDEXAMPLE/<credential_scope> */
  unsigned char* str = auth_header->value.data;
  str =  aws_sigv4_sprintf(str, "AWS4-HMAC-SHA256 Credential=%V/",
                           &sigv4_params->access_key_id);

  aws_sigv4_str_t credential_scope = { .data = str };
  get_credential_scope(sigv4_params, &credential_scope);
  str += credential_scope.len;

  /* SignedHeaders=<signed_headers> */
  str = aws_sigv4_sprintf(str, ", SignedHeaders=", &sigv4_params->access_key_id);
  aws_sigv4_str_t signed_headers = { .data = str };
  get_signed_headers(sigv4_params, &signed_headers);
  str += signed_headers.len;

  /* Signature=<signature> */
  str = aws_sigv4_sprintf(str, ", Signature=", &sigv4_params->access_key_id);
  /* Task 1: Create a canonical request */
  unsigned char canonical_request_buf[AWS_SIGV4_CANONICAL_REQUEST_BUF_LEN]  = { 0 };
  aws_sigv4_str_t canonical_request = { .data = canonical_request_buf };
  get_canonical_request(sigv4_params, &canonical_request);
  /* Task 2: Create a string to sign */
  unsigned char string_to_sign_buf[AWS_SIGV4_STRING_TO_SIGN_BUF_LEN]  = { 0 };
  aws_sigv4_str_t string_to_sign = { .data = string_to_sign_buf };
  get_string_to_sign(&sigv4_params->x_amz_date, &credential_scope,
                     &canonical_request, &string_to_sign);
  /* Task 3: Calculate the signature */
  /* 3.1: Derive signing key */
  unsigned char signing_key_buf[AWS_SIGV4_KEY_BUF_LEN] = { 0 };
  aws_sigv4_str_t signing_key = { .data = signing_key_buf };
  get_signing_key(sigv4_params, &signing_key);
  /* 3.2: Calculate signature on the string to sign */
  unsigned char signed_msg_buf[HMAC_MAX_MD_CBLOCK] = { 0 };
  aws_sigv4_str_t signed_msg = { .data = signed_msg_buf };
  /* get HMAC SHA256 */
  HMAC(EVP_sha256(),
       signing_key.data, signing_key.len,
       string_to_sign.data, string_to_sign.len,
       signed_msg.data, (unsigned int*) &signed_msg.len);
  aws_sigv4_str_t signature = { .data = str };
  get_hexdigest(&signed_msg, &signature);
  str += signature.len;
  auth_header->value.len = str - auth_header->value.data;
  return rc;
err:
  /* deallocate memory in case of failure */
  if (auth_header && auth_header->value.data)
  {
    free(auth_header->value.data);
    auth_header->value.data = NULL;
  }
  return rc;
}

int aws_sigv4_sign_with_cstr(unsigned char*   secret_access_key,
                             unsigned char*   access_key_id,
                             unsigned char*   method,
                             unsigned char*   uri,
                             unsigned char*   query_str,
                             unsigned char*   host,
                             unsigned char*   x_amz_date,
                             unsigned char*   payload,
                             unsigned char*   service,
                             unsigned char*   region,
                             unsigned char**  auth_header_value)
{
  if (auth_header_value == NULL)
  {
    return AWS_SIGV4_INVALID_INPUT_ERROR;
  }
  aws_sigv4_params_t sigv4_params;
  sigv4_params.secret_access_key  = aws_sigv4_string(secret_access_key);
  sigv4_params.access_key_id      = aws_sigv4_string(access_key_id);
  sigv4_params.method             = aws_sigv4_string(method);
  sigv4_params.uri                = aws_sigv4_string(uri);
  sigv4_params.query_str          = aws_sigv4_string(query_str);
  sigv4_params.host               = aws_sigv4_string(host);
  sigv4_params.x_amz_date         = aws_sigv4_string(x_amz_date);
  sigv4_params.payload            = aws_sigv4_string(payload);
  sigv4_params.service            = aws_sigv4_string(service);
  sigv4_params.region             = aws_sigv4_string(region);

  aws_sigv4_header_t auth_header;
  int rc = aws_sigv4_sign(&sigv4_params, &auth_header);
  if (rc == AWS_SIGV4_OK)
  {
    *auth_header_value = auth_header.value.data;
  }
  return rc;
}
