/**************************************************************************************************
**
** Copyright (C) 2018 Anton Sholokhov
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software
** and associated documentation files (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge, publish, distribute,
** sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or
** substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
** BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
** DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
***************************************************************************************************/
#ifndef _CBOR_H_
#define _CBOR_H_

// -------------------------------------------------------------------------------------------------

/**
 * Encoder and decoder support enable
*/
#define CBOR_ENABLE_ENCODER_SUPPORT
#define CBOR_ENABLE_DECODER_SUPPORT

/**
 * Select integer type
 *
 * @note If all commented out, types will be int8_t and uint8_t.
*/
#define CBOR_INTTYPE_64 // int64_t and uint64_t
//#define CBOR_INTTYPE_32 // int32_t and uint32_t
//#define CBOR_INTTYPE_16 // int16_t and uint16_t

/**
 * Enable float type
*/
#define CBOR_ENABLE_FLOAT32_SUPPORT
#define CBOR_ENABLE_FLOAT64_SUPPORT

/**
 * Enable UTF-8 support
*/
#define CBOR_ENABLE_UTF8_SUPPORT

// -------------------------------------------------------------------------------------------------

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CBOR_ENABLE_FLOAT32_SUPPORT
  #define CBOR_INTTYPE_32
#endif

#ifdef CBOR_ENABLE_FLOAT64_SUPPORT
  #define CBOR_INTTYPE_64
#endif

#ifdef CBOR_INTTYPE_64

typedef int64_t  cbor_int;
typedef uint64_t cbor_uint;

#define CBOR_UINT_MAX 0xFFFFFFFFFFFFFFFFULL

#elif defined(CBOR_INTTYPE_32)

typedef int32_t  cbor_int;
typedef uint32_t cbor_uint;

#define CBOR_UINT_MAX 0xFFFFFFFFUL

#elif  defined(CBOR_INTTYPE_16)

typedef int16_t  cbor_int;
typedef uint16_t cbor_uint;

#define CBOR_UINT_MAX 0xFFFF

#else

typedef int8_t   cbor_int;
typedef uint8_t  cbor_uint;

#define CBOR_UINT_MAX 0xFF

#endif

typedef enum
{
  cbor_ok,
  cbor_eos,  // end of stream
  cbor_efmt, // format error
  cbor_eio   // I/O error
} cbor_status;

