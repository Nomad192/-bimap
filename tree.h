#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <utility>

struct Node {
  Node* parent;
  mutable Node* left = nullptr;
  Node* rght = nullptr;

  explicit Node(Node* parent);
  virtual ~Node();

  bool is_right();
  void unlink();
  void unlink_child();

  static Node* min_node(Node* cur);
  static Node* max_node(Node* cur);
  static Node* next_node(Node* cur);
  static Node* prev_node(Node* cur);

  Node* next();
  Node* prev();
  virtual void set_child(Node* child, Node* set) {
    if (child->is_right())
      this->rght = set;
    else
      this->left = set;
  }
};
///--------------------------------------------------------------------------///
template <typename T, typename Compare = std::less<T>>
struct DNode : Node {
  T data;

  DNode(Node* parent, T&& data) : Node(parent), data(std::move(data)) {}

  static void swap_val(DNode<T, Compare>& first, DNode<T, Compare>& second) {
    T buf(std::move(first.data));
    first.data = std::move(second.data);
    second.data = std::move(buf);
  }

  static inline DNode<T, Compare>* make_pointer(Node* pointer) {
    return static_cast<DNode<T, Compare>*>(pointer);
  }

  DNode<T, Compare>* add(T&& new_data, Compare& compare, bool& is_success) {
    DNode<T, Compare>* cur = this;
    while (true) {
      if (compare(cur->data, new_data)) {
        if (cur->rght)
          cur = make_pointer(cur->rght);
        else {
          is_success = true;
          return make_pointer(
              cur->rght = new DNode<T, Compare>(cur, std::move(new_data)));
        }
      } else if (compare(new_data, cur->data)) {
        if (cur->left)
          cur = make_pointer(cur->left);
        else {
          is_success = true;
          return make_pointer(
              cur->left = new DNode<T, Compare>(cur, std::move(new_data)));
        }
      } else {
        is_success = false;
        return cur;
      }
    }
    is_success = false;
  }

  DNode<T, Compare>* find(const T& x, Compare const& compare) const {
    auto* cur = const_cast<DNode<T, Compare>*>(this); ///!!!!!!!!!!!!!!!!!!
    while (cur) {
      if (compare(cur->data, x))
        cur = make_pointer(cur->rght);
      else if (compare(x, cur->data))
        cur = make_pointer(cur->left);
      else
        break;
    }
    return cur;
  }

  Node* find_fake_node(const T& x, Compare const& compare,
                       Node* fake_node) const {
    DNode<T, Compare>* cur =
        const_cast<DNode<T, Compare>*>(this); ///!!!!!!!!!!!!!!!!!!
    do {
      if (compare(cur->data, x)) {
        if (cur->rght)
          cur = make_pointer(cur->rght);
        else {
          fake_node->parent = cur;
          cur->rght = fake_node;
          return fake_node;
        }
      } else if (compare(x, cur->data)) {
        if (cur->left)
          cur = make_pointer(cur->left);
        else {
          fake_node->parent = cur;
          cur->left = fake_node;
          return fake_node;
        }
      } else
        break;
    } while (cur);

    return cur;
  }

  Node* find_next(const T& x, Compare const& compare) const {
    Node fake_node(nullptr);
    Node* cur = find_fake_node(x, compare, &fake_node);
    if (fake_node.parent != nullptr) {
      Node* next = fake_node.next();
      // fake_node.parent->set_child(&fake_node, nullptr);
      //      fake_node.unlink();
      //      fake_node.parent = nullptr;
      return next;
    }
    return cur;
  }

  Node* find_prev(const T& x, Compare const& compare) const {
    Node fake_node(nullptr);
    Node* cur = find_fake_node(x, compare, &fake_node);
    if (fake_node.parent != nullptr) {
      Node* prev = fake_node.prev();
      fake_node.unlink();
      return prev;
    }
    return cur;
  }

