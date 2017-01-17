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
#include "ExceptionSystem.hh"
#include "SubscriptionPointer.hh"
#include "demangle.hh"
#include <algorithm>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

namespace krims {

// forwand declare SubscriptionPointer
template <typename Subscribable>
class SubscriptionPointer;

/** Class which handles subscribtions from a subscription pointer
 * If upon deletion still subscriptions exist, it throws an exception in debug
 * mode
 */
class Subscribable {
#ifdef DEBUG
  // Declare SubscriptionPointer as friend.
  template <typename T>
  friend class ::krims::SubscriptionPointer;
#endif

 public:
  /** A swap function for Subscribables */
  friend void swap(Subscribable&, Subscribable&) {
    // do nothing since the pointers that point to first,
    // still point to the first object and those which
    // point to second object still point to the second.
  }

  //
  // Exception declarations
  //

  /** Exception to indicate that Subscribable is still used */
  DefException3(ExcStillUsed, std::string, size_t, std::string,
                << "Object of type \"" << arg1 << "\" is still used by " << arg2
                << " other objects, which are (from new to old): " << arg3);

  /** Exception to indicate that Subscriber is not known. */
  DefException2(ExcUnknownSubscriberId, std::string, std::string,
                << "No subscriber with identifier \"" << arg1
                << "\" is known to have subscribed to the class " << arg2 << ".");

  //
  // Constructor, destructor and assignment
  //

  /** Check if all subscriptions have been removed
   *
   * @note This function is empty unless we are in DEBUG mode
   */
  virtual ~Subscribable() {
#ifdef DEBUG
    assert_no_subscriptions();
#endif
  }

  /** Default constructor */
  Subscribable() = default;

  /** Default move constructor */
  explicit Subscribable(Subscribable&& other) {
#ifdef DEBUG
    // check that other has no subscriptions
    // if it does have subscriptions, raise an
    // Exception there:
    other.assert_no_subscriptions();
#endif
  }

  /** Copy constructor */
  explicit Subscribable(const Subscribable&) {
    // Since copies are different objects,
    // do not copy the list of subscribers
    // or anything else
  }

  /** Copy assignment operator */
  Subscribable& operator=(const Subscribable&) {
    // All pointers to this object stay intact, so there is no reason
    // to do anything here.
    return *this;
  }

  /** Move assignment operator */
  Subscribable& operator=(Subscribable&&) {
    // All pointers to this object stay intact, so there is no reason
    // to do anything here.
    return *this;
  }

  //
  // Access subscriptions
  //
  /** Return the current number of subscriptions to this object.
   *
   * @note If we are not in DEBUG mode this count is always zero.
   * */
  size_t n_subscriptions() const {
#ifdef DEBUG
    // Note: Getting the size of a list in C++11 is constant time!
    return m_subscribers.size();
#else
    // return constant 0
    return 0;
#endif
  }

  /** Return the current subscriptions to this object.
   * The list contains a *copy* of the identification strings passed
   * on the subscribe call.
   * The first object is the object which has most recently subscribed
   * and conversely the last object is the oldest object which has still
   * not cancelled its subscription to this Subscribable.
   *
   * @note If we are in RELEASE mode, this list will always be empty
   */
  std::vector<std::string> subscribers() const {
#ifdef DEBUG
    std::vector<std::string> v(m_subscribers.size());
    std::transform(std::begin(m_subscribers), std::end(m_subscribers), std::begin(v),
                   [](const std::shared_ptr<const std::string>& p) { return *p; });
    return v;
#else
    return std::vector<std::string>{};
#endif
  }

 protected:
#ifdef DEBUG
  /** Assert that this has no subscriptions made to it.
   * If this is not the case, than abort the program via
   * assert_abort.
   *
   * @note In RELEASE mode this function does not exist.
   */
  void assert_no_subscriptions() const {
    if (m_subscribers.size() > 0) {
      // build the string of subscribing objects
      std::stringstream s;
      for (auto& p : m_subscribers) {
        s << " " << *p;
      }

      // Raise the exception
      // Note: There is no reason to continue here since we will anyways
      //       have dangling pointers in the SubscriptionPointer classes
      //       after this has occurred. There is no way we can get out of
      //       this gracefully.
      assert_abort(false, ExcStillUsed(m_classname, m_subscribers.size(), s.str()));
    }
  }
#endif

 private:
#ifdef DEBUG
  //
  // Deal with subscriptions:
  //

  /** Remove a subscription.
   *
   * @param id Reference to the same string object which was used upon
   *           subscription
   *
   * @note Does only exist in DEBUG mode
   * */
  void unsubscribe(const std::shared_ptr<const std::string>& id_ptr) const {
    for (auto it = std::begin(m_subscribers); it != std::end(m_subscribers); ++it) {
      // check if the pointers agree
      if (it->get() == id_ptr.get()) {
        // TODO This part is not thread-safe
        //
        //      Have a mutex to guard the erase
        m_subscribers.erase(it);
        return;
      }
    }
    assert_dbg(false, ExcUnknownSubscriberId(*id_ptr, m_classname));
  }

  /** Get a subscription
   *
   * @param id The id to print if the subscription is not removed properly
   *           before deletion
   *
   * @note Does only exist in DEBUG mode
   * */
  void subscribe(const std::shared_ptr<const std::string>& id_ptr) const {
    // TODO This part is not thread-safe
    //
    //      Have a mutex to guard the push and the actual setting
    //      of the classname.

    // Set classname here, since this is actually executed by the
    // precise object we subscribe to and not the generic Subscribable
    // class. So here we have the "proper" type available in this.
    const std::string expected_classname = demangled_string(typeid(*this).name());
    if (m_classname != expected_classname) {
      m_classname = expected_classname;
    }

    m_subscribers.push_front(id_ptr);
  }

  /** List to contain the pointer of string object, which are passed
   *  on subscription.
   *
   * Marked as mutable in order to allow to subscribe / unsubscribe from
   * const references as well.
   */
  mutable std::list<std::shared_ptr<const std::string>> m_subscribers;

  /**
   * Name of the actual child Subscribable class
   * Set on call of the subscribe function
   *
   * Marked as mutable in order to allow to subscribe / unsubscribe from
   * const references as well.
   * */
  mutable std::string m_classname{"(unknown)"};
#endif
};

}  // namespace krims
