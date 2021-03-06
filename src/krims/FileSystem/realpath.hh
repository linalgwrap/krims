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
#include <krims/ExceptionSystem.hh>
#include <string>

namespace krims {

DefException2(ExcRealpathError, int, std::string,
              << "An error in realpath occurred: " << arg2 << " (Error code " << arg1
              << ")");

/** Return the realpath of an input path */
std::string realpath(const std::string& path);

}  // namespace krims
