//
// Copyright (C) 2016 by the krims authors
//
// This file is part of krims.
//
// krims is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// krims is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with krims. If not, see <http://www.gnu.org/licenses/>.
//

#pragma once
#include <algorithm>
#include <iterator>
#include <numeric>
#include <vector>
namespace krims {

template <typename RandomAccessIterator, typename Compare>
std::vector<size_t> argsort(const RandomAccessIterator first,
                            const RandomAccessIterator last, Compare cmp) {
  // Initialise index array
  std::vector<size_t> indices(last - first);
  std::iota(std::begin(indices), std::end(indices), 0);

  std::sort(std::begin(indices), std::end(indices), [&first, &cmp](size_t i1, size_t i2) {
    return cmp(*(first + i1), *(first + i2));
  });

  return indices;
}

template <typename RandomAccessIterator>
std::vector<size_t> argsort(const RandomAccessIterator first,
                            const RandomAccessIterator last) {
  return argsort(first, last, std::less<decltype(*first)>{});
}
}  // krims
