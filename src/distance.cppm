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
#include <vector>

export module wordmeter:distance;

export namespace wm {
std::vector<std::size_t> calcDist(std::vector<std::size_t> const &posA,
                                  std::vector<std::size_t> const &posB,
                                  std::size_t const totalWordCount) {
  auto const p = [](auto const a, auto const b) { return a > b; };
  std::vector<std::size_t> out{};
  out.reserve(posA.size());

  for (auto nearestA = posA.begin(); nearestA != posA.end();) {
    auto nearestB = std::upper_bound(posB.begin(), posB.end(), *nearestA);

    if (nearestB == posB.end()) {
      auto rnearestA =
          std::upper_bound(posA.rbegin(), posA.rend(), posB.front(), p);
      if (rnearestA == posA.rend())
        out.insert(out.end(), totalWordCount - posA.back() + posB.front());
      return out;
    }

    auto rnearestA = std::upper_bound(posA.rbegin(), posA.rend(), *nearestB, p);

    if (rnearestA.base() > nearestA)
      nearestA = rnearestA.base();
    else
      ++nearestA;

    out.insert(out.end(), *nearestB - *rnearestA);
  }

  return out;
}
} // namespace wm
