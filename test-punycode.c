#include "punycode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static size_t unilen(const uint32_t *input) {
  const uint32_t *p;

  for (p = input; *p; p++);

  return p - input;
}

static void test_enc(const uint32_t *input, const char *expected) {
  static const char canary[] = { 0xDE, 0xAD, 0xBA, 0xBE };
  char dstbuf[1024];
  size_t n_converted;
  size_t dstlen;
  size_t i;

  for (i = 0; i < sizeof dstbuf; i += sizeof canary) {
    memcpy(&dstbuf[i], canary, sizeof canary);
  }

  dstlen = sizeof dstbuf;
  n_converted = punycode_encode(input, unilen(input), dstbuf, &dstlen);

  assert(dstlen < sizeof dstbuf);

  /* check for buffer overrun */
  for (i = dstlen; i < sizeof dstbuf; i++) {
    assert(dstbuf[i] == canary[i % sizeof canary]);
  }

  assert(n_converted == unilen(input));
  assert(memcmp(dstbuf, expected, strlen(expected)) == 0);
}

int main(void) {
  /* ü */
  const uint32_t stanza0[] = { 0xFC, 0 };

  /* Bach (only basic code points) */
  const uint32_t stanza1[] = { 'B', 'a', 'c', 'h', 0 };

  /* bücher */
  const uint32_t stanza2[] = { 'b', 0xFC, 'c', 'h', 'e', 'r', 0 };

  /* Willst du die Blüthe des frühen, die Früchte des späteren Jahres */
  const uint32_t stanza3[] = {
    'W', 'i', 'l', 'l', 's', 't', ' ', 'd', 'u', ' ',
    'd', 'i', 'e', ' ', 'B', 'l', 0xFC, 't', 'h', 'e', ' ',
    'd', 'e', 's', ' ', 'f', 'r', 0xFC, 'h', 'e', 'n', ',', ' ',
    'd', 'i', 'e', ' ', 'F', 'r', 0xFC, 'c', 'h', 't', 'e', ' ',
    'd', 'e', 's', ' ', 's', 'p', 0xE4, 't', 'e', 'r', 'e', 'n', ' ',
    'J', 'a', 'h', 'r', 'e', 's', 0
  };

  struct {
    const uint32_t *unicode;
    const char *punycode;
  }
  simple_tests[] = {
    { stanza0, "tda" },
    { stanza1, "Bach" },
    { stanza2, "bcher-kva" },
    { stanza3, "Willst du die Blthe des frhen, die Frchte des spteren Jahres-x9e96lkal" },
  };

  unsigned i;

  for (i = 0; i < sizeof(simple_tests) / sizeof(simple_tests[0]); i++) {
    test_enc(simple_tests[i].unicode, simple_tests[i].punycode);
  }

  return 0;
}
