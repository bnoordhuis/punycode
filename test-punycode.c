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

  /* Examples from RFC */
  const uint32_t exampleA[] = { 0x0644, 0x064A, 0x0647, 0x0645, 0x0627, 0x0628, 0x062A, 0x0643, 0x0644
      , 0x0645, 0x0648, 0x0634, 0x0639, 0x0631, 0x0628, 0x064A, 0x061F, 0 };
  const uint32_t exampleB[] = { 0x4ED6, 0x4EEC, 0x4E3A, 0x4EC0, 0x4E48, 0x4E0D, 0x8BF4, 0x4E2D, 0x6587, 0 };
  const uint32_t exampleC[] = { 0x4ED6, 0x5011, 0x7232, 0x4EC0, 0x9EBD, 0x4E0D, 0x8AAA, 0x4E2D, 0x6587, 0 };
  const uint32_t exampleD[] = { 0x0050, 0x0072, 0x006F, 0x010D, 0x0070, 0x0072, 0x006F, 0x0073, 0x0074
      , 0x011B, 0x006E, 0x0065, 0x006D, 0x006C, 0x0075, 0x0076, 0x00ED, 0x010D
      , 0x0065, 0x0073, 0x006B, 0x0079, 0 };
  const uint32_t exampleE[] = { 0x05DC, 0x05DE, 0x05D4, 0x05D4, 0x05DD, 0x05E4, 0x05E9, 0x05D5, 0x05D8
      , 0x05DC, 0x05D0, 0x05DE, 0x05D3, 0x05D1, 0x05E8, 0x05D9, 0x05DD, 0x05E2
      , 0x05D1, 0x05E8, 0x05D9, 0x05EA, 0 };
  const uint32_t exampleF[] = { 0x092F, 0x0939, 0x0932, 0x094B, 0x0917, 0x0939, 0x093F, 0x0928, 0x094D
      , 0x0926, 0x0940, 0x0915, 0x094D, 0x092F, 0x094B, 0x0902, 0x0928, 0x0939
      , 0x0940, 0x0902, 0x092C, 0x094B, 0x0932, 0x0938, 0x0915, 0x0924, 0x0947
      , 0x0939, 0x0948, 0x0902, 0 };
  const uint32_t exampleG[] = { 0x306A, 0x305C, 0x307F, 0x3093, 0x306A, 0x65E5, 0x672C, 0x8A9E, 0x3092
      , 0x8A71, 0x3057, 0x3066, 0x304F, 0x308C, 0x306A, 0x3044, 0x306E, 0x304B, 0 };
  const uint32_t exampleH[] = { 0xC138, 0xACC4, 0xC758, 0xBAA8, 0xB4E0, 0xC0AC, 0xB78C, 0xB4E4, 0xC774
      , 0xD55C, 0xAD6D, 0xC5B4, 0xB97C, 0xC774, 0xD574, 0xD55C, 0xB2E4, 0xBA74
      , 0xC5BC, 0xB9C8, 0xB098, 0xC88B, 0xC744, 0xAE4C, 0 };
 const uint32_t exampleI[] = { 0x043F, 0x043E, 0x0447, 0x0435, 0x043C, 0x0443, 0x0436, 0x0435, 0x043E
      , 0x043D, 0x0438, 0x043D, 0x0435, 0x0433, 0x043E, 0x0432, 0x043E, 0x0440
      , 0x044F, 0x0442, 0x043F, 0x043E, 0x0440, 0x0443, 0x0441, 0x0441, 0x043A
      , 0x0438, 0 };
  const uint32_t exampleJ[] = { 0x0050, 0x006F, 0x0072, 0x0071, 0x0075, 0x00E9, 0x006E, 0x006F, 0x0070
      , 0x0075, 0x0065, 0x0064, 0x0065, 0x006E, 0x0073, 0x0069, 0x006D, 0x0070
      , 0x006C, 0x0065, 0x006D, 0x0065, 0x006E, 0x0074, 0x0065, 0x0068, 0x0061
      , 0x0062, 0x006C, 0x0061, 0x0072, 0x0065, 0x006E, 0x0045, 0x0073, 0x0070
      , 0x0061, 0x00F1, 0x006F, 0x006C, 0 };
  const uint32_t exampleK[] = { 0x0054, 0x1EA1, 0x0069, 0x0073, 0x0061, 0x006F, 0x0068, 0x1ECD, 0x006B
      , 0x0068, 0x00F4, 0x006E, 0x0067, 0x0074, 0x0068, 0x1EC3, 0x0063, 0x0068
      , 0x1EC9, 0x006E, 0x00F3, 0x0069, 0x0074, 0x0069, 0x1EBF, 0x006E, 0x0067
      , 0x0056, 0x0069, 0x1EC7, 0x0074, 0 };
  const uint32_t exampleL[] = { 0x0033, 0x5E74, 0x0042, 0x7D44, 0x91D1, 0x516B, 0x5148, 0x751F, 0 };
  const uint32_t exampleM[] = { 0x5B89, 0x5BA4, 0x5948, 0x7F8E, 0x6075, 0x002D, 0x0077, 0x0069, 0x0074
      , 0x0068, 0x002D, 0x0053, 0x0055, 0x0050, 0x0045, 0x0052, 0x002D, 0x004D
      , 0x004F, 0x004E, 0x004B, 0x0045, 0x0059, 0x0053, 0 };
  const uint32_t exampleN[] = { 0x0048, 0x0065, 0x006C, 0x006C, 0x006F, 0x002D, 0x0041, 0x006E, 0x006F
      , 0x0074, 0x0068, 0x0065, 0x0072, 0x002D, 0x0057, 0x0061, 0x0079, 0x002D
      , 0x305D, 0x308C, 0x305E, 0x308C, 0x306E, 0x5834, 0x6240, 0 };
  const uint32_t exampleO[] = { 0x3072, 0x3068, 0x3064, 0x5C4B, 0x6839, 0x306E, 0x4E0B, 0x0032, 0 };
  const uint32_t exampleP[] = { 0x004D, 0x0061, 0x006A, 0x0069, 0x3067, 0x004B, 0x006F, 0x0069, 0x3059
      , 0x308B, 0x0035, 0x79D2, 0x524D, 0 };
  const uint32_t exampleQ[] = { 0x30D1, 0x30D5, 0x30A3, 0x30FC, 0x0064, 0x0065, 0x30EB, 0x30F3, 0x30D0, 0 };
  const uint32_t exampleR[] = { 0x305D, 0x306E, 0x30B9, 0x30D4, 0x30FC, 0x30C9, 0x3067, 0 };
  const uint32_t exampleS[] = { 0x002D, 0x003E, 0x0020, 0x0024, 0x0031, 0x002E, 0x0030, 0x0030, 0x0020
      , 0x003C, 0x002D, 0 };

  struct {
    const uint32_t *unicode;
    const char *punycode;
  }
  simple_tests[] = {
    { stanza0, "tda" },
    { stanza1, "Bach-" },
    { stanza2, "bcher-kva" },
    { stanza3, "Willst du die Blthe des frhen, die Frchte des spteren Jahres-x9e96lkal" },
    { exampleA, "egbpdaj6bu4bxfgehfvwxn" },
    { exampleB, "ihqwcrb4cv8a8dqg056pqjye"},
    { exampleC, "ihqwctvzc91f659drss3x8bo0yb"},
    { exampleD, "Proprostnemluvesky-uyb24dma41a"},
    { exampleE, "4dbcagdahymbxekheh6e0a7fei0b"},
    { exampleF, "i1baa7eci9glrd9b2ae1bj0hfcgg6iyaf8o0a1dig0cd"},
    { exampleG, "n8jok5ay5dzabd5bym9f0cm5685rrjetr6pdxa"},
    { exampleH, "989aomsvi5e83db1d2a355cv1e0vak1dwrv93d5xbh15a0dt30a5jpsd879ccm6fea98c"},
    { exampleI, "b1abfaaepdrnnbgefbadotcwatmq2g4l"},
    { exampleJ, "PorqunopuedensimplementehablarenEspaol-fmd56a"},
    { exampleK, "TisaohkhngthchnitingVit-kjcr8268qyxafd2f1b9g"},
    { exampleL, "3B-ww4c5e180e575a65lsy2b"},
    { exampleM, "-with-SUPER-MONKEYS-pc58ag80a8qai00g7n9n"},
    { exampleN, "Hello-Another-Way--fc4qua05auwb3674vfr0b"},
    { exampleO, "2-u9tlzr9756bt3uc0v"},
    { exampleP, "MajiKoi5-783gue6qz075azm5e"},
    { exampleQ, "de-jg4avhby1noc0d"},
    { exampleR, "d9juau41awczczp"},
    { exampleS, "-> $1.00 <--"}
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