  Node* unlink_node(DNode<T, Compare>* cur) { ///!!!!!!!!!!!!!!!!
    Node* next_local = cur->next();
    if (cur->rght != nullptr && cur->left == nullptr) {
      cur->parent->set_child(this, cur->rght);
      //      if (cur->is_right())
      //        cur->parent->rght = cur->rght;
      //      else
      //        cur->parent->left = cur->rght;
      cur->rght->parent = cur->parent;
      cur->rght = nullptr;
    } else if (cur->rght == nullptr && cur->left != nullptr) {
      cur->parent->set_child(this, cur->left);
      //      if (cur->is_right())
      //        cur->parent->rght = cur->left;
      //      else
      //        cur->parent->left = cur->left;
      cur->left->parent = cur->parent;
      cur->left = nullptr;
    } else if (cur->rght != nullptr && cur->left != nullptr) {
      DNode<T, Compare>* next = make_pointer(cur->next());

      if (next->rght) {
        if (next->is_right())
          next->parent->rght = next->rght;
        else
          next->parent->left = next->rght;
        next->rght->parent = next->parent;
      } else {
        if (next->is_right())
          next->parent->rght = nullptr;
        else
          next->parent->left = nullptr;
      }
      next->left = cur->left;
      next->left->parent = next;
      next->rght = cur->rght;
      if (next->rght)
        next->rght->parent = next;
      next->parent = cur->parent;
      if (cur->is_right())
        cur->parent->rght = next;
      else
        cur->parent->left = next;
    } else {
      cur->parent->set_child(this, nullptr);
    }
    cur->parent = nullptr;
    cur->left = nullptr;
    cur->rght = nullptr;
    return next_local;
  }

  bool remove(const T& x, Compare& compare) { ///!!!!!!!!!!!!!!!!!!!!!!!!!!!
    DNode<T, Compare>* cur = find(x, compare);
    if (cur == nullptr)
      return false;

    unlink_node(cur);
    assert(cur->parent == nullptr);
    assert(cur->rght == nullptr && cur->left == nullptr);
    delete cur;
    return true;
  }

  Node* remove_this() {
    Node* next = unlink_node(this);
    assert(this->parent == nullptr);
    assert(this->rght == nullptr && this->left == nullptr);
    delete this;
    return next;
  }

  ~DNode() override {
    delete make_pointer(Node::rght);
    delete make_pointer(Node::left);
  }
};

///--------------------------------------------------------------------------///
template <typename T, typename Compare>
struct Sentinel : Node {
private:
  void set_child(Node* child, Node* set) {
    this->rght = set;
    this->left = set;
  }

public:
  DNode<T, Compare>* root() const {
    //assert(left == rght);
    left = rght;
    return static_cast<DNode<T, Compare>*>(rght);
  }

  Node* to_node_pointer() const {
    return const_cast<Node*>(static_cast<const Node*>(this));
  }

  Sentinel() : Node(nullptr) {}
  void swap(Sentinel& other) {
    std::swap(this->left, other.left);
    std::swap(this->rght, other.rght);
    rght->parent = this;
    other.rght->parent = &other;
  }

  bool empty() const {
    return rght == nullptr;
  }

  Node* add(T&& new_data, Compare& compare, bool& is_success) {
    if (!root()) {
      set_child(nullptr, new DNode<T, Compare>(this, std::move(new_data)));
      //      rght = new DNode<T, Compare>(this, std::move(new_data));
      //      left = rght; ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      is_success = true;
      return rght;
    }
    return static_cast<DNode<T, Compare>*>(rght)->add(std::move(new_data),
                                                      compare, is_success);
  }

  Node* find(const T& x, Compare const& compare) const {
    if (root())
      return root()->find(x, compare);
    return nullptr;
  }

  Node* find_next(const T& x, Compare const& compare) const {
    if (root())
      return root()->find_next(x, compare);
    return this->to_node_pointer();
  }

  Node* find_prev(const T& x, Compare const& compare) const {
    if (root())
      return root()->find_prev(x, compare);
    return this->to_node_pointer();
  }

  bool remove(const T& x, Compare& compare) { ///!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if (root())
      return root().remove(x, compare);
    return false;
  }

  ~Sentinel() override {
    delete root();
  }
};

///==========================================================================///
template <typename T, typename Compare = std::less<T>>
class Tree {
  Compare comp;
  Sentinel<T, Compare> sentinel;
  size_t n_node = 0;

public:
  Compare get_compare() const {
    return this->comp;
  }

