/*
 *	BIRD Library -- SHA-256, SHA-224, HMAC-SHA-256 and HMAC-SHA224 Tests
 *
 *	(c) 2015 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdlib.h>

#include "test/birdtest.h"
#include "test/birdtest_support.h"
#include "sysdep/config.h"
#include "lib/sha256.h"
#include "lib/sha256.c" /* REMOVE ME */


static void
byte_to_hex(char *out, const byte *in, uint len)
{
  int i;
  for(i = 0; i < len; i++)
    sprintf(out + i*2, "%02x", in[i]);
}

static void
get_sha256(const char *str, char (*out_hash)[SHA256_HEX_SIZE])
{
  struct sha256_context ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, str, strlen(str));
  byte *hash = sha256_final(&ctx);
  byte_to_hex((char*)out_hash, hash, SHA256_SIZE);
}

static void
get_sha224(const char *str, char (*out_hash)[SHA256_HEX_SIZE])
{
  struct sha224_context ctx;
  sha224_init(&ctx);
  sha224_update(&ctx, str, strlen(str));
  byte *hash = sha224_final(&ctx);
  byte_to_hex((char*)out_hash, hash, SHA224_SIZE);
}

static int
t_sha256(void)
{
  struct in_out {
    char *in;
    char out[SHA256_HEX_SIZE];
  } in_out[] = {
      {
	  .in  = "",
	  .out = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
      },
      {
	  .in  = "a",
	  .out = "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb",
      },
      {
	  .in  = "abc",
	  .out = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
      },
      {
	  .in  = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
	  .out = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
      },
      {
	  .in  = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
	  .out = "bf18b43b61652b5d73f41ebf3d72e5e43aebf5076f497dde31ea3de9de4998ef",
      },
  };

  bt_assert_fn_in_out(get_sha256, in_out, "'%s'", "'%s'");

  return BT_SUCCESS;
}

static int
t_sha224(void)
{
  struct in_out {
    char *in;
    char out[SHA256_HEX_SIZE];
  } in_out[] = {
      {
	  .in  = "",
	  .out = "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f",
      },
      {
	  .in  = "a",
	  .out = "abd37534c7d9a2efb9465de931cd7055ffdb8879563ae98078d6d6d5",
      },
      {
	  .in  = "abc",
	  .out = "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7",
      },
      {
	  .in  = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
	  .out = "75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525",
      },
      {
	  .in  = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
	  .out = "cca7dd1a332a17775d8b0429bdb45055c2d4368ebaab0c7cf385586e",
      },

  };

  bt_assert_fn_in_out(get_sha224, in_out, "'%s'", "'%s'");

  return BT_SUCCESS;
}

static int
t_sha256_concating(void)
{
  char hash_a[SHA256_HEX_SIZE];
  char hash_b[SHA256_HEX_SIZE];

  char *str_a  = "a" "bb" "ccc" "dddd" "eeeee" "ffffff";
  char *str_b1 = "a"                                   ;
  char *str_b2 =     "bb"                              ;
  char *str_b3 =          "ccc"                        ;
  char *str_b4 =                "dddd"                 ;
  char *str_b5 =                       "eeeee"         ;
  char *str_b6 =                               "ffffff";

  struct sha256_context ctx_a;
  sha256_init(&ctx_a);
  sha256_update(&ctx_a, str_a, strlen(str_a));
  byte *hash_a_ = sha256_final(&ctx_a);
  byte_to_hex(hash_a, hash_a_, SHA256_SIZE);

  struct sha256_context ctx_b;
  sha256_init(&ctx_b);
  sha256_update(&ctx_b, str_b1, strlen(str_b1));
  sha256_update(&ctx_b, str_b2, strlen(str_b2));
  sha256_update(&ctx_b, str_b3, strlen(str_b3));
  sha256_update(&ctx_b, str_b4, strlen(str_b4));
  sha256_update(&ctx_b, str_b5, strlen(str_b5));
  sha256_update(&ctx_b, str_b6, strlen(str_b6));
  byte *hash_b_ = sha256_final(&ctx_b);
  byte_to_hex(hash_b, hash_b_, SHA256_SIZE);

  int are_hash_a_b_equal = (strncmp(hash_a, hash_b, sizeof(hash_a)) == 0);
  bt_assert_msg(are_hash_a_b_equal, "Hashes are different: \n A: %s \n B: %s ", hash_a, hash_b);

  return BT_SUCCESS;
}

#define HMAC_BUFFER_SIZE 160
struct hmac_data_in {
  byte key[HMAC_BUFFER_SIZE];
  uint key_len;
  byte data[HMAC_BUFFER_SIZE];
  uint data_len;
};

static void
get_sha256_hmac(const struct hmac_data_in in, char (*out_hash)[SHA256_HEX_SIZE])
{
  struct sha256_hmac_context ctx;
  sha256_hmac_init(&ctx, in.key, in.key_len);
  sha256_hmac_update(&ctx, in.data, in.data_len);
  byte *hash_byte = sha256_hmac_final(&ctx);
  byte_to_hex((char*)out_hash, hash_byte, SHA256_SIZE);
}

static void
get_sha224_hmac(const struct hmac_data_in in, char (*out_hash)[SHA224_HEX_SIZE])
{
  struct sha224_hmac_context ctx;
  sha224_hmac_init(&ctx, in.key, in.key_len);
  sha224_hmac_update(&ctx, in.data, in.data_len);
  byte *hash_byte = sha224_hmac_final(&ctx);
  byte_to_hex((char*)out_hash, hash_byte, SHA224_SIZE);
}