// Registered CBOR tags from: https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml
typedef enum
{
  cbor_tag_std_datetime   = 0,       // standard date/time string
  cbor_tag_epoch_datetime = 1,       // epoch-based date/time
  cbor_tag_pos_bignum     = 2,       // positive bignum
  cbor_tag_neg_bignum     = 3,       // negative bignum
  cbor_tag_dec_fract      = 4,       // decimal fraction
  cbor_tag_bigfloat       = 5,       // bigfloat
  cbor_cose_encrypt0      = 16,      // COSE Single Recipient Encrypted Data Object
  cbor_cose_mac0          = 17,      // COSE Mac w/o Recipients Object
  cbor_cose_sign1         = 18,      // COSE Single Signer Data Object
  cbor_tag_exp_base64url  = 21,      // expected conversion to base64url encoding
  cbor_tag_exp_base64     = 22,      // expected conversion to base64 encoding;
  cbor_tag_exp_base16     = 23,      // expected conversion to base16 encoding
  cbor_tag_data_item      = 24,      // encoded CBOR data item
  cbor_tag_string_ref     = 25,      // reference the nth previously seen string
  cbor_tag_perl_object    = 26,      // serialised Perl object with classname and constructor
                                     // arguments
  cbor_tag_generic_object = 27,      // serialised language-independent object with type name and
                                     // constructor arguments
  cbor_tag_shared_value   = 28,      // mark value as (potentially) shared
  cbor_tag_value_ref      = 29,      // reference nth marked value
  cbor_tag_rational_num   = 30,      // rational number
  cbor_tag_uri            = 32,      // URI
  cbor_tag_base64url      = 33,      // base64url
  cbor_tag_base64         = 34,      // base64
  cbor_tag_regexpr        = 35,      // regular expression
  cbor_tag_mime           = 36,      // MIME message
  cbor_tag_binary_uuid    = 37,      // binary UUID
  cbor_tag_langtag        = 38,      // language-tagged string
  cbor_tag_identifier     = 39,      // identifier
  cbor_tag_cwt            = 61,      // CBOR Web Token (CWT)
  cbor_tag_cose_encrypt   = 96,      // COSE Encrypted Data Object
  cbor_tag_cose_mac       = 97,      // COSE MACed Data Object
  cbor_tag_cose_sign      = 98,      // COSE Signed Data Object
  cbor_tag_geo_coords     = 103,     // geographic coordinates
  cbor_tag_iot_data_point = 120,     // Internet of Things data point
  cbor_tag_have_strref    = 256,     // mark value as having string references
  cbor_tag_binary_mime    = 257,     // binary MIME message
  cbor_tag_finite_set     = 258,     // mathematical finite set
  cbor_tag_netaddr        = 260,     // network address (IPv4 or IPv6 or MAC address)
  cbor_tag_netaddr_prefix = 261,     // network address prefix (IPv4 or IPv6 Address + Mask Length)
  cbor_tag_json           = 262,     // embedded JSON object
  cbor_tag_hexstr         = 263,     // hexadecimal string
  cbor_tag_decfract       = 264,     // decimal fraction with arbitrary exponent
  cbor_tag_bigfloat_exp   = 265,     // bigfloat with arbitrary exponent
  cbor_tag_ext_time       = 1001,    // extended time
  cbor_tag_duration       = 1002,    // duration
  cbor_tag_period         = 1003,    // period
  cbor_tag_indirection    = 22098,   // hint that indicates an additional level of indirection
  cbor_tag_magic          = 55799,   // self-describe CBOR
  cbor_tag_rains          = 15309736 // RAINS Message
} cbor_tag;

typedef enum
{
  cbor_false = 20,
  cbor_true  = 21,
  cbor_null  = 22,
  cbor_undef = 23
} cbor_simple;

typedef enum
{
  cbor_tuint     = 0,                 // positive integer
  cbor_tfloat32,                      // float
  cbor_tfloat64,                      // double
  cbor_tint      = 1 << 5,            // negative integer
  cbor_tbytestr  = 2 << 5,            // raw binary data
  cbor_ttextstr  = 3 << 5,            // utf-8 text string
  cbor_tarray    = 4 << 5,            // array
  cbor_tmap      = 5 << 5,            // key-value map
  cbor_ttag      = 6 << 5,            // type tag
  cbor_tsimple   = 7 << 5,            // true, false, null, undef
  cbor_tvbytestr = cbor_tbytestr | 1, // raw binary data (variable length)
  cbor_tvtextstr = cbor_ttextstr | 1, // utf-8 text string (variable length)
  cbor_tvarray   = cbor_tarray   | 1, // array (variable length)
  cbor_tvmap     = cbor_tmap     | 1, // key-value map (variable length)
  cbor_tbreak,                        // break code for variable length objects
  cbor_tinvalid                       // invalid token
} cbor_token;

#ifdef CBOR_ENABLE_ENCODER_SUPPORT

#define CBOR_ENCODER_MIN_BUFFER_SIZE 9

typedef struct cbenc_ctx
{
  // public:
  cbor_status (*write)(const void *data, cbor_uint sz, void *usrdata); // data write callback
  uint8_t *buf; // pointer to data buffer
  cbor_uint bufsz; // data buffer size (must not be less then: CBOR_ENCODER_MIN_BUFFER_SIZE)
  void *usrdata; // user data pointer

  // private:
  uint8_t *end;
  uint8_t *mem;
} cbenc_ctx_t;

/**
 * Initializer for encoder context
 *
 * @param write   - data write callback
 * @param buf     - ptr to buffer
 * @param bufsz   - buffer size (min 9 bytes!)
 * @param usrdata - ptr to user data
*/
#define CBOR_ENCODER_CTX_INITIALIZER(write, buf, bufsz, usrdata) {write, buf, bufsz, usrdata, 0, 0}

