#pragma once

#include <algorithm>

#include "intrusive_node.h"

namespace intrusive {

template <typename T, typename Compare, typename Tag = default_tag>
class intrusive_tree : public Compare {
  using node_t = node<Tag>;
  static_assert(std::is_convertible_v<T*, node_t*>, "invalid value type");

  node_t* sentinel;

public:
  explicit intrusive_tree(node_t* sentinel, Compare compare = Compare{})
      : sentinel(sentinel), Compare(std::move(compare)) {}

  intrusive_tree(intrusive_tree const& other) = delete;
  intrusive_tree(intrusive_tree&& other) = delete;

  intrusive_tree& operator=(intrusive_tree const& other) {
    if (this != &other)
      Tree(other).swap(*this);
    return *this;
  }

  intrusive_tree& operator=(intrusive_tree&& other) {
    if (this != &other)
      Tree(std::move(other)).swap(*this);
    return *this;
  }

  void swap(intrusive_tree& other) {
    std::swap(this->comp, other.comp);
    sentinel->swap(*other.sentinel);
  }

  bool empty() const {
    return sentinel->left == nullptr;
  }

private:
  template <typename iT>
  class inorder_iterator {
  private:
    node_t* cur = nullptr;

    template <typename tT, typename tCompare, typename tTag>
    friend class intrusive_tree;

    template <typename tT, typename tCompare, typename tTag>
    static inorder_iterator
    begin_iter(const intrusive_tree<tT, tCompare, tTag>* tree) {
      return (node_t::min_node(tree->sentinel));
    }

    template <typename tT, typename tCompare, typename tTag>
    static inorder_iterator
    end_iter(const intrusive_tree<tT, tCompare, tTag>* tree) {
      return (tree->sentinel);
    }

  public:
    inorder_iterator(node_t* cur) : cur(cur) {}

    using difference_type = ptrdiff_t;
    using value_type = iT;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;

    inorder_iterator() = default;

    template <typename eq_iT>
    bool operator==(inorder_iterator<eq_iT> const& other) const {
      return cur == other.cur;
    }

    template <typename eq_iT>
    bool operator!=(inorder_iterator<eq_iT> const& other) const {
      return cur != other.cur;
    }

    reference operator*() const {
      return *(static_cast<iT*>(cur));
    }

    pointer operator->() const {
      return (static_cast<iT*>(cur));
    }

    inorder_iterator& operator++() {
      cur = cur->next();
      return *this;
    }

    inorder_iterator& operator--() {
      cur = cur->prev();
      return *this;
    }

    inorder_iterator operator++(int) {
      inorder_iterator res(*this);
      ++(*this);
      return res;
    }

    inorder_iterator operator--(int) {
      inorder_iterator res(*this);
      --(*this);
      return res;
    }
  };

public:
  using iterator = inorder_iterator<T>;
  using const_iterator = inorder_iterator<const T>;

  iterator begin() const {
    return iterator::template begin_iter<T, Compare, Tag>(this);
  }

  iterator end() const {
    return iterator::template end_iter<T, Compare, Tag>(this);
  }

  static const T* make_p(node_t* p) {
    return static_cast<T*>(p);
  }

  static const T& make_r(node_t& p) {
    return static_cast<T&>(p);
  }

private:
  //enum class find_result { THERE_IS, ADD_RIGHT, ADD_LEFT };
  struct find_result
  {
    enum { THERE_IS, ADD_RIGHT, ADD_LEFT } flag;
    node_t *node;
  };
  find_result find_with_result(const T& data) const {
    find_result res = {find_result::ADD_LEFT, sentinel};
    if (sentinel->left == nullptr)
      return res;

    node_t* cur = sentinel->left;
    while (cur != nullptr) {
      if (Compare::operator()(make_r(*cur), data)) {
        if (cur->right)
          cur = cur->right;
        else {
          res.flag = find_result::ADD_RIGHT;
          break;
        }
      } else if (Compare::operator()(data, make_r(*cur))) {
        if (cur->left)
          cur = cur->left;
        else {
          res.flag = find_result::ADD_LEFT;
          break;
        }
      } else {
        res.flag = find_result::THERE_IS;
        break;
      }
    }
    res.node = cur;
    return res;
  }

public:
  iterator find(const T& x) const {
    find_result res = find_with_result(x);
    if (res.flag == find_result::THERE_IS)
      return (res.node);

    return end();
  }

  iterator lower_bound(const T& x) const {
    find_result res = find_with_result(x);
    if (res.flag == find_result::THERE_IS || res.flag == find_result::ADD_LEFT)
      return (res.node);
    return (res.node->next());
  }

  iterator upper_bound(const T& x) const {
    find_result res = find_with_result(x);
    if (res.flag == find_result::ADD_LEFT)
      return (res.node);
    return (res.node->next());
  }

  iterator insert(T& data) {
    find_result res = find_with_result(data);
    if (res.flag == find_result::THERE_IS)
      return end();

    static_cast<node_t*>(&data)->parent = res.node;
    if (res.flag == find_result::ADD_LEFT) {
      res.node->left = &data;
      return (res.node->left);
    } else { /// res == ADD_RIGHT)
      res.node->right = &data;
      return (res.node->right);
    }
  }

  iterator remove(iterator it) {
    iterator it_next(it.cur->next());
    it.cur->unlink();
    return it_next;
  }

  iterator remove(T& data) {
    find_result res = find_with_result(data);
    if (res.flag == find_result::THERE_IS)
      return end();
    return remove(iterator(res.node));
  }
};
} // namespace intrusive
