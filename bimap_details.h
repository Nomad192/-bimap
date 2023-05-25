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

template <typename Left, typename Right>
struct node_t : public key_t<Left, left_tag>, public key_t<Right, right_tag> {
  node_t(Left left, Right right)
      : key_t<Left, left_tag>(std::move(left)), key_t<Right, right_tag>(
                                                    std::move(right)) {}
};

} // namespace details