  Tree() = default;
  Tree(Compare compare) : comp(std::move(compare)) {}

  Tree(Tree const& other) : comp(other.comp) {
    for (auto it = other.begin(); it != other.end(); it++) {
      this->add(*it);
    }
  }

  Tree(Tree&& other) : comp(other.comp) {
    this->swap(other);
  }

  Tree& operator=(Tree const& other) {
    if (this != &other)
      Tree(other).swap(*this);
    return *this;
  }

  Tree& operator=(Tree&& other) {
    if (this != &other)
      Tree(std::move(other)).swap(*this);
    return *this;
  }

  bool empty() const {
    return sentinel.empty();
  }

  size_t size() const {
    return n_node;
  }

  //  DNode<T, Compare>* root_node() {
  //    return sentinel.root();
  //  }

  void swap(Tree& b) {
    std::swap(this->n_node, b.n_node);
    std::swap(this->comp, b.comp);
    sentinel.swap(b.sentinel);
  }

private:
  ///------------------------------------------------------------------------///
  template <typename iT, typename iCompare>
  class inorder_iterator {
  private:
    Node* cur = nullptr;

    template <typename tT, typename tCompare>
    friend class Tree;

    template <typename tT, typename tCompare>
    static inorder_iterator begin_iter(const Tree<tT, tCompare>* tree) {
      if (tree->sentinel.root())
        return (Node::min_node(tree->sentinel.root()));

      return end_iter(tree);
    }

    template <typename tT, typename tCompare>
    static inorder_iterator end_iter(const Tree<tT, tCompare>* tree) {
      return (const_cast<Sentinel<tT, tCompare>*>(
          &tree->sentinel)); ////////////!!!!!!!!!!!!!!!!!!!
    }

  public:
    inorder_iterator(decltype(cur) cur) : cur(cur) {}

    using difference_type = ptrdiff_t;
    using value_type = iT;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;

    inorder_iterator() = default;

    template <typename eq_iT, typename eq_iCompare>
    bool operator==(inorder_iterator<eq_iT, eq_iCompare> const& other) const {
      return cur == other.cur;
    }

    template <typename eq_iT, typename eq_iCompare>
    bool operator!=(inorder_iterator<eq_iT, eq_iCompare> const& other) const {
      return cur != other.cur;
    }

    bool is_end() const {
      if (cur->parent == nullptr)
        return true;
      return false;
    }

    reference operator*() const {
      return static_cast<DNode<iT, iCompare>*>(cur)->data;
    }

    pointer operator->() const {
      return &(static_cast<DNode<iT, iCompare>*>(cur)->data);
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
  ///------------------------------------------------------------------------///

public:
  using iterator = inorder_iterator<T, Compare>;
  using const_iterator = inorder_iterator<const T, Compare>;

  iterator begin() const {
    return iterator::begin_iter(this);
  }

  //  const_iterator begin() const {
  //    return const_iterator::begin_iter(this);
  //  }

  iterator end() const {
    return iterator::end_iter(this);
  }

  //  const_iterator end() const {
  //    return const_iterator::end_iter(this);
  //  }

  ///------------------------------------------------------------------------///

  iterator add(T data) {
    bool is_success = false;
    Node* result = sentinel.add(std::move(data), comp, is_success);
    if (is_success)
      n_node++;
    return is_success ? iterator(result) : end();
  }

  //  iterator min() const {
  //    return (Node::min_node(root_node()));
  //  }
  //
  //  iterator max() const {
  //    return (Node::max_node(root_node()));
  //  }

  iterator find(const T& x) const {
    Node* cur = sentinel.find(x, comp);
    return cur ? iterator(cur) : iterator::end_iter(this);
  }

  iterator find_next(const T& x) const {
    return iterator(sentinel.find_next(x, comp));
  }

  iterator find_prev(const T& x) const {
    return iterator(sentinel.find_prev(x, comp));
  }

  bool remove(const T& x) {

    if (sentinel.remove(x, comp)) {
      n_node--;
      return true;
    }
    return false;
  }

  iterator remove(iterator it) {
    n_node--;
    return {static_cast<DNode<T, Compare>*>(it.cur)->remove_this()};
  }
};
