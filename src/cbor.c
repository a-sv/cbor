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
#include <string.h>
#include "cbor.h"

#ifdef CBOR_INTTYPE_64
  #define CBOR_INTTYPE_32
#endif

#ifdef CBOR_INTTYPE_32
  #define CBOR_INTTYPE_16
#endif

#ifdef _MSC_VER
  // MSVC, which implies Windows, which implies little-endian and sizeof(long) == 4
  #include <stdlib.h>
  #define cbor_bswap16 _byteswap_ushort
  #define cbor_bswap32 _byteswap_ulong
  #define cbor_bswap64 _byteswap_uint64

  #define inline __inline
#else

  #if   (defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
         __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || \
        (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && __BYTE_ORDER == __BIG_ENDIAN) || \
        (defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN) || \
        (defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)) || \
        (defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)) || \
        defined(__ARMEB__) || defined(__MIPSEB__) || defined(__s390__) || defined(__sparc__)

  #define CBOR_BIG_ENDIAN

  #elif (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
         __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || \
        (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN) || \
        (defined(BYTE_ORDER) && defined(LITTLE_ENDIAN) && BYTE_ORDER == LITTLE_ENDIAN) || \
        defined(_LITTLE_ENDIAN) || defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) || \
        defined(__MIPSEL__) || defined(__i386) || defined(__i386__) || defined(__x86_64) || \
        defined(__x86_64__) || defined(__amd64)

  #define CBOR_LITTLE_ENDIAN

  #else
    #error "unable to determine byte order!"
  #endif

  #if defined(__GNUC__)
    #ifdef CBOR_LITTLE_ENDIAN
      #define cbor_bswap16 __builtin_bswap16
      #define cbor_bswap32 __builtin_bswap32
      #define cbor_bswap64 __builtin_bswap64
    #else
      #define cbor_bswap16(x) (x)
      #define cbor_bswap32(x) (x)
      #define cbor_bswap64(x) (x)
    #endif
  #endif

#endif // _MSC_VER

enum sub_type
{
  st_size8  = 24,
  st_size16 = 25,
  st_size32 = 26,
  st_size64 = 27,
  st_varbrk = 31
};

#define return_if_fail(x) if((cs = (x)) != cbor_ok) { return cs; }

#ifdef CBOR_ENABLE_ENCODER_SUPPORT

static inline cbor_uint encbuf_datalen(cbenc_ctx_t *ctx) { return ctx->end - ctx->buf; }
static inline cbor_uint encbuf_avail(cbenc_ctx_t *ctx) { return ctx->bufsz - encbuf_datalen(ctx); }

static inline cbor_status encbuf_grow(cbenc_ctx_t *ctx, cbor_uint size)
{
  cbor_status cs = cbor_ok;

  if(encbuf_avail(ctx) < size) {
    cs = ctx->write(ctx->buf, encbuf_datalen(ctx), ctx->usrdata);
    ctx->end = ctx->buf;
  }

  ctx->mem  = ctx->end;
  ctx->end += size;

  return cs;
}

static inline cbor_status cbenc_header(cbenc_ctx_t *ctx, uint8_t type, cbor_uint grow)
{
  cbor_status cs = cbor_ok;

  return_if_fail(encbuf_grow(ctx, grow + 1));
  ctx->mem[0] = type;

  return cs;
}

static inline cbor_status cbenc_bytes(cbenc_ctx_t *ctx, uint8_t type, const void *data, cbor_uint sz)
{
  cbor_status cs = cbor_ok;

  return_if_fail(cbenc_uint(ctx, sz));
  ctx->mem[0] |= type;

  if(sz == 0) { return cs; }

  return cbenc_swrite(ctx, data, sz);
}

cbor_status cbenc_begin(cbenc_ctx_t *ctx) { ctx->end = ctx->buf; return cbor_ok; }

cbor_status cbenc_end(cbenc_ctx_t *ctx)
{
  cbor_status cs = cbor_ok;
  cbor_uint len = encbuf_datalen(ctx);

  if(len > 0) {
    cs = ctx->write(ctx->buf, len, ctx->usrdata);
    ctx->end = ctx->buf;
  }

  return cs;
}

