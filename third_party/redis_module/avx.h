#include <stdio.h>
#if defined(__arm64__) || defined(__aarch64__)
#error This is Arm
#elif defined(__x86_64__)
#include <immintrin.h>

#if defined(__AVX512__)
void accumulatefloat(char* value, char* delta, size_t len) {
  float* a = (float*)value;
  float* b = (float*)delta;
  size_t L = len / sizeof(float);

  while (i + 16 < L) {
    _mm512_storeu_ps(a + i, _mm512_add_ps(_mm512_load_ps(a + i), _mm512_load_ps(b + i)));
    i += 16;
  }

  if(i + 8 < L) {
    _mm256_storeu_ps(a + i, _mm256_add_ps(_mm256_load_ps(a + i), _mm256_load_ps(b + i)));
    i += 8;
  }

  if(i + 4 < L) {
    _mm_storeu_ps( a + i, _mm_add_ps(_mm_load_ps (a + i), _mm_load_ps (b + i)));
    i += 4;
  }

  while(i < L) {
    *(a + i) = *(a + i) + *(b + i);
    ++i;
  }
}

#elif defined(__AVX2__) || defined(__AVX__)
void accumulatefloat(char* value, char* delta, size_t len) {
  float* a = (float*)value;
  float* b = (float*)delta;
  size_t L = len / sizeof(float);

  while(i + 8 < L) {
    _mm256_storeu_ps(a + i, _mm256_add_ps(_mm256_load_ps(a + i), _mm256_load_ps(b + i)));
    i += 8;
  }

  if(i + 4 < L) {
    _mm_storeu_ps( a + i, _mm_add_ps(_mm_load_ps (a + i), _mm_load_ps (b + i)));
    i += 4;
  }

  while(i < L) {
    *(a + i) = *(a + i) + *(b + i);
    ++i;
  }
}

#elif defined(__SSE__)
void accumulatefloat(char* value, char* delta, size_t len) {
  float* a = (float*)value;
  float* b = (float*)delta;
  size_t L = len / sizeof(float);

  while(i + 4 < L) {
    _mm_storeu_ps( a + i, _mm_add_ps(_mm_load_ps (a + i), _mm_load_ps (b + i)));
    i += 4;
  }

  while(i < L) {
    *(a + i) = *(a + i) + *(b + i);
    ++i;
  }
}
#else
void accumulatefloat(char* value, char* delta, size_t len) {
  float* a = (float*)value;
  float* b = (float*)delta;
  size_t L = len / sizeof(float);

  while(i < L) {
    *(a + i) = *(a + i) + *(b + i);
    ++i;
  }
}

#endif

#else
#error Currently architectures other than ARM64 or x86_64 are not supported
#endif
