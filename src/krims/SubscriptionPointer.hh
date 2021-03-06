//
// Copyright (C) 2016-17 by the krims authors
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
#include "Subscribable.hh"
#include "TypeUtils/EnableIfLibrary.hh"
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace krims {

// Forward-declare Subscribable
class Subscribable;

template <typename T>
class SubscriptionPointer {
  static_assert(std::is_base_of<Subscribable, T>::value,
                "T must be a child class of Subscribable");

 public:
  /** The template parameter T, i.e. the type of the managed object */
  typedef T element_type;

  /** \brief Create an empty Subscription pointer pointing to no object at all
   *
   * Actual subscription to an object may be done using the reset function.
   *
   * \param subscriber_id Id used for the subscription.
   * */
  explicit SubscriptionPointer(const std::string& subscriber_id)
        : m_subscribed_obj_ptr(nullptr),
          m_subscriber_id_ptr(new std::string(subscriber_id)) {}

  /** Create a Subscription pointer from a Subscribable Class and an
   * identifier
   *
   * \param subscriber_id  identifier used for subscription
   * \param s object to subscribe to
   * */
  SubscriptionPointer(const std::string& subscriber_id, T& s)
        : SubscriptionPointer(subscriber_id) {
    register_at(&s);
  }

  /** Destructor */
  ~SubscriptionPointer() {
    register_at(nullptr);  // i.e. unregister us
  }

  /** Copy constructor */
  SubscriptionPointer(const SubscriptionPointer& other)
        : m_subscribed_obj_ptr(nullptr),
          m_subscriber_id_ptr(new std::string(*(other.m_subscriber_id_ptr))) {
    register_at(other.m_subscribed_obj_ptr);
  }

  /** \brief Implicit conversion from a different element type */
  template <typename U, typename = enable_if_t<std::is_convertible<U*, T*>::value>>
  SubscriptionPointer(const SubscriptionPointer<U>& other)
        : m_subscribed_obj_ptr(nullptr),
          m_subscriber_id_ptr(new std::string(other.subscriber_id())) {
    register_at(other.get());  // Register (here the ptr conversion happens)
  }

  /** Move constructor */
  SubscriptionPointer(SubscriptionPointer&& other)
        : m_subscribed_obj_ptr(other.m_subscribed_obj_ptr),
          m_subscriber_id_ptr(std::move(other.m_subscriber_id_ptr)) {
    other.m_subscribed_obj_ptr = nullptr;
  }

  /** Assignment operator */
  SubscriptionPointer& operator=(SubscriptionPointer other) {
#ifdef DEBUG
    register_at(nullptr);  // Unregister us first
#endif

    // Move the subscription of other over:
    m_subscribed_obj_ptr       = other.m_subscribed_obj_ptr;
    m_subscriber_id_ptr        = std::move(other.m_subscriber_id_ptr);
    other.m_subscribed_obj_ptr = nullptr;

    return *this;
  }

  /** \brief Reset the subscription and subscribe to a new object.
   *
   * Unsubscribes from the old object and subscribes to the new object
   * with the exact same subscriber id
   *
   * \param s The new object to subscribe to
   * */
  void reset(T& s) { register_at(&s); }

  /** \brief Reset the subscription and become empty */
  void reset() { register_at(nullptr); }

  /** \brief Check if this object is empty or not */
  explicit operator bool() const { return m_subscribed_obj_ptr != nullptr; }

  /** \brief Raw access to the inner pointer */
  T* get() const { return m_subscribed_obj_ptr; }

  /** Dereference object */
  T& operator*() const { return *m_subscribed_obj_ptr; }

  /** Dereference object member */
  T* operator->() const { return m_subscribed_obj_ptr; }

  /** Obtain the subscriber id provided upon construction. */
  const std::string& subscriber_id() const { return *m_subscriber_id_ptr; }

 private:
  /** Register at the given object */
  void register_at(T* object_ptr) {
#ifdef DEBUG
    if (m_subscribed_obj_ptr) {
      // Unregister subscription of this pointer from old object
      m_subscribed_obj_ptr->unsubscribe(m_subscriber_id_ptr);
      m_subscribed_obj_ptr = nullptr;
    }

    if (object_ptr) {
      // Register subscription at new object
      object_ptr->subscribe(m_subscriber_id_ptr);
      m_subscribed_obj_ptr = object_ptr;
    }
#else
    // Since we have no subscribing/unsubscribing business to do:
    m_subscribed_obj_ptr = object_ptr;
#endif  // DEBUG
  }

  /// Pointer to the actual object.
  T* m_subscribed_obj_ptr;

/** The pointer which is the unique identifier of this very pointer object
 *  Note that in DEBUG the actual pointer address is used as the identifier
 *  such that just barely using the string value is not enough
 */
#ifdef DEBUG
  std::shared_ptr<const std::string> m_subscriber_id_ptr;
#else
  // use unique_ptr in RELEASE mode since this guy is way faster.
  std::unique_ptr<const std::string> m_subscriber_id_ptr;
#endif
};

/**
 * \brief Convenience wrapper for making subscription pointers
 * and subscribing to objects
 *
 * When making subscriptions to a reference in this
 * way, the mechanism protects you from deallocating
 * the referenced object and leaving a dangling pointer
 * behind.
 *
 * It does *not* protect you from doing silly things to
 * the actual memory, however. So if you move the object
 * somewhere else, i.e. invalidate its memory while
 * still holding subscriptions to it, this mechanism
 * does not detect and complain about this.
 *
 * Similarly if you make changes to the object after
 * having made these subscriptions, these changes are
 * of course also seen from the Subscription pointers
 * which hold a subscription to said object.
 */
template <typename T>
inline SubscriptionPointer<T> make_subscription(T& object,
                                                const std::string& subscriber_id) {
  return SubscriptionPointer<T>(subscriber_id, object);
}

//
// == and != comparison operators:
//
/** Compare if the SubscriptionPointers point to the same object */
template <typename T>
inline bool operator==(const SubscriptionPointer<T>& lhs,
                       const SubscriptionPointer<T>& rhs) {
  return lhs.get() == rhs.get();
}

/** Compare if the SubscriptionPointers do not point to the same object */
template <typename T>
inline bool operator!=(const SubscriptionPointer<T>& lhs,
                       const SubscriptionPointer<T>& rhs) {
  return !operator==(lhs, rhs);
}

/** Compare if the Subscription pointer points to no object, i.e. stores a
 * nullptr */
template <typename T>
inline bool operator==(const SubscriptionPointer<T>& lhs, std::nullptr_t) {
  return lhs.get() == nullptr;
}

/** Compare if the Subscription pointer points to no object, i.e. stores a
 * nullptr */
template <typename T>
inline bool operator==(std::nullptr_t, const SubscriptionPointer<T>& rhs) {
  return rhs == nullptr;
}

/** Compare if the Subscription pointer does not point to no object, i.e. stores
 * no nullptr */
template <typename T>
inline bool operator!=(const SubscriptionPointer<T>& lhs, std::nullptr_t) {
  return !operator==(lhs, nullptr);
}

/** Compare if the Subscription pointer does not point to no object, i.e. stores
 * no nullptr */
template <typename T>
inline bool operator!=(std::nullptr_t, const SubscriptionPointer<T>& rhs) {
  return !operator==(nullptr, rhs);
}

}  // namespace krims
