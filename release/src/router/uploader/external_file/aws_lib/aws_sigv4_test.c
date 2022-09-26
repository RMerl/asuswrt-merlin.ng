#include <stdlib.h>
#include <check.h>
#include "aws_sigv4.h"

START_TEST (AwsSigv4Test_HEX)
{
  /*
   * test data is from https://tools.ietf.org/html/rfc4231
   */
  const unsigned char* expected_output_str1 = "4a656665";
  const unsigned char* expected_output_str2 = "7768617420646f2079612077616e7420666f72206e6f7468696e673f";
  unsigned char hex_buff[65] = { 0 };

  aws_sigv4_str_t str_in_str1 = aws_sigv4_string("Jefe");
  aws_sigv4_str_t hex_out     = { .data = hex_buff };
  get_hexdigest(&str_in_str1, &hex_out);
  ck_assert_pstr_eq(hex_buff, expected_output_str1);
  ck_assert_mem_eq(hex_out.data, expected_output_str1, hex_out.len);

  memset(hex_buff, 0, 65);
  aws_sigv4_str_t str_in_str2  = aws_sigv4_string("what do ya want for nothing?");
  get_hexdigest(&str_in_str2, &hex_out);
  ck_assert_pstr_eq(hex_buff, expected_output_str2);
  ck_assert_mem_eq(hex_out.data, expected_output_str2, hex_out.len);
}
END_TEST

START_TEST (AwsSigv4Test_HexSHA256)
{
  /*
   * test data is from https://docs.aws.amazon.com/general/latest/gr/sigv4-create-canonical-request.html
   */
  const unsigned char* empty_str_sha256 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
  unsigned char hex_sha256[65] = { 0 };
  aws_sigv4_str_t str_in          = aws_sigv4_string(NULL);
  aws_sigv4_str_t hex_sha256_out  = { .data = hex_sha256 };
  get_hex_sha256(&str_in, &hex_sha256_out);
  ck_assert_pstr_eq(hex_sha256, empty_str_sha256);
  ck_assert_mem_eq(hex_sha256_out.data, empty_str_sha256, hex_sha256_out.len);
}
END_TEST

START_TEST (AwsSigv4Test_SigningKey)
{
  /*
   * test data is from https://docs.aws.amazon.com/general/latest/gr/sigv4-calculate-signature.html
   */
  aws_sigv4_params_t sigv4_params = { .secret_access_key  = aws_sigv4_string("wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY"),
                                      .x_amz_date = aws_sigv4_string("20150830T050102Z"),
                                      .region     = aws_sigv4_string("us-east-1"),
                                      .service    = aws_sigv4_string("iam") };
  unsigned char key_buff[1024] = { 0 };
  unsigned char msg_buff[1024] = { 0 };
  unsigned char signing_key_buff[32]      = { 0 };
  unsigned char signing_key_hash_buff[65] = { 0 };

  aws_sigv4_str_t signing_key       = { .data = signing_key_buff };
  aws_sigv4_str_t signing_key_hash  = { .data = signing_key_hash_buff };
  get_signing_key(&sigv4_params, &signing_key);
  get_hexdigest(&signing_key, &signing_key_hash);
  const unsigned char* expected_signing_key_hash = "c4afb1cc5771d871763a393e44b703571b55cc28424d1a5e86da6ed3c154a4b9";
  ck_assert_pstr_eq(signing_key_hash_buff, expected_signing_key_hash);
  ck_assert_mem_eq(signing_key_hash.data, expected_signing_key_hash, signing_key_hash.len);
}
END_TEST

START_TEST (AwsSigv4Test_CredentialScope)
{
  unsigned char credential_scope_data[1024] = { 0 };
  aws_sigv4_str_t credential_scope  = { .data = credential_scope_data };
  aws_sigv4_params_t sigv4_params   = { .x_amz_date = aws_sigv4_string("20180717T074800Z"),
                                        .region     = aws_sigv4_string("us-east-1"),
                                        .service    = aws_sigv4_string("s3") };
  get_credential_scope(&sigv4_params, &credential_scope);
  const unsigned char* expected_credential_scope = "20180717/us-east-1/s3/aws4_request";
  int expected_len = strlen(expected_credential_scope);
  ck_assert_mem_eq(credential_scope.data, expected_credential_scope, expected_len);
  ck_assert_int_eq(credential_scope.len, expected_len);
}
END_TEST

START_TEST (AwsSigv4Test_SignedHeaders)
{
  unsigned char signed_headers_data[128];
  aws_sigv4_str_t signed_headers  = { .data = signed_headers_data };
  aws_sigv4_params_t sigv4_params = { .host       = aws_sigv4_string("abc.com"),
                                      .x_amz_date = aws_sigv4_string("20180717T074800Z") };

  get_signed_headers(&sigv4_params, &signed_headers);
  const unsigned char* expected_signed_headers = "host;x-amz-date";
  int expected_len = strlen(expected_signed_headers);
  ck_assert_mem_eq(signed_headers.data, expected_signed_headers, expected_len);
  ck_assert_int_eq(signed_headers.len, expected_len);
}
END_TEST