cbor_status cbenc_uint(cbenc_ctx_t *ctx, cbor_uint val)
{
  cbor_status cs = cbor_ok;

  if(val < 24) {
    return_if_fail(cbenc_header(ctx, val, 0));
  }
  else

#ifdef CBOR_INTTYPE_16
  if(val <= 0xFF)
#endif
  {
    return_if_fail(cbenc_header(ctx, st_size8, 1));
    ctx->mem[1] = val;
  }

#ifdef CBOR_INTTYPE_16
  else
  #ifdef CBOR_INTTYPE_32
  if(val <= 0xFFFF)
  #endif
  {
    return_if_fail(cbenc_header(ctx, st_size16, 2));
    *(uint16_t*)(&ctx->mem[1]) = cbor_bswap16(val);
  }
#endif

#ifdef CBOR_INTTYPE_32
  else
  #ifdef CBOR_INTTYPE_64
  if(val <= 0xFFFFFFFFUL)
  #endif
  {
    return_if_fail(cbenc_header(ctx, st_size32, 4));
    *(uint32_t*)(&ctx->mem[1]) = cbor_bswap32(val);
  }
#endif

#ifdef CBOR_INTTYPE_64
  else {
    return_if_fail(cbenc_header(ctx, st_size64, 8));
    *(uint64_t*)(&ctx->mem[1]) = cbor_bswap64(val);
  }
#endif

  return cs;
}

cbor_status cbenc_int(cbenc_ctx_t *ctx, cbor_int val)
{
  cbor_status cs = cbor_ok;

  if(val >= 0) { return cbenc_uint(ctx, val); }

  return_if_fail(cbenc_uint(ctx, (-val) - 1));
  ctx->mem[0] |= cbor_tint;

  return cs;
}

#ifdef CBOR_ENABLE_FLOAT32_SUPPORT

/**
 * Convert float32 to float16
 *
 * @param h - float32 bits
 * @return float16 bits
 *
 * @note
 *   source from: http://gamedev.stackexchange.com/questions/17326/conversion-of-a-number-from-
 *                single-precision-floating-point-representation-to-a
*/
static uint16_t encode_float16(uint32_t h)
{
  uint16_t bits = (h >> 16) & 0x8000; /* Get the sign */
  uint16_t m = (h >> 12) & 0x7FF; /* Keep one extra bit for rounding */
  unsigned int e = (h >> 23) & 0xFF; /* Using int is faster here */

  /* If zero, or denormal, or exponent underflows too much for a denormal
   * half, return signed zero. */
  if(e >= 103) {
    /* If NaN, return NaN. If Inf or exponent overflow, return Inf. */
    if(e > 142) {
      bits |= 0x7C00u;
      /* If exponent was 0xff and one mantissa bit was set, it means NaN,
       * not Inf, so make sure we set one mantissa bit too. */
      bits |= e == 255 && (h & 0x7FFFFFu);
    }
    else
    /* If exponent underflows but not too much, return a denormal */
    if(e < 113) {
      m |= 0x800u;
      /* Extra rounding may overflow and set mantissa to 0 and exponent
       * to 1, which is OK. */
      bits |= (m >> (114 - e)) + ((m >> (113 - e)) & 1);
    }
    else {
      bits |= ((e - 112) << 10) | (m >> 1);
      /* Extra rounding. An overflow will set mantissa to 0 and increment
       * the exponent, which is OK. */
      bits += m & 1;
    }
  }

  return bits;
}

cbor_status cbenc_float16(cbenc_ctx_t *ctx, float val)
{
  cbor_status cs = cbor_ok;
  union { float f; uint32_t bits; } v = { val };

  if(v.bits == 0) { return cbenc_uint(ctx, 0); }

  return_if_fail(cbenc_header(ctx, cbor_tsimple | st_size16, 2));

  *(uint16_t*)(&ctx->mem[1]) = cbor_bswap16(encode_float16(v.bits));

  return cs;
}

cbor_status cbenc_float32(cbenc_ctx_t *ctx, float val)
{
  cbor_status cs = cbor_ok;
  union { float f; uint32_t u; } v = { val };

  if(v.u == 0) { return cbenc_uint(ctx, 0); }

  return_if_fail(cbenc_header(ctx, cbor_tsimple | st_size32, 4));
  *(uint32_t*)(&ctx->mem[1]) = cbor_bswap32(v.u);

  return cs;
}

#endif // CBOR_ENABLE_FLOAT32_SUPPORT

#ifdef CBOR_ENABLE_FLOAT64_SUPPORT

cbor_status cbenc_float64(cbenc_ctx_t *ctx, double val)
{
  cbor_status cs = cbor_ok;
  union { double f; uint64_t u; } v = { val };

  if(v.u == 0) { return cbenc_uint(ctx, 0); }

  return_if_fail(cbenc_header(ctx, cbor_tsimple | st_size64, 8));
  *(uint64_t*)(&ctx->mem[1]) = cbor_bswap64(v.u);

  return cs;
}