/**
 * Start encoding
 *
 * @param  ctx - encoder context
 * @return status code
*/
cbor_status cbenc_begin(cbenc_ctx_t *ctx);

/**
 * Finish encoding
 *
 * @param  ctx - encoder context
 * @return status code
*/
cbor_status cbenc_end(cbenc_ctx_t *ctx);

/**
 * Encode signed and unsigned int value
 *
 * @param  ctx - encoder context
 * @param  val - value to encode
 * @return status code
 */
cbor_status cbenc_uint(cbenc_ctx_t *ctx, cbor_uint val);
cbor_status cbenc_int(cbenc_ctx_t *ctx, cbor_int val);

#ifdef CBOR_ENABLE_FLOAT32_SUPPORT

/**
 * Encode half float and float value
 *
 * @param  ctx - encoder context
 * @param  val - value to encode
 * @return status code
 */
cbor_status cbenc_float16(cbenc_ctx_t *ctx, float val);
cbor_status cbenc_float32(cbenc_ctx_t *ctx, float val);

#endif

#ifdef CBOR_ENABLE_FLOAT64_SUPPORT

/**
 * Encode double value
 *
 * @param  ctx - encoder context
 * @param  val - value to encode
 * @return status code
 */
cbor_status cbenc_float64(cbenc_ctx_t *ctx, double val);

#endif

/**
 * Encode CBOR simple type value
 *
 * @param  ctx - encoder context
 * @param  val - value to encode
 * @return status code
 */
cbor_status cbenc_simple(cbenc_ctx_t *ctx, cbor_simple val);

/**
 * Begin encode byte string with variable length
 *
 * @param  ctx - encoder context
 * @return status code
 *
 * @brief  After cbenc_bytestr_begin, call cbenc_bytestr to actually write data as many times as
 *         necessary. At the end, call cbenc_break to complete.
*/
cbor_status cbenc_bytestr_begin(cbenc_ctx_t *ctx);

/**
 * Encode byte string with fixed length
 *
 * @param  ctx  - encoder context
 * @param  data - ptr to to data for writing
 * @param  sz   - data size
 * @return status code
 *
 * @brief  The data can contain any binary format.
*/
cbor_status cbenc_bytestr(cbenc_ctx_t *ctx, const void *data, cbor_uint sz);

/**
 * Begin encode text string with variable length
 *
 * @param  ctx - encoder context
 * @return status code
 *
 * @brief  After cbenc_textstr_begin, call cbenc_textstr to actually write data as many times as
 *         necessary. At the end, call cbenc_break to complete.
*/
cbor_status cbenc_textstr_begin(cbenc_ctx_t *ctx);

/**
 * Encode text string with fixed length
 *
 * @param  ctx  - encoder context
 * @param  data - ptr to to data for writing
 * @param  sz   - data size
 * @return status code
 *
 * @brief  The data must be a valid utf-8 string.
*/
cbor_status cbenc_textstr(cbenc_ctx_t *ctx, const char *data, cbor_uint sz);

/**
 * Encode null terminated c-string
 *
 * @param  ctx  - encoder context
 * @param  data - ptr to to data for writing
 * @return status code
 *
 * @brief  If string length less than CBOR_UINT_MAX, will be used fixed size, otherwise a variable
 *         length.
 *
 *         The data must be a valid utf-8 string.
*/
cbor_status cbenc_cstring(cbenc_ctx_t *ctx, const char *data);

/**
 * Begin encode byte string with fixed length
 *
 * @param  ctx - encoder context
 * @param  sz  - data size
 * @return status code
 *
 * @brief  After cbenc_bytestr_begin_sz, call cbenc_swrite until all data is written. The total
 *         amount of data write must be equal to the size specified in sz parameter.
 *
 *         The data can contain any binary format.
*/
cbor_status cbenc_bytestr_begin_sz(cbenc_ctx_t *ctx, cbor_uint sz);

