/* Copyright (c) 2026 unixdev73@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

module;

#include <algorithm>

export module wordmeter:distance;

export namespace wm {
template <template <typename> typename C>
C<std::size_t> calcDist(C<std::size_t> const &posA,
                        C<std::size_t> const &posB,
                        std::size_t const totalWordCount) {
  if (posA.size() == 1 && posB.size() == 1 && *posA.begin() == *posB.begin())
    return C<std::size_t>{totalWordCount - 1};

  C<std::size_t> out{};
  out.reserve(posA.size());

  for (std::size_t i = 0; i < posA.size(); ++i) {
    bool loopAround = false;
    auto nearestB = std::upper_bound(posB.begin(), posB.end(), posA.at(i));
    if (nearestB == posB.end()) {
      nearestB = posB.begin();
      loopAround = true;
    }

    auto nearestA = posA.begin();
    if (loopAround)
      nearestA = --(posA.end());
    else
      nearestA =
          std::prev(std::lower_bound(posA.begin(), posA.end(), *nearestB));

    i = nearestA - posA.begin();

    if (*nearestA == *nearestB)
      continue;

    if (loopAround)
      out.push_back(totalWordCount - *nearestA + *nearestB);
    else
      out.push_back(*nearestB - *nearestA);
  }

  return out;
}
} // namespace wm
