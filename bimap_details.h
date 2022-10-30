#pragma once

#include "intrusive_tree.h"

namespace details {

struct left_tag {};
struct right_tag {};

template <typename Key, typename Tag>
struct storage : public intrusive::node<Tag> {
  virtual Key const& get() const = 0;
  virtual ~storage() = default;
};

template <typename Key, typename Tag>
struct key_t : public storage<Key, Tag> {
  Key key;

  explicit key_t(Key&& key) : key(std::move(key)) {}

  Key const& get() const {
    return key;
  }
};

template <typename Key, typename Tag>
struct fake_key_t : public storage<Key, Tag> {
  Key const* key;

  explicit fake_key_t(Key const& key) : key(&key) {}
  explicit fake_key_t(std::nullptr_t) : key(nullptr) {}

  Key const& get() const {
    return *key;
  }
};

template <typename Base, typename Comparator, typename Tag>
struct comparator_t {
  Comparator comp;
  explicit comparator_t(Comparator comp) : comp(comp) {}
  bool operator()(const storage<Base, Tag>& a,
                  const storage<Base, Tag>& b) const {
    return comp(a.get(), b.get());
  }
};

template <typename Left, typename Right>
struct node_t : public key_t<Left, left_tag>, public key_t<Right, right_tag> {
  node_t(Left left, Right right)
      : key_t<Left, left_tag>(std::move(left)), key_t<Right, right_tag>(
                                                    std::move(right)) {}
};

template <typename Left, typename Right>
struct sentinel_t : public fake_key_t<Left, left_tag>,
                    public fake_key_t<Right, right_tag> {
  sentinel_t()
      : fake_key_t<Left, left_tag>(nullptr), fake_key_t<Right, right_tag>(
                                                 nullptr) {}
};

} // namespace details
