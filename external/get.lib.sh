#!/bin/sh
## ---------------------------------------------------------------------
##
## Copyright (C) 2016 by the krims authors
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

# Meaning of the exit codes:
#
# 1      Normal error updating the repository
# 128    Some internal command has failed

is_host_up() {
	# Check whether the host is up
	ping  -c 1 "$@" > /dev/null 2>&1
}

is_url_available() {
	# The url to check
	I_URL="$1"

	# Extract hostname from FROM url
	I_FROMHOST=`echo "$I_URL" | cut -d "/" -f 3 | sed 's/^.*@//; s/:.*$//'`
	if [ -z "$I_FROMHOST" ]; then
		echo "Invalid url given to is_url_available" >&2
		exit 1
	fi

	is_host_up "$I_FROMHOST"
}

mark_update_done() {
	# Flag that the update has been done right now
	#
	# The string used to indicate the repository
	I_WHAT="$1"
	mkdir -p .last_pull || exit 128
	touch .last_pull/"$I_WHAT" || exit 128
}

last_modification_time() {
	# Get the time when the last modification was done
	# in nanoseconds since the epoch
	#
	# The file to determine the modification time of
	I_FILE="$1"

	if [ ! -f "$I_FILE" ]; then
		echo "No valid file: $1" >&2
		return 1
	fi

	UNAME=`uname -s`
	case $UNAME in
		Linux)     # All kind of linuxes
			date +%s -r "$I_FILE" || exit 128
			;;
		Darwin)    # macOS and MacOS X
			# TODO untested
			stat -f %m -t %s "$I_FILE" || exit 128
			;;
		FreeBSD)
			# TODO untested
			date +%s -r "$I_FILE" || exit 128
			;;
		NetBSD)
			# TODO untested
			stat -f %m -t %s "$I_FILE" || exit 128
			;;
		OpenBSD)
			stat -f %m -t %s "$I_FILE" || exit 128
			;;
		*)
			echo "WARNING: Unknown OS ... falling back to default" >&2
			date +%s -r "$I_FILE" || exit 128
	esac
	return 0
}

needs_update() {
	# Check the last pull/update time and see if we need update
	#
	# The string used to indicate the last checkout
	I_WHAT="$1"
	#
	# The requested update interval
	I_INTERVAL="$2"
	mkdir -p .last_pull

	if echo "$I_INTERVAL" | grep -q '[0-9]'; then
		:
	else
		echo "Interval '$I_INTERVAL' contains invalid characters:" >&2
		echo "Only number of seconds allowed." >&2
		exit 1
	fi

	# If there is no last pull file, we need to pull
	[ -f ".last_pull/$I_WHAT" ] || return 0

	# The last time an update was done:
	I_TIME_LAST_PULL=`last_modification_time ".last_pull/$I_WHAT"`

	# Add the allowed interval:
	I_BEAT=`expr $I_TIME_LAST_PULL + $I_INTERVAL` || exit 128

	# If result is still smaller than now, we need an update.
	I_NOW=`date +%s`
	[ "$I_BEAT" -lt "$I_NOW" ]
}

checkout_repo () {
	# where to checkout stuff trom
	I_FROM="$1"
	# what repo to checkout
	I_WHAT="$2"
	# the file to check after checkout
	I_CHECKFILE="$3"
	# which branch to checkout
	I_BRANCH="$4"

	# No branch specified
	if [ "$I_BRANCH" ]; then
		I_EXTRA=" (branch: $I_BRANCH)"
		I_ARGS="--branch $I_BRANCH"
	fi

	echo "-- Cloning  $I_WHAT${I_EXTRA}  from git"
	if ! is_url_available "$I_FROM"; then
		echo "   Could not reach url \"$I_FROM\" ... failing." >&2
		return 1
	fi

	git clone $I_ARGS --recursive "$I_FROM" "$I_WHAT" || return 1
	if [ -f "$I_WHAT/$I_CHECKFILE" ]; then
		mark_update_done "$I_WHAT" || return 1
		return 0
	else
		echo "   Error ... file $I_CHECKFILE not found after checkout" >&2
		return 1
	fi

}

pull_repo() {
	# where to checkout stuff trom
	I_FROM="$1"
	# what repo to checkout
	I_WHAT="$2"
	# The interval after which to perform a pull
	I_INTERVAL="$3"

	if ! needs_update "$I_WHAT" "$I_INTERVAL"; then
		return 0
	fi

	echo "-- Updating  $I_WHAT  from git"
	if is_url_available "$I_FROM"; then
		(
			cd "$I_WHAT" && \
			git pull && \
			git submodule update --init --recursive
		) && \
		mark_update_done "$I_WHAT"
	else
		echo "   No network ... we stay with old version."
	fi
	return 0
}

update_repo() {
	# where to checkout stuff trom
	I_FROM="$1"
	# Name of repo to checkout (i.e. folder to checkout into)
	I_WHAT="$2"
	# File to check inside repo (realative url)
	# if it exists checkout was successful
	I_CHECKFILE="$3"
	# The interval after which to perform a pull
	I_INTERVAL="$4"
	# Optional branch to checkout (if not default)
	I_BRANCH="$5"

	if [ -z "$I_FROM" ]; then
		echo "Please define I_FROM (1st arg) to a non-zero value"
		exit 1
	fi
	if [ -z "$I_WHAT" ]; then
		echo "Please define I_WHAT (2nd arg) to a non-zero value"
		exit 1
	fi
	if [ -z "$I_CHECKFILE" ]; then
		echo "Please define I_CHECKFILE (3rd arg) to a non-zero value"
		exit 1
	fi
	if [ -z "$I_INTERVAL" ]; then
		echo "Please define I_INTERVAL (4th arg) to a non-zero value"
		exit 1
	fi

	if [ -f "$I_WHAT/$I_CHECKFILE" ]; then
		pull_repo "$I_FROM" "$I_WHAT" "$I_INTERVAL"
		return $?
	else
		checkout_repo "$I_FROM" "$I_WHAT" "$I_CHECKFILE" "$I_BRANCH"
		return $?
	fi
}
