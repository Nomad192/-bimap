#pragma once
#include <algorithm>

namespace intrusive {

struct default_tag;

template <typename Key, typename Tag = default_tag>
struct node {
  node* parent = nullptr;
  node* left = nullptr;
  node* right = nullptr;

  node() = default;
  explicit node(node* parent) : parent(parent) {}
  ~node(){
    unlink();
  }

  inline bool is_right(){
    return parent->right == this;
  }

  inline void relink_parent(node* set)
  {
    if (is_right())
      parent->right = set;
    else
      parent->left = set;
    if(set)
      set->parent = parent;
  }

  inline void repair_childs()
  {
    if(left)
      left->parent = this;
    if(right)
      right->parent = this;
  }

  void unlink()
  {
    if (parent) {
      if(right != nullptr && left == nullptr)
      {
        relink_parent(right);
        right = nullptr;
      }
      else if(right == nullptr && left != nullptr)
      {
        relink_parent(left);
        left = nullptr;
      }
      else if(right != nullptr && left != nullptr)
      {
        node *next = min_node(right);
        next->unlink();
        relink_parent(next);
        this->swap(*next);
        next->repair_childs();


//        node *next = right;
//
//        while (next->left)
//          next = next->left;
//
//        next->unlink_parent(next->right);
//        next->right = nullptr;
//        next->parent = nullptr;
//
//        next->parent = parent;
//        next->right = right;
//        next->left = left;
//
//        unlink_parent(next);
//
//        if(next->left)
//          next->left->parent = next;
//        if(next->right)
//          next->right->parent = next;
//        right = nullptr;
//        left = nullptr;
      }
      else
      {
        relink_parent(nullptr);
      }
      parent = nullptr;
    }
  }

  static node* min_node(node* cur)
  {
    while (cur->left)
      cur = cur->left;
    return cur;
  }
  static node* max_node(node* cur)
  {
    while (cur->right)
      cur = cur->right;
    return cur;
  }
  static node* next_node(node* cur)
  {
    if (cur->right)
      return min_node(cur->right);
    node* next = cur->parent;
    while (next->parent != nullptr && cur == next->right) {
      cur = next;
      next = next->parent;
    }
    return next;
  }
  static node* prev_node(node* cur)
  {
    if (cur->left)
      return max_node(cur->left);
    node* prev = cur->parent;
    while (prev->parent != nullptr && cur == prev->left) {
      cur = prev;
      prev = prev->parent;
    }
    return prev;
  }

  node* next()
  {
    return next_node(this);
  }

  node* prev()
  {
    return prev_node(this);
  }

  void swap(node& other)
  {
    std::swap(parent, other.parent);
    std::swap(left, other.left);
    std::swap(right, other.right);
  }
};

//template <typename Tag = default_tag>
//struct node : public node_base {};

template <typename T, typename Key, typename Compare, typename Tag = default_tag> ////11111111
class intrusive_tree {
  using node_t = node<Key, Tag>;
  static_assert(std::is_convertible_v<T *, node_t *>, "invalid value type");

  node_t *sentinel;
  Compare comp;
//
//  static inline node_t *make_p_node_t(node_base *p)
//  {
//    return static_cast<node_t *>(p);
//  }

public:
  intrusive_tree(node_t *sentinel, Compare compare = Compare{}) : sentinel(sentinel), comp(std::move(compare)) {
  }

private:
  template <typename iT>
  class inorder_iterator {
  private:
    node_t* cur = nullptr;

    template <typename tT, typename tKey, typename tCompare, typename tTag>
    friend class intrusive_tree;

    template <typename tT, typename tKey, typename tCompare, typename tTag>
    static inorder_iterator begin_iter(const intrusive_tree<tT, tKey, tCompare, tTag>* tree) {
      //if (tree->sentinel->left)
        return (node_t::min_node(tree->sentinel)); ///!!!!!!!!!!!!!!!!!!!!

      //return end_iter(tree);
    }

    template <typename tT, typename tKey, typename tCompare, typename tTag>
    static inorder_iterator end_iter(const intrusive_tree<tT, tKey, tCompare, tTag>* tree) {
      return (tree->sentinel);
    }

  public:
    inorder_iterator(decltype(cur) cur) : cur(cur) {}

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

//    bool is_end() const {
//      return cur->parent == nullptr;
//    }

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
    return iterator::template begin_iter<T, Key, Compare, Tag>(this);
  }

  iterator end() const {
    return iterator::template end_iter<T, Key, Compare, Tag>(this);
  }

//  const_iterator begin() const {
//    return const_iterator::begin_iter(this);
//  }
//
//  const_iterator end() const {
//    return const_iterator::end_iter(this);
//  }

  inline static const T* make_p(node_t *p)
  {
    return static_cast<T *>(p);
  }

//  inline static const T* make_p(const node_t *p)
//  {
//    return static_cast<T *>(p);
//  }

  inline static const T& make_r(node_t &p)
  {
    return static_cast<T &>(p);
  }

//  inline static const T& make_r(const node_t &p)
//  {
//    return static_cast<T &>(p);
//  }

private:
  enum find_result
  {
    THERE_IS,
    ADD_RIGHT,
    ADD_LEFT
  };
  node_t *find(const T& data, find_result &res) const
  {
    res = ADD_LEFT;
    if(sentinel->left == nullptr)
      return sentinel;

    node_t *cur = sentinel->left;
    while(cur != nullptr)
    {
      if(comp(make_r(*cur), data))
      {
        if(cur->right)
          cur = cur->right;
        else {
          res = ADD_RIGHT;
          break;
        }
      }
      else if(comp(data, make_r(*cur)))
      {
        if(cur->left)
          cur = cur->left;
        else {
          res = ADD_LEFT;
          break;
        }
      }
      else
      {
        res = THERE_IS;
        break;
      }


    }
    return cur;
  }

public:
  iterator find(const T& x) const {
    find_result res = THERE_IS;
    node_t *cur = find(x, res);
    if(res == THERE_IS)
      return (cur);

    return end();
  }

  iterator find_next(const T& x) const {
    find_result res = THERE_IS;
    node_t *cur = find(x, res);
    if(res == THERE_IS || res == ADD_LEFT)
      return (cur);
    return (cur->next());
  }

  iterator insert(T &data) {
    find_result res = ADD_LEFT;
    node_t *cur = find(data, res);
    if(res == THERE_IS) return end();

    if      (res == ADD_LEFT)
    {
      cur->left   = &data;
      static_cast<node_t*>(&data)->parent = cur;
      return (cur->left);
    }
    else {//if (res == ADD_RIGHT)  {
      cur->right  = &data;
      static_cast<node_t*>(&data)->parent = cur;
      return (cur->right);
    }
  }

  iterator remove(T &data)
  {
    find_result res = THERE_IS;
    node_t *cur = find(data, res);
    if(res == THERE_IS) return end();
    return remove(iterator(cur));
  }

  iterator remove(iterator it)
  {
    iterator it_next(it.cur->next());
    it.cur->unlink();
    return it_next;
  }
};
} // namespace intrusive