START_TEST (AwsSigv4Test_CanonicalHeaders)
{
  unsigned char canonical_headers_data[256];
  aws_sigv4_str_t canonical_headers = { .data = canonical_headers_data };
  aws_sigv4_params_t sigv4_params   = { .host       = aws_sigv4_string("abc.com"),
                                        .x_amz_date = aws_sigv4_string("20180717T074800Z") };

  get_canonical_headers(&sigv4_params, &canonical_headers);
  const unsigned char* expected_canonical_headers = "host:abc.com\nx-amz-date:20180717T074800Z\n";
  int expected_len = strlen(expected_canonical_headers);
  ck_assert_mem_eq(canonical_headers.data, expected_canonical_headers, expected_len);
  ck_assert_int_eq(canonical_headers.len, expected_len);
}
END_TEST

START_TEST (AwsSigv4Test_CanonicalRequest)
{
  /* test data is from https://docs.aws.amazon.com/general/latest/gr/signature-v4-test-suite.html */
  unsigned char canonical_request_data[1024] = { 0 };
  aws_sigv4_str_t canonical_request = { .data = canonical_request_data };
  aws_sigv4_params_t sigv4_params   = { .method     = aws_sigv4_string("GET"),
                                        .host       = aws_sigv4_string("example.amazonaws.com"),
                                        .x_amz_date = aws_sigv4_string("20150830T123600Z"),
                                        .uri        = aws_sigv4_string("/"),
                                        .query_str  = aws_sigv4_string("Param2=value2&Param1=value1"),
                                        .payload    = aws_sigv4_string(NULL) };

  get_canonical_request(&sigv4_params, &canonical_request);
  const unsigned char* expected_canonical_request = \
"GET\n\
/\n\
Param1=value1&Param2=value2\n\
host:example.amazonaws.com\n\
x-amz-date:20150830T123600Z\n\
\n\
host;x-amz-date\n\
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
  int expected_len = strlen(expected_canonical_request);
  ck_assert_int_eq(canonical_request.len, expected_len);
  ck_assert_pstr_eq(canonical_request.data, expected_canonical_request);
  ck_assert_mem_eq(canonical_request.data, expected_canonical_request, expected_len);
}
END_TEST

START_TEST (AwsSigv4Test_StringToSign)
{
  /*
   * test data is from https://docs.aws.amazon.com/general/latest/gr/signature-v4-test-suite.html
   */
  unsigned char string_to_sign_data[1024] = { 0 };
  const unsigned char* request_date_data       = "20150830T123600Z";
  const unsigned char* credential_scope_data   = "20150830/us-east-1/service/aws4_request";
  const unsigned char* canonical_request_data  = \
"GET\n\
/\n\
Param1=value1&Param2=value2\n\
host:example.amazonaws.com\n\
x-amz-date:20150830T123600Z\n\
\n\
host;x-amz-date\n\
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
  aws_sigv4_str_t request_date      = aws_sigv4_string(request_date_data);
  aws_sigv4_str_t credential_scope  = aws_sigv4_string(credential_scope_data);
  aws_sigv4_str_t canonical_request = aws_sigv4_string(canonical_request_data);
  aws_sigv4_str_t string_to_sign    = { .data = string_to_sign_data };
  get_string_to_sign(&request_date, &credential_scope, &canonical_request, &string_to_sign);
  const unsigned char* expected_string_to_sign = \
"AWS4-HMAC-SHA256\n\
20150830T123600Z\n\
20150830/us-east-1/service/aws4_request\n\
816cd5b414d056048ba4f7c5386d6e0533120fb1fcfa93762cf0fc39e2cf19e0";
  int expected_len = strlen(expected_string_to_sign);
  ck_assert_int_eq(string_to_sign.len, expected_len);
  ck_assert_pstr_eq(string_to_sign.data, expected_string_to_sign);
  ck_assert_mem_eq(string_to_sign.data, expected_string_to_sign, expected_len);
}
END_TEST

