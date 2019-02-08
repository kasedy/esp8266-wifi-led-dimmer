#pragma once

#include <stddef.h>
#include <vector>
#include <algorithm>

/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define PPCAT_NX(A, B) A ## B

/*
 * Concatenate preprocessor tokens A and B after macro-expanding them.
 */
#define PPCAT(A, B) PPCAT_NX(A, B)

/*
 * Turn A into a string literal without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define STRINGIZE_NX(A) #A

/*
 * Turn A into a string literal after macro-expanding it.
 */
#define STRINGIZE(A) STRINGIZE_NX(A)

template<class T, size_t N>
constexpr size_t len(T (&)[N]) { return N; }

template<class T>
T difference(T a, T b) {
  return (a > b) ? (a - b) : (b - a);
}

template <typename T> 
int sgn(T a, T b) {
  return (b < a) - (a < b);
}

template<typename T, typename Predicate>
std::vector<T> filter_vector(const std::vector<T> &input, Predicate predicate) {
  int size = std::count_if(input.begin(), input.end(), predicate);
  std::vector<T> result(size);
  std::copy_if(input.begin(), input.end(), result.begin(), predicate);
  return result;
}
