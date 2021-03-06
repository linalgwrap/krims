## ---------------------------------------------------------------------
##
## Copyright (C) 2016-17 by the krims authors
##
## This file is part of krims.
##
## krims is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published
## by the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## krims is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with krims. If not, see <http://www.gnu.org/licenses/>.
##
## ---------------------------------------------------------------------

#
# set the source files.
#
set(KRIMS_SOURCES
	demangle.cc
	DataFiles/FindDataFile.cc
	DataFiles/FloatingPointType.cc
	DataFiles/ieee_convert.cc
	DataFiles/read_binary.cc
	ExceptionSystem/addr2line.cc
	ExceptionSystem/Backtrace.cc
	ExceptionSystem/ExceptionBase.cc
	ExceptionSystem/ExceptionSystem.cc
	FileSystem/basename.cc
	FileSystem/dirname.cc
	FileSystem/path_exists.cc
	FileSystem/realpath.cc
	FileSystem/splitext.cc
	GenMap.cc
	NumComp/NumCompConstants.cc
	version.cc
)

# Write the config file config.hh (into the binary dir)
configure_file("config.hh.in" "config.hh")

# Add the libraries for Debug and Release builds:
drb_add_library(krims
	DBGSUFFIX  ".g"
	FILES ${KRIMS_SOURCES}
)

drb_set_target_properties(
	ALL krims
	PROPERTIES
	VERSION "${PROJECT_VERSION}"
)

drb_target_link_libraries(ALL     krims ${KRIMS_DEPENDENCIES})
drb_target_link_libraries(DEBUG   krims ${KRIMS_DEPENDENCIES_DEBUG})
drb_target_link_libraries(RELEASE krims ${KRIMS_DEPENDENCIES_RELEASE})

# set the installation property for these targets:
install(TARGETS ${krims_TARGETS}
	EXPORT KrimsTargets
	DESTINATION lib
	COMPONENT library
)

# install the includes:
install(DIRECTORY .
	DESTINATION "include/krims"
	COMPONENT devel
	FILES_MATCHING PATTERN "*.hh"
)
install(FILES "${krims_BINARY_DIR}/src/krims/config.hh"
	DESTINATION "include/krims"
	COMPONENT devel
)

# Export the target specifications for krims.
export(EXPORT KrimsTargets
	FILE "${krims_BINARY_DIR}/krimsTargets.cmake"
	NAMESPACE Upstream::
)

# and install them appropriately:
install(EXPORT KrimsTargets
	FILE "krimsTargets.cmake"
	NAMESPACE Upstream::
	DESTINATION "share/cmake/krims"
)