/**
 * Begin encode text string with fixed length
 *
 * @param  ctx - encoder context
 * @param  sz  - data size
 * @return status code
 *
 * @brief  After cbenc_textstr_begin_sz, call cbenc_swrite until all data is written. The total
 *         amount of data write must be equal to the size specified in sz parameter.
 *
 *         The data must be a valid utf-8 string.
*/
cbor_status cbenc_textstr_begin_sz(cbenc_ctx_t *ctx, cbor_uint sz);

/**
 * Write byte and text string's data
 *
 * @param  ctx  - encoder context
 * @param  data - ptr to to data for writing
 * @param  sz   - data size
 * @return status code
 *
 * @brief  This function must be used only with cbenc_bytestr_begin_sz or cbenc_textstr_begin_sz.
*/
cbor_status cbenc_swrite(cbenc_ctx_t *ctx, const void *data, cbor_uint sz);

/**
 * Begin encode array with variable length
 *
 * @param  ctx - encoder context
 * @return status code
 *
 * @brief  After cbenc_array_begin, call other encode functions to actually write data as many
 *         times as necessary. At the end, call cbenc_break to complete.
*/
cbor_status cbenc_array_begin(cbenc_ctx_t *ctx);

/**
 * Begin encode array with fixed length
 *
 * @param  ctx - encoder context
 * @param  sz  - number of elements in the array
 * @return status code
 *
 * @brief  After cbenc_array, call other encode functions to actually write data.
*/
cbor_status cbenc_array(cbenc_ctx_t *ctx, cbor_uint sz);

/**
 * Begin encode map (key-value container) with variable length
 *
 * @param  ctx - encoder context
 * @return status code
 *
 * @brief  After cbenc_map_begin, call other encode functions to actually write data as many
 *         times as necessary. At the end, call cbenc_break to complete.
*/
cbor_status cbenc_map_begin(cbenc_ctx_t *ctx);

/**
 * Begin encode map (key-value container) with fixed length
 *
 * @param  ctx - encoder context
 * @param  sz  - number of key-value pairs in the map
 * @return status code
 *
 * @brief  After cbenc_map, call other encode functions to actually write data.
*/
cbor_status cbenc_map(cbenc_ctx_t *ctx, cbor_uint sz);

/**
 * Encode break code for containers with variable length.
 *
 * @param  ctx - encoder context
 * @return status code
*/
cbor_status cbenc_break(cbenc_ctx_t *ctx);

/**
 * Encode CBOR tag
 *
 * @param  ctx - encoder context
 * @param  tag - CBOR tag value
 * @return status code
*/
cbor_status cbenc_tag(cbenc_ctx_t *ctx, cbor_uint tag);

#endif // CBOR_ENABLE_ENCODER_SUPPORT

#ifdef CBOR_ENABLE_DECODER_SUPPORT

typedef struct cbdec_ctx
{
  // public:
  cbor_status (*read)(void *data, cbor_uint sz, void *usrdata); // data read callback
  void *usrdata; // user data pointer
  cbor_token token; // current token (read only!)

  union {
    cbor_int    s;
    cbor_uint   u;

#ifdef CBOR_ENABLE_FLOAT32_SUPPORT
    float       f32;
#endif

#ifdef CBOR_ENABLE_FLOAT64_SUPPORT
    double      f64;
#endif

    cbor_simple st;
  } value; // current object value or length (read only!)

  // private:
  uint8_t buf[8];
} cbdec_ctx_t;

/**
 * Initializer for decoder context
 *
 * @param read    - data read callback
 * @param usrdata - ptr to user data
*/
#define CBOR_DECODER_CTX_INITIALIZER(read, usrdata) \
  {read, usrdata, cbor_tinvalid, 0, {0, 0, 0, 0, 0, 0, 0, 0}}

/**
 * Perform one decoder step
 *
 * @param  ctx - decoder context
 * @return status code
*/
cbor_status cbdec_step(cbdec_ctx_t *ctx);

/**
 * Read data for byte or text string
 *
 * @param  ctx - decoder context
 * @param  sz  - size to read
 * @return status code
*/
cbor_status cbdec_sread(cbdec_ctx_t *ctx, void *data, cbor_uint sz);

#endif // CBOR_ENABLE_DECODER_SUPPORT

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _CBOR_H_
