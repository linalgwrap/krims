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

#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <krims/exceptions.hh>

int main(int argc, char* const argv[]) {
  // Make sure that the program does not get aborted,
  // but all krims exceptions throw instead.
  krims::exceptions::AssertDbgEffect::set_throw();

  // Run catch:
  int result = Catch::Session().run(argc, argv);
  return result;
}
