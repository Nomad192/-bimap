#pragma once

namespace intrusive {

struct default_tag;

template <typename Tag = default_tag>
struct node {
  node* parent = nullptr;
  node* left = nullptr;
  node* right = nullptr;

  node() = default;
  explicit node(node* parent) : parent(parent) {}
  ~node() {
    unlink();
  }

  bool is_right() {
    return parent->right == this;
  }

  void relink_parent(node* set) {
    if (is_right())
      parent->right = set;
    else
      parent->left = set;
    if (set)
      set->parent = parent;
  }

  void repair_childs() {
    if (left)
      left->parent = this;
    if (right)
      right->parent = this;
  }

  void unlink() {
    if (parent) {
      if (right != nullptr && left == nullptr) {
        relink_parent(right);
        right = nullptr;
      } else if (right == nullptr && left != nullptr) {
        relink_parent(left);
        left = nullptr;
      } else if (right != nullptr && left != nullptr) {
        node* next = min_node(right);
        next->unlink();
        this->swap(*next);
      } else {
        relink_parent(nullptr);
      }
      parent = nullptr;
    }
  }

  static node* min_node(node* cur) {
    while (cur->left)
      cur = cur->left;
    return cur;
  }
  static node* max_node(node* cur) {
    while (cur->right)
      cur = cur->right;
    return cur;
  }
  static node* next_node(node* cur) {
    if (cur->right)
      return min_node(cur->right);
    node* next = cur->parent;
    while (next->parent != nullptr && cur == next->right) {
      cur = next;
      next = next->parent;
    }
    return next;
  }
  static node* prev_node(node* cur) {
    if (cur->left)
      return max_node(cur->left);
    node* prev = cur->parent;
    while (prev->parent != nullptr && cur == prev->left) {
      cur = prev;
      prev = prev->parent;
    }
    return prev;
  }

  node* next() {
    return next_node(this);
  }

  node* prev() {
    return prev_node(this);
  }

  void swap(node& other) {
    if (other.parent)
      other.relink_parent(this);
    if (parent)
      relink_parent(&other);

    std::swap(parent, other.parent);

    std::swap(left, other.left);
    std::swap(right, other.right);
    repair_childs();
    other.repair_childs();
  }

  void simple_swap(node& other) {
    std::swap(parent, other.parent);
    std::swap(left, other.left);
    std::swap(right, other.right);
  }
};
} // namespace intrusive
