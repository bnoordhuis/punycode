#include "punycode.h"
#include <stdint.h>
#include <assert.h>

/* punycode parameters, see http://tools.ietf.org/html/rfc3492#section-5 */
#define BASE 36
#define TMIN 1
#define TMAX 26
#define SKEW 38
#define DAMP 700
#define INITIAL_N 128
#define INITIAL_BIAS 72

static uint32_t adapt_bias(uint32_t delta, unsigned n_points, int is_first) {
  uint32_t k;

  delta /= is_first ? DAMP : 2;
  delta += delta / n_points;

  /* while delta > 455: delta /= 35 */
  for (k = 0; delta > ((BASE - TMIN) * TMAX) / 2; k += BASE) {
    delta /= (BASE - TMIN);
  }

  return k + (((BASE - TMIN + 1) * delta) / (delta + SKEW));
}

static char encode_digit(int c) {
  assert(c >= 0 && c <= BASE - TMIN);
  if (c > 25) {
    return c + 22; /* '0'..'9' */
  }
  else {
    return c + 'a'; /* 'a'..'z' */
  }
}

/* Encode as a generalized variable-length integer. Returns number of bytes written. */
static size_t encode_var_int(const size_t bias, const size_t delta, char *const dst, size_t dstlen) {
  size_t i, k, q, t;

  i = 0;
  k = BASE;
  q = delta;

  while (i < dstlen) {
    if (k <= bias) {
      t = TMIN;
    }
    else if (k >= bias + TMAX) {
      t = TMAX;
    }
    else {
      t = k - bias;
    }

    if (q < t) {
      break;
    }

    dst[i++] = encode_digit(t + (q - t) % (BASE - t));

    q = (q - t) / (BASE - t);
    k += BASE;
  }

  if (i < dstlen) {
    dst[i++] = encode_digit(q);
  }

  return i;
}

size_t punycode_encode(const uint32_t *const src, const size_t srclen, char *const dst, size_t *const dstlen) {
  size_t b, h;
  size_t delta, bias;
  size_t m, n;
  size_t si, di;

  for (si = 0, di = 0; si < srclen && di < *dstlen; si++) {
    if (src[si] < 128) {
      dst[di++] = src[si];
    }
  }

  b = h = di;

  /* Write out delimiter if any basic code points were processed. */
  if (di > 0 && di < *dstlen) {
    dst[di++] = '-';
  }

  n = INITIAL_N;
  bias = INITIAL_BIAS;
  delta = 0;

  for (; h < srclen && di < *dstlen; n++, delta++) {
    /* Find next smallest non-basic code point. */
    for (m = SIZE_MAX, si = 0; si < srclen; si++) {
      if (src[si] >= n && src[si] < m) {
        m = src[si];
      }
    }

    if ((m - n) > (SIZE_MAX - delta) / (h + 1)) {
      /* OVERFLOW */
      assert(0 && "OVERFLOW");
      goto fail;
    }

    delta += (m - n) * (h + 1);
    n = m;

    for (si = 0; si < srclen; si++) {
      if (src[si] < n) {
        if (++delta == 0) {
          /* OVERFLOW */
          assert(0 && "OVERFLOW");
          goto fail;
        }
      }
      else if (src[si] == n) {
        di += encode_var_int(bias, delta, &dst[di], *dstlen - di);
        bias = adapt_bias(delta, h + 1, h == b);
        delta = 0;
        h++;
      }
    }
  }

fail:
  /* Tell the caller how many bytes were written to the output buffer. */
  *dstlen = di;

  /* Return how many Unicode code points were converted. */
  return si;
}

size_t punycode_decode(const char *const src, const size_t srclen, uint32_t *const dst, size_t *const dstlen) {
  (void) src;
  (void) srclen;
  (void) dst;
  (void) dstlen;
  return 0;
}