#endif // CBOR_ENABLE_FLOAT64_SUPPORT

cbor_status cbenc_simple(cbenc_ctx_t *ctx, cbor_simple val)
{
  return cbenc_header(ctx, cbor_tsimple | val, 0);
}

cbor_status cbenc_bytestr_begin(cbenc_ctx_t *ctx)
{
  return cbenc_header(ctx, cbor_tbytestr | st_varbrk, 0);
}

cbor_status cbenc_bytestr(cbenc_ctx_t *ctx, const void *data, cbor_uint sz)
{
  return cbenc_bytes(ctx, cbor_tbytestr, data, sz);
}

cbor_status cbenc_textstr_begin(cbenc_ctx_t *ctx)
{
  return cbenc_header(ctx, cbor_ttextstr | st_varbrk, 0);
}

#ifdef CBOR_ENABLE_UTF8_SUPPORT

static const char *utf8_truncate(const uint8_t *s, cbor_uint char_limit)
{
  cbor_uint i, n;
  const uint8_t *end = s;

#ifndef CBOR_INTTYPE_64
  cbor_uint byte_limit = CBOR_UINT_MAX;
#endif

  while(*s && char_limit--) {
    n = 1;
    if(*s > 127) {
      if     ((*s & 0xE0) == 0xC0) { n += 1; }
      else if((*s & 0xF0) == 0xE0) { n += 2; }
      else if((*s & 0xF8) == 0xF0) { n += 3; }
      else if((*s & 0xFC) == 0xF8) { n += 4; }
      else if((*s & 0xFE) == 0xFC) { n += 5; }
      else                         { break;  }
    }

#ifndef CBOR_INTTYPE_64
    if(byte_limit < n) { break; }
    byte_limit -= n;
#endif

    for(i = 1; i < n; i++) {
      if((s[i] & 0xC0) != 0x80) { return (char*)end; }
    }

    s += n; end = s;
  }

  return (char*)end;
}

#endif // CBOR_ENABLE_UTF8_SUPPORT

cbor_status cbenc_textstr(cbenc_ctx_t *ctx, const char *data, cbor_uint sz)
{
#ifdef CBOR_ENABLE_UTF8_SUPPORT
  const char *end = utf8_truncate((uint8_t*)data, sz);
  return cbenc_bytes(ctx, cbor_ttextstr, data, end - data);
#else
  return cbenc_bytes(ctx, cbor_ttextstr, data, sz);
#endif
}

cbor_status cbenc_cstring(cbenc_ctx_t *ctx, const char *data)
{
  cbor_status cs = cbor_ok;

#ifdef CBOR_ENABLE_UTF8_SUPPORT
  const char *end = utf8_truncate((uint8_t*)data, CBOR_UINT_MAX);
  if(*end == 0) { return cbenc_bytes(ctx, cbor_ttextstr, data, end - data); }

  return_if_fail(cbenc_header(ctx, cbor_ttextstr | st_varbrk, 0));

  do {
    return_if_fail(cbenc_bytes(ctx, cbor_ttextstr, data, end - data));
    data = end;
    end  = utf8_truncate((uint8_t*)data, CBOR_UINT_MAX);
  } while(*end);

  if(end > data) { return_if_fail(cbenc_bytes(ctx, cbor_ttextstr, data, end - data)); }

  return cbenc_break(ctx);
#else
  size_t len, n;

  len = strlen(data);

  if(len > CBOR_UINT_MAX) {
    return_if_fail(cbenc_header(ctx, cbor_ttextstr | st_varbrk, 0));

    while(len) {
      n = (len > CBOR_UINT_MAX)? CBOR_UINT_MAX : len;
      return_if_fail(cbenc_bytes(ctx, cbor_ttextstr, data, n));
      len -= n; data += n;
    }

    return cbenc_break(ctx);
  }

  return cbenc_bytes(ctx, cbor_ttextstr, data, len);
#endif
}

cbor_status cbenc_bytestr_begin_sz(cbenc_ctx_t *ctx, cbor_uint sz)
{
  cbor_status cs = cbor_ok;

  return_if_fail(cbenc_uint(ctx, sz));
  ctx->mem[0] |= cbor_tbytestr;

  return cs;
}

cbor_status cbenc_textstr_begin_sz(cbenc_ctx_t *ctx, cbor_uint sz)
{
  cbor_status cs = cbor_ok;

  return_if_fail(cbenc_uint(ctx, sz));
  ctx->mem[0] |= cbor_ttextstr;

  return cs;
}

