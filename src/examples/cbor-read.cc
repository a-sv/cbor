#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include "cbor.h"

static std::fstream cbor_f;

cbor_status cbor_read(void *data, cbor_uint sz, void*)
{
  cbor_f.read(static_cast<char*>(data), sz);
  return cbor_f.good()? cbor_ok : cbor_eos;
}

static void print_bytestr(cbdec_ctx *ctx)
{
  char buf[64];

  std::cout << "h'";
  while(ctx->value.u) {
    cbor_uint u = ctx->value.u;
    cbdec_sread(ctx, buf, sizeof(buf));

    const uint8_t *p = (uint8_t*)buf;
    while(ctx->value.u < u--) {
      std::cout << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << int(*p++);
    }
  }
  std::cout << "'";
}

static void print_textstr(cbdec_ctx *ctx)
{
  char buf[64];

  std::cout << '"';
  while(ctx->value.u) {
    cbor_uint u = ctx->value.u;
    cbdec_sread(ctx, buf, sizeof(buf));
    std::cout.write(buf, u - ctx->value.u);
  }
  std::cout << '"';
}

static void print(cbdec_ctx *ctx)
{
  switch(ctx->token) {
  case cbor_tuint:
    std::cout.operator<<(ctx->value.u);
    break;

  case cbor_tint:
    std::cout.operator<<(ctx->value.s);
    break;

  case cbor_tfloat32:
    std::cout << std::fixed << std::setprecision(8) << ctx->value.f32;
    break;

  case cbor_tfloat64:
    std::cout << std::fixed << std::setprecision(8) << ctx->value.f64;
    break;

  case cbor_tbytestr:
    print_bytestr(ctx);
    break;

  case cbor_ttextstr:
    print_textstr(ctx);
    break;

  case cbor_tarray: {
    bool first = true;
    std::cout << "[";
    cbor_uint u = ctx->value.u;
    while(u-- && cbdec_step(ctx) == cbor_ok) {
      if(!first) { std::cout << ", "; } else { first = false; }
      print(ctx);
    }
    std::cout << "]";
  } break;

  case cbor_tmap: {
    bool first = true;
    std::cout << "{";
    cbor_uint u = ctx->value.u;
    while(u--) {
      if(!first) { std::cout << ", "; } else { first = false; }

      if(cbdec_step(ctx) != cbor_ok) { return; }
      print(ctx);

      std::cout << " : ";

      if(cbdec_step(ctx) != cbor_ok) { return; }
      print(ctx);
    }
    std::cout << "}";
  } break;

  case cbor_ttag:
    std::cout.operator<<(ctx->value.u);
    std::cout << "(";

    if(cbdec_step(ctx) != cbor_ok) { return; }
    print(ctx);

    std::cout << ")";
    break;

  case cbor_tsimple: {
    switch(ctx->value.st) {
    case cbor_false: std::cout << "false";     break;
    case cbor_true:  std::cout << "true";      break;
    case cbor_null:  std::cout << "null";      break;
    case cbor_undef: std::cout << "undefined"; break;
    }
  } break;

  case cbor_tvbytestr: {
    bool first = true;
    std::cout << "(_ ";
    while(cbdec_step(ctx) == cbor_ok && ctx->token != cbor_tbreak) {
      if(!first) { std::cout << ", "; } else { first = false; }

      if(ctx->token == cbor_tbytestr) { print_bytestr(ctx); } else { return; }
    }
    std::cout << ")";
  } break;

  case cbor_tvtextstr: {
    bool first = true;
    std::cout << "(_ ";
    while(cbdec_step(ctx) == cbor_ok && ctx->token != cbor_tbreak) {
      if(!first) { std::cout << ", "; } else { first = false; }

      if(ctx->token == cbor_ttextstr) { print_textstr(ctx); } else { return; }
    }
    std::cout << ")";
  } break;

  case cbor_tvarray: {
    bool first = true;
    std::cout << "[";
    while(cbdec_step(ctx) == cbor_ok && ctx->token != cbor_tbreak) {
      if(!first) { std::cout << ", "; } else { first = false; }
      print(ctx);
    }
    std::cout << "]";
  } break;

  case cbor_tvmap: {
    bool first = true;
    std::cout << "{";
    while(cbdec_step(ctx) == cbor_ok && ctx->token != cbor_tbreak) {
      if(!first) { std::cout << ", "; } else { first = false; }

      print(ctx);

      std::cout << " : ";

      if(cbdec_step(ctx) != cbor_ok) { return; }
      print(ctx);
    }
    std::cout << "}";
  } break;

  default: break;
  }
}

int main(int, char**)
{
  cbor_f.open("/tmp/test.cb", std::ios::in | std::ios::binary);

  cbdec_ctx ctx = CBOR_DECODER_CTX_INITIALIZER(cbor_read, nullptr);

  while(cbdec_step(&ctx) == cbor_ok) { print(&ctx); }

  std::cout << std::endl;

  return 0;
}
