/**
 * Copyright (C) 2011 by Ben Noordhuis <info@bnoordhuis.nl>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "punycode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

static const char canary[] = { 0xDE, 0xAD, 0xBA, 0xBE };

static size_t unilen(const uint32_t *input) {
  const uint32_t *p;

  for (p = input; *p; p++);

  return p - input;
}

static void write_canary(void *dst, size_t len) {
  size_t i;

  for (i = 0; i < len; i += sizeof canary) {
    memcpy((char *) dst + i, canary, min(len - i, sizeof canary));
  }
}

static void check_canary(const void *dst, size_t len, size_t off) {
  for (; off < len; off++) {
    assert(*((const char *) dst + off) == canary[off % sizeof canary]);
  }
}

static void test_encoder(const uint32_t *input, const char *expected) {
  char dstbuf[1024];
  size_t n_converted;
  size_t dstlen;

  write_canary(dstbuf, sizeof dstbuf);

  dstlen = sizeof dstbuf;
  n_converted = punycode_encode(input, unilen(input), dstbuf, &dstlen);

  assert(dstlen <= sizeof dstbuf);
  check_canary(dstbuf, sizeof dstbuf, dstlen);

  assert(n_converted == unilen(input));
  assert(memcmp(dstbuf, expected, strlen(expected)) == 0);
}

static void test_decoder(const char *input, const uint32_t *expected) {
  uint32_t dstbuf[1024];
  size_t n_converted;
  size_t dstlen;

  write_canary(dstbuf, sizeof dstbuf);

  dstlen = sizeof dstbuf;
  n_converted = punycode_decode(input, strlen(input), dstbuf, &dstlen);

  assert(dstlen <= sizeof dstbuf);
  check_canary(dstbuf, sizeof dstbuf, dstlen * sizeof(dstbuf[0]));

  assert(n_converted == strlen(input));
  assert(memcmp(dstbuf, expected, unilen(expected)) == 0);
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
    test_encoder(simple_tests[i].unicode, simple_tests[i].punycode);
  }

  for (i = 0; i < sizeof(simple_tests) / sizeof(simple_tests[0]); i++) {
    test_decoder(simple_tests[i].punycode, simple_tests[i].unicode);
  }

  return 0;
}