cbor_status cbenc_swrite(cbenc_ctx_t *ctx, const void *data, cbor_uint sz)
{
  cbor_status cs = cbor_ok;
  cbor_uint avail;
  const uint8_t *p = (uint8_t*)data;

  avail = encbuf_avail(ctx);

  if(avail > 0 && sz <= avail) {
    cs = encbuf_grow(ctx, sz);
    if(cs == cbor_ok) { memcpy(ctx->mem, p, sz); }
    return cs;
  }

  if(encbuf_datalen(ctx) > 0) {
    // flush buffer
    return_if_fail(ctx->write(ctx->buf, encbuf_datalen(ctx), ctx->usrdata));
    ctx->end = ctx->buf;
  }

  return ctx->write(p, sz, ctx->usrdata);
}

cbor_status cbenc_array_begin(cbenc_ctx_t *ctx)
{
  return cbenc_header(ctx, cbor_tarray | st_varbrk, 0);
}

cbor_status cbenc_array(cbenc_ctx_t *ctx, cbor_uint sz)
{
  cbor_status cs = cbor_ok;

  return_if_fail(cbenc_uint(ctx, sz));
  ctx->mem[0] |= cbor_tarray;

  return cs;
}

cbor_status cbenc_map_begin(cbenc_ctx_t *ctx)
{
  return cbenc_header(ctx, cbor_tmap | st_varbrk, 0);
}

cbor_status cbenc_map(cbenc_ctx_t *ctx, cbor_uint sz)
{
  cbor_status cs = cbor_ok;

  return_if_fail(cbenc_uint(ctx, sz));
  ctx->mem[0] |= cbor_tmap;

  return cs;
}

cbor_status cbenc_break(cbenc_ctx_t *ctx)
{
  return cbenc_header(ctx, cbor_tsimple | st_varbrk, 0);
}

cbor_status cbenc_tag(cbenc_ctx_t *ctx, cbor_uint tag)
{
  cbor_status cs = cbor_ok;

  return_if_fail(cbenc_uint(ctx, tag));
  ctx->mem[0] |= cbor_ttag;

  return cs;
}

#endif // CBOR_ENABLE_ENCODER_SUPPORT


#ifdef CBOR_ENABLE_DECODER_SUPPORT

static cbor_status cbdec_uint(cbdec_ctx_t *ctx, uint8_t s_type)
{
  cbor_status cs = cbor_ok;

  if(s_type < 24) { ctx->value.u = s_type; return cbor_ok; }

  switch(s_type) {
  case st_size8:
    return_if_fail(ctx->read(ctx->buf, 1, ctx->usrdata));
    ctx->value.u = *(uint8_t*)(ctx->buf);
    break;

#ifdef CBOR_INTTYPE_16
  case st_size16:
    return_if_fail(ctx->read(ctx->buf, 2, ctx->usrdata));
    ctx->value.u = cbor_bswap16(*(uint16_t*)(ctx->buf));
    break;
#endif

#ifdef CBOR_INTTYPE_32
  case st_size32:
    return_if_fail(ctx->read(ctx->buf, 4, ctx->usrdata));
    ctx->value.u = cbor_bswap32(*(uint32_t*)(ctx->buf));
    break;
#endif

#ifdef CBOR_INTTYPE_64
  case st_size64:
    return_if_fail(ctx->read(ctx->buf, 8, ctx->usrdata));
    ctx->value.u = cbor_bswap64(*(uint64_t*)(ctx->buf));
    break;
#endif

  default: return cbor_efmt;
  }

  return cs;
}

static inline cbor_status cbdec_int(cbdec_ctx_t *ctx, uint8_t s_type)
{
  cbor_status cs = cbor_ok;

  return_if_fail(cbdec_uint(ctx, s_type));
  ctx->value.s = -(ctx->value.s + 1);

  return cs;
}

#ifdef CBOR_ENABLE_FLOAT32_SUPPORT

