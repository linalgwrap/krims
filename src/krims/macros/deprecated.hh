//
// Copyright (C) 2017 by the krims authors
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
#include "krims/config.hh"

namespace krims {

//! Mark something as deprecated
#ifdef KRIMS_HAVE_CXX14
#define KRIMS_DEPRECATED(msg) [[deprecated(#msg)]]
#else  // KRIMS_HAVE_CXX14
#define KRIMS_DEPRECATED(msg) __attribute__((deprecated))
#endif  // KRIMS_HAVE_CXX14

}  // namespace krims
