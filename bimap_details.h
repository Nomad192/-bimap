#pragma once

#include "intrusive_tree.h"

namespace details {

struct left_tag {};
struct right_tag {};

template <typename Key, typename Tag>
struct key_t : public intrusive::node<Tag> {
  Key key;

  explicit key_t(Key&& key) : key(std::move(key)) {}
};

template <typename Base, typename Comparator, typename Tag>
struct comparator_t : Comparator {

  template <typename Base1, typename Base2>
  bool operator()(const key_t<Base1, Tag>& a,
                  const key_t<Base2, Tag>& b) const {
    return Comparator::operator()(a.key, b.key);
  }
};

template <typename Left, typename Right>
struct node_t : public key_t<Left, left_tag>, public key_t<Right, right_tag> {
  node_t(Left left, Right right)
      : key_t<Left, left_tag>(std::move(left)), key_t<Right, right_tag>(
                                                    std::move(right)) {}
};

struct sentinel_t : public intrusive::node<left_tag>,
                    public intrusive::node<right_tag> {};

} // namespace details