/**
 * Convert float16 to float32
 *
 * @param  h - float16 bits
 * @return float32 bits
 *
 * @link   https://github.com/numpy/numpy
 * @copyright
 *   Copyright (c) 2005-2017, NumPy Developers.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are
 *   met:
 *
 *       * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *
 *       * Redistributions in binary form must reproduce the above
 *          copyright notice, this list of conditions and the following
 *          disclaimer in the documentation and/or other materials provided
 *          with the distribution.
 *
 *       * Neither the name of the NumPy Developers nor the names of any
 *          contributors may be used to endorse or promote products derived
 *          from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
static uint32_t decode_float16(uint16_t h)
{
  uint16_t h_exp, h_sig;
  uint32_t f_sgn, f_exp, f_sig;

  h_exp = (h & 0x7C00u);
  f_sgn = ((uint32_t)h & 0x8000u) << 16;
  switch(h_exp) {
  case 0x0000u: /* 0 or subnormal */
    h_sig = (h & 0x03FFu);
    /* Signed zero */
    if(h_sig == 0) { return f_sgn; }
    /* Subnormal */
    h_sig <<= 1;
    while((h_sig & 0x0400u) == 0) {
      h_sig <<= 1;
      h_exp++;
    }
    f_exp = ((uint32_t)(127 - 15 - h_exp)) << 23;
    f_sig = ((uint32_t)(h_sig & 0x03FFu)) << 13;
    return f_sgn + f_exp + f_sig;

  case 0x7C00u: /* inf or NaN */
    /* All-ones exponent and a copy of the significand */
    return f_sgn + 0x7F800000u + (((uint32_t)(h & 0x03FFu)) << 13);

  default: /* normalized */
    /* Just need to adjust the exponent and shift */
    return f_sgn + (((uint32_t)(h & 0x7FFFu) + 0x1C000u) << 13);
  }
}

static cbor_status cbdec_float(cbdec_ctx_t *ctx, uint8_t s_type)
{
  cbor_status cs = cbor_ok;

  if(s_type == st_size16) {
    return_if_fail(ctx->read(ctx->buf, 2, ctx->usrdata));
    ctx->value.u = decode_float16( cbor_bswap16(*(uint16_t*)(ctx->buf)) );
    return cbor_ok;
  }

  if(s_type == st_size32) {
    return_if_fail(ctx->read(ctx->buf, 4, ctx->usrdata));
    ctx->value.u = cbor_bswap32(*(uint32_t*)(ctx->buf));
    return cbor_ok;
  }

  return cbor_efmt;
}

#endif // CBOR_ENABLE_FLOAT32_SUPPORT

#ifdef CBOR_ENABLE_FLOAT64_SUPPORT

static cbor_status cbdec_float64(cbdec_ctx_t *ctx)
{
  cbor_status cs = cbor_ok;

  return_if_fail(ctx->read(ctx->buf, 8, ctx->usrdata));
  ctx->value.u = cbor_bswap64(*(uint64_t*)(ctx->buf));

  return cs;
}

#endif // CBOR_ENABLE_FLOAT64_SUPPORT

cbor_status cbdec_step(cbdec_ctx_t *ctx)
{
  cbor_status cs = cbor_ok;
  uint8_t s_type;

  return_if_fail(ctx->read(ctx->buf, 1, ctx->usrdata));

  ctx->token = ctx->buf[0] & 0xE0;
  s_type     = ctx->buf[0] & 0x1F;

  if(ctx->token == cbor_tsimple) {
    switch(s_type) {
#ifdef CBOR_ENABLE_FLOAT32_SUPPORT
    case st_size16:
    case st_size32:
      ctx->token = cbor_tfloat32;
      return cbdec_float(ctx, s_type);
#else
    case st_size16:
    case st_size32:
      return cbor_efmt;
#endif

#ifdef CBOR_ENABLE_FLOAT64_SUPPORT
    case st_size64:
      ctx->token = cbor_tfloat64;
      return cbdec_float64(ctx);
#else
    case st_size64:
      return cbor_efmt;
#endif

    case st_varbrk:
      ctx->token = cbor_tbreak;
      return cbor_ok;

    case cbor_false:
    case cbor_true:
    case cbor_null:
    case cbor_undef:
      ctx->value.st = s_type;
      return cbor_ok;
    }
  }
  else
  if(s_type == st_varbrk && ctx->token >= cbor_tbytestr && ctx->token <= cbor_tmap) {
    ctx->token |= 1; return cbor_ok;
  }

  switch(ctx->token) {
    case cbor_tuint:
    case cbor_ttag:
    case cbor_tbytestr:
    case cbor_ttextstr:
    case cbor_tarray:
    case cbor_tmap:
      return cbdec_uint(ctx, s_type);

    case cbor_tint:
      return cbdec_int(ctx, s_type);

    default:
      ctx->token = cbor_tinvalid;
      break;
  }

  return cbor_efmt;
}

cbor_status cbdec_sread(cbdec_ctx_t *ctx, void *data, cbor_uint sz)
{
  cbor_status cs = cbor_ok;
  cbor_uint n;

  n = (sz > ctx->value.u)? ctx->value.u : sz;
  if(n == 0) { return cbor_ok; }

  return_if_fail(ctx->read(data, n, ctx->usrdata));
  ctx->value.u -= n;

  return cs;
}

#endif // CBOR_ENABLE_DECODER_SUPPORT