START_TEST (AwsSigv4Test_AwsSigv4Sign)
{
  /* test data is from https://docs.aws.amazon.com/general/latest/gr/signature-v4-test-suite.html */
  aws_sigv4_params_t sigv4_params   = { .secret_access_key  = aws_sigv4_string("wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY"),
                                        .access_key_id      = aws_sigv4_string("AKIDEXAMPLE"),
                                        .method             = aws_sigv4_string("GET"),
                                        .host               = aws_sigv4_string("example.amazonaws.com"),
                                        .x_amz_date         = aws_sigv4_string("20150830T123600Z"),
                                        .uri                = aws_sigv4_string("/"),
                                        .query_str          = aws_sigv4_string("Param2=value2&Param1=value1"),
                                        .payload            = aws_sigv4_string(NULL),
                                        .service            = aws_sigv4_string("service"),
                                        .region             = aws_sigv4_string("us-east-1") };
  aws_sigv4_header_t auth_header = { .key = aws_sigv4_string(NULL), .value = aws_sigv4_string(NULL) };
  int rc = aws_sigv4_sign(&sigv4_params, &auth_header);
  const unsigned char* expected_auth_header_name  = "Authorization";
  const unsigned char* expected_auth_header_value = \
"AWS4-HMAC-SHA256 \
Credential=AKIDEXAMPLE/20150830/us-east-1/service/aws4_request, \
SignedHeaders=host;x-amz-date, \
Signature=b97d918cfa904a5beff61c982a1b6f458b799221646efd99d3219ec94cdf2500";
  int expected_len = strlen(expected_auth_header_value);
  ck_assert_int_eq(rc, AWS_SIGV4_OK);

  ck_assert_pstr_eq(auth_header.key.data, expected_auth_header_name);
  ck_assert_int_eq(auth_header.key.len, strlen(expected_auth_header_name));
  ck_assert_mem_eq(auth_header.key.data, expected_auth_header_name, strlen(expected_auth_header_name));

  ck_assert_pstr_eq(auth_header.value.data, expected_auth_header_value);
  ck_assert_int_eq(auth_header.value.len, expected_len);
  ck_assert_mem_eq(auth_header.value.data, expected_auth_header_value, expected_len);

  /* test with c str input */
  unsigned char* auth_header_value = NULL;
  rc = aws_sigv4_sign_with_cstr("wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY",
                                "AKIDEXAMPLE",
                                "GET",
                                "/",
                                "Param2=value2&Param1=value1",
                                "example.amazonaws.com",
                                "20150830T123600Z",
                                "",
                                "service",
                                "us-east-1",
                                &auth_header_value);
  ck_assert_int_eq(rc, AWS_SIGV4_OK);
  ck_assert_pstr_eq(auth_header_value, expected_auth_header_value);
}
END_TEST

Suite * aws_sigv4_test_suite(void)
{
  Suite *s;
  s = suite_create("AwsSigv4Test");

  TCase *tc_hex               = tcase_create("AwsSigv4Test_HEX");
  TCase *tc_hex_sha256        = tcase_create("AwsSigv4Test_HexSHA256");
  TCase *tc_credential_scope  = tcase_create("AwsSigv4Test_CredentialScope");
  TCase *tc_signed_headers    = tcase_create("AwsSigv4Test_SignedHeaders");
  TCase *tc_canonical_headers = tcase_create("AwsSigv4Test_CanonicalHeaders");
  TCase *tc_canonical_request = tcase_create("AwsSigv4Test_CanonicalRequest");
  TCase *tc_string_to_sign    = tcase_create("AwsSigv4Test_StringToSign");
  TCase *tc_signing_key       = tcase_create("AwsSigv4Test_SigningKey");
  TCase *tc_aws_sigv4_sign    = tcase_create("AwsSigv4Test_AwsSigv4Sign");
  tcase_add_test(tc_hex_sha256, AwsSigv4Test_HexSHA256);
  tcase_add_test(tc_hex, AwsSigv4Test_HEX);
  tcase_add_test(tc_credential_scope, AwsSigv4Test_CredentialScope);
  tcase_add_test(tc_signed_headers, AwsSigv4Test_SignedHeaders);
  tcase_add_test(tc_canonical_headers, AwsSigv4Test_CanonicalHeaders);
  tcase_add_test(tc_canonical_request, AwsSigv4Test_CanonicalRequest);
  tcase_add_test(tc_string_to_sign, AwsSigv4Test_StringToSign);
  tcase_add_test(tc_signing_key, AwsSigv4Test_SigningKey);
  tcase_add_test(tc_aws_sigv4_sign, AwsSigv4Test_AwsSigv4Sign);
  suite_add_tcase(s, tc_hex);
  suite_add_tcase(s, tc_hex_sha256);
  suite_add_tcase(s, tc_credential_scope);
  suite_add_tcase(s, tc_signed_headers);
  suite_add_tcase(s, tc_canonical_headers);
  suite_add_tcase(s, tc_canonical_request);
  suite_add_tcase(s, tc_string_to_sign);
  suite_add_tcase(s, tc_signing_key);
  suite_add_tcase(s, tc_aws_sigv4_sign);
  return s;
}

int main(int argc, char **argv) {
  int number_failed;
  SRunner *sr;

  sr = srunner_create(aws_sigv4_test_suite());

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
