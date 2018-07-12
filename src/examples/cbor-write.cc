#include <fstream>
#include <iostream>
#include <iomanip>
#include "cbor.h"

static std::fstream cbor_f;

static cbor_status cbor_write(const void *data, cbor_uint n, void*)
{
  cbor_f.write(static_cast<const char*>(data), n);

// HEX output:
//  const uint8_t *p = (uint8_t*)data;
//  while(n--) { std::cout << std::setw(2) << std::setfill('0') << std::hex << int(*p++); }
  return cbor_ok;
}

int main(int, char**)
{
  cbor_f.open("/tmp/test.cb", std::ios::out | std::ios::binary);

  uint8_t buf[9];
  cbenc_ctx ctx = CBOR_ENCODER_CTX_INITIALIZER(cbor_write, buf, sizeof(buf), nullptr);

  struct
  {
    uint32_t n = 10;
    uint8_t buf[4] = {0x1, 0x2, 0x3, 0x4};
    float val = 0.321;
  } test;

  //
  // Equivalent data structure
  //
  // [
  //   true,
  //   null,
  //   undefined,
  //   12345(9000),
  //   [1.0, 0.12300000339746475, 123456789.12345679, "some text"],
  //   {
  //     1: "hello",
  //     2: -9223372036854775807,
  //     "test": 12345678
  //   },
  //   {
  //     "key1": 0.1,
  //     "key2": 0.3,
  //     "key3": 0.5
  //   },
  //   (_ "This ", "text", " is one ", "string ", "encoded", " by parts"),
  //   "This text is one string encoded by parts",
  //   h'0A000000010203041D5AA43E'
  // ]

  cbenc_begin(&ctx); // start CBOR encoding
    cbenc_array_begin(&ctx); // begin variable length array

      // encode some simple types
      cbenc_simple(&ctx, cbor_true);
      cbenc_simple(&ctx, cbor_null);
      cbenc_simple(&ctx, cbor_undef);

      // uint value with 12345 tag
      cbenc_tag(&ctx, 12345);
        cbenc_uint(&ctx, 9000);

      // fixed length array with 4 items
      cbenc_array(&ctx, 4);
        cbenc_float16(&ctx, 1.0);
        cbenc_float32(&ctx, 0.123);
        cbenc_float64(&ctx, 123456789.123456789);
        cbenc_cstring(&ctx, "some text");
        // end of array

      // fixed length map with 3 key-value pairs
      cbenc_map(&ctx, 3);
        cbenc_uint(&ctx, 1); // key
        cbenc_textstr(&ctx, "hello", 5); // value

        cbenc_uint(&ctx, 2); // key
        cbenc_int(&ctx, -9223372036854775807); // value

        cbenc_cstring(&ctx, "test"); // key
        cbenc_uint(&ctx, 12345678);
        // end of fixed length map

      // begin variable length map
      cbenc_map_begin(&ctx);
        cbenc_cstring(&ctx, "key1"); // key
        cbenc_float32(&ctx, 0.1); // value

        cbenc_cstring(&ctx, "key2"); // key
        cbenc_float32(&ctx, 0.3); // value

        cbenc_cstring(&ctx, "key3"); // key
        cbenc_float32(&ctx, 0.5); // value
      cbenc_break(&ctx); // end of variable length map

      // begin variable length string
      cbenc_textstr_begin(&ctx);
        cbenc_textstr(&ctx, "This ", 5);
        cbenc_textstr(&ctx, "text", 4);
        cbenc_textstr(&ctx, " is one ", 8);
        cbenc_textstr(&ctx, "string ", 7);
        cbenc_textstr(&ctx, "encoded", 7);
        cbenc_textstr(&ctx, " by parts", 9);
      cbenc_break(&ctx); // end of variable length string

      // fixed length string writed by parts (encoded result, is fully equivalent to cbenc_textstr)
      cbenc_textstr_begin_sz(&ctx, 5 + 4 + 8 + 7 + 7 + 9);
        cbenc_swrite(&ctx, "This ", 5);
        cbenc_swrite(&ctx, "text", 4);
        cbenc_swrite(&ctx, " is one ", 8);
        cbenc_swrite(&ctx, "string ", 7);
        cbenc_swrite(&ctx, "encoded", 7);
        cbenc_swrite(&ctx, " by parts", 9);
        // end of fixed length string

      // binary data
      cbenc_bytestr(&ctx, &test, sizeof(test));

    cbenc_break(&ctx); // end of variable length array
  cbenc_end(&ctx); // end of CBOR encoding

  return 0;
}