static int
t_sha256_hmac(void)
{
  struct in_out {
    struct hmac_data_in in;
    char out[SHA256_HEX_SIZE];
  } in_out[] = {
      {
	  .in  = {
	      .key = {
		  0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
		  0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	      },
	      .key_len = 20,
	      .data = "Hi There",
	      .data_len = 8,
	  },
	  .out = "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7",
      },
      {
	  .in  = {
	      .key = "Jefe",
	      .key_len = 4,
	      .data = "what do ya want for nothing?",
	      .data_len = 28,
	  },
	  .out = "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843",
      },
      {
	  .in  = {
	      .key = {
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	      },
	      .key_len = 20,
	      .data = {
		  0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
		  0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
		  0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
		  0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
		  0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
	      },
	      .data_len = 50,
	  },
	  .out = "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe",
      },
      {
	  .in  = {
	      .key = {
		  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
		  0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
		  0x15, 0x16, 0x17, 0x18, 0x19,
	      },
	      .key_len = 25,
	      .data = {
		  0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		  0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		  0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		  0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		  0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
	      },
	      .data_len = 50,
	  },
	  .out = "82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b",
      },
      {
	  .in  = {
	      .key = {
		  0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
		  0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
	      },
	      .key_len = 20,
	      .data = "Test With Truncation",
	      .data_len = 20,
	  },
	  .out = "a3b6167473100ee06e0c796c2955552bfa6f7c0a6a8aef8b93f860aab0cd20c5",
      },
      {
	  .in  = {
	      .key = {
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa,
	      },
	      .key_len = 131,
	      .data = "Test Using Larger Than Block-Size Key - Hash Key First",
	      .data_len = 54,
	  },
	  .out = "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54",
      },
      {
	  .in  = {
	      .key = {
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa,
	      },
	      .key_len = 131,
	      .data = "This is a test using a larger than block-size key and a larger than block-size data. The key needs to be hashed before being used by the HMAC algorithm.",
	      .data_len = 152,
	  },
	  .out = "9b09ffa71b942fcb27635fbcd5b0e944bfdc63644f0713938a7f51535c3a35e2",
      },
  };

  bt_assert_fn_in_out(get_sha256_hmac, in_out, NULL, "'%s'");

  return BT_SUCCESS;
}


static int
t_sha224_hmac(void)
{
  struct in_out {
    struct hmac_data_in in;
    char out[SHA224_HEX_SIZE];
  } in_out[] = {
      {
	  .in  = {
	      .key = {
		  0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
		  0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	      },
	      .key_len = 20,
	      .data = "Hi There",
	      .data_len = 8,
	  },
	  .out = "896fb1128abbdf196832107cd49df33f47b4b1169912ba4f53684b22",
      },
      {
	  .in  = {
	      .key = "Jefe",
	      .key_len = 4,
	      .data = "what do ya want for nothing?",
	      .data_len = 28,
	  },
	  .out = "a30e01098bc6dbbf45690f3a7e9e6d0f8bbea2a39e6148008fd05e44",
      },
      {
	  .in  = {
	      .key = {
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	      },
	      .key_len = 20,
	      .data = {
		  0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
		  0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
		  0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
		  0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
		  0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
	      },
	      .data_len = 50,
	  },
	  .out = "7fb3cb3588c6c1f6ffa9694d7d6ad2649365b0c1f65d69d1ec8333ea",
      },
      {
	  .in  = {
	      .key = {
		  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
		  0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
		  0x15, 0x16, 0x17, 0x18, 0x19,
	      },
	      .key_len = 25,
	      .data = {
		  0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		  0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		  0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		  0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
		  0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd,
	      },
	      .data_len = 50,
	  },
	  .out = "6c11506874013cac6a2abc1bb382627cec6a90d86efc012de7afec5a",
      },
      {
	  .in  = {
	      .key = {
		  0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
		  0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
	      },
	      .key_len = 20,
	      .data = "Test With Truncation",
	      .data_len = 20,
	  },
	  .out = "0e2aea68a90c8d37c988bcdb9fca6fa8099cd857c7ec4a1815cac54c",
      },
      {
	  .in  = {
	      .key = {
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa,
	      },
	      .key_len = 131,
	      .data = "Test Using Larger Than Block-Size Key - Hash Key First",
	      .data_len = 54,
	  },
	  .out = "95e9a0db962095adaebe9b2d6f0dbce2d499f112f2d2b7273fa6870e",
      },
      {
	  .in  = {
	      .key = {
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		  0xaa,
	      },
	      .key_len = 131,
	      .data = "This is a test using a larger than block-size key and a larger than block-size data. The key needs to be hashed before being used by the HMAC algorithm.",
	      .data_len = 152,
	  },
	  .out = "3a854166ac5d9f023f54d517d0b39dbd946770db9c2b95c9f6f565d1",
      },
  };

  bt_assert_fn_in_out(get_sha224_hmac, in_out, NULL, "'%s'");

  return BT_SUCCESS;
}

int
main(int argc, char *argv[])
{
  bt_init(argc, argv);

  bt_test_suite(t_sha256, "Testing SHA256");
  bt_test_suite(t_sha224, "Testing SHA224");
  bt_test_suite(t_sha256_concating, "Testing concating input string to hash process via sha256_update");

  bt_test_suite(t_sha256_hmac, "Test Suite by RFC 4231");
  bt_test_suite(t_sha224_hmac, "Test Suite by RFC 4231");

  return bt_end();
}