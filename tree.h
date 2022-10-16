#pragma once

#include <cstddef>
#include <iterator>
#include <utility>

struct Node {
  Node* parent;
  Node* left = nullptr;
  Node* rght = nullptr;

  explicit Node(Node* parent);
  virtual ~Node();

  bool is_right();
  void unlink();

  static Node* min_node(Node* cur);
  static Node* max_node(Node* cur);
  static Node* next_node(Node* cur);
  static Node* prev_node(Node* cur);

  Node* next();
  Node* prev();
};
///--------------------------------------------------------------------------///
template <typename T, typename Compare = std::less<T>>
struct DNode : Node {
  T data;

  DNode(Node* parent, T&& data) : Node(parent), data(std::move(data)) {}

  static void swap_val(DNode<T>& first, DNode<T>& second) {
    T buf(std::move(first.data));
    first.data = std::move(second.data);
    second.data = std::move(buf);
  }

  static inline DNode<T>* make_pointer(Node* pointer) {
    return static_cast<DNode<T>*>(pointer);
  }

  DNode<T>* add(T&& new_data, Compare& compare, bool &is_success) {
    DNode<T>* cur = this;
    while (true) {
      if (compare(cur->data, new_data)) {
        if (cur->rght)
          cur = make_pointer(cur->rght);
        else
        {
          is_success = true;
          return make_pointer(cur->rght =
                                  new DNode<T>(cur, std::move(cur->data)));
        }
      } else if (compare(new_data, cur->data)) {
        if (cur->left)
          cur = make_pointer(cur->left);
        else
        {
          is_success = true;
          return make_pointer(cur->left =
                                  new DNode<T>(cur, std::move(cur->data)));
        }
      } else
      {
        is_success = false;
        return cur;
      }
    }
    is_success = false;
  }

  DNode<T>* find(const T& x, Compare& compare) {
    DNode<T>* cur = this;
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

  DNode<T>* remove_node(DNode<T>* cur) {
    if (cur->rght != nullptr && cur->left == nullptr) {
      if (cur->is_right())
        cur->parent->rght = cur->rght;
      else
        cur->parent->left = cur->rght;
      cur->rght->parent = cur->parent;
    } else if (cur->rght == nullptr && cur->left != nullptr) {
      if (cur->is_right())
        cur->parent->rght = cur->left;
      else
        cur->parent->left = cur->left;
      cur->left->parent = cur->parent;
    } else if (cur->rght != nullptr && cur->left != nullptr) {
      DNode<T>* next = make_pointer(cur->next());
      swap_val(*cur, *next);
      cur = remove_node(next);
    }
    return cur;
  }

  bool remove(const T& x, Compare& compare) {
    DNode<T>* cur = find(x, compare);
    if (cur == nullptr)
      return false;

    cur = remove_node(cur);

    delete cur;
    return true;
  }

  ~DNode() override {
    delete make_pointer(Node::rght);
    delete make_pointer(Node::left);
  }
};

///--------------------------------------------------------------------------///
template <typename T, typename Compare = std::less<T>>
struct Sentinel : Node {
  Sentinel() : Node(nullptr) {}

  DNode<T>* root() const {
    return static_cast<DNode<T>*>(Node::rght);
  }

  bool empty() {
    return rght == nullptr;
  }

  Node* add(T&& new_data, Compare& compare, bool &is_success) {
    if (Node::rght == nullptr) {
      rght = new DNode<T>(this, std::move(new_data));
      left = rght;
      is_success = true;
      return rght;
    }
    return static_cast<DNode<T, Compare>*>(rght)->add(std::move(new_data),
                                                      compare, is_success);
  }

  ~Sentinel() override {
    delete root();
  }
};

///==========================================================================///
template <typename T, typename Compare = std::less<T>>
class Tree {
  Compare comp = Compare{};
  Sentinel<T, Compare> sentinel;
  size_t n_node = 0;

public:
  Tree() = default;
  explicit Tree(Compare compare) : comp(std::move(compare)) {}

  bool empty() const {
    return sentinel.empty();
  }

  size_t size()
  {
    return n_node;
  }

  DNode<T>* root_node() {
    return sentinel.root();
  }

private:
  ///------------------------------------------------------------------------///
  template <typename iT>
  class preorder_iterator {
  private:
    Node* cur = nullptr;

    template <typename tT>
    friend class Tree;

    template <class tT>
    static preorder_iterator begin_iter(const Tree<tT>* tree) {
      return (tree->sentinel.root);
    }

    template <class tT>
    static preorder_iterator end_iter(const Tree<tT>* tree) {
      return (&tree->sentinel);
    }

  public:
    preorder_iterator(preorder_iterator &&pi) : cur(pi.cur) {pi.cur == nullptr;}
    preorder_iterator(const preorder_iterator &pi) : cur(pi.cur) {}

    preorder_iterator(decltype(cur) cur) : cur(cur) {}

    using difference_type = ptrdiff_t;
    using value_type = iT;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;

    preorder_iterator() = default;

    template <typename eq_iT>
    bool operator==(preorder_iterator<eq_iT> const& other) const {
      return cur == other.cur;
    }

    template <typename eq_iT>
    bool operator!=(preorder_iterator<eq_iT> const& other) const {
      return cur != other.cur;
    }

    DNode<iT>* get_node() const {
      return static_cast<DNode<iT>*>(cur);
    }

    reference operator*() const {
      return static_cast<DNode<iT>*>(cur)->get();
    }

    pointer operator->() const {
      return static_cast<DNode<iT>*>(cur)->get();
    }

    preorder_iterator& operator++() {
      if (cur->left) {
        cur = cur->left;
      } else if (cur->rght) {
        cur = cur->rght;
      } else {
        while (cur->parent != nullptr &&
               (cur->parent->rght == cur ||
                (cur->parent->left == cur && cur->parent->rght == nullptr))) {
          cur = cur->parent;
        }
        if (cur->parent != nullptr)
          cur = cur->parent->rght;
      }
      return *this;
    }

    preorder_iterator& operator--() {
      if (cur->parent) {
        if (cur->parent->left == cur || cur->parent->left == nullptr) {
          cur = cur->parent;
          return *this;
        } else {
          cur = cur->parent->left;
        }
      }
      while (cur->rght != nullptr || cur->left != nullptr) {
        if (cur->rght)
          cur = cur->rght;
        else
          cur = cur->left;
      }
      return *this;
    }

    preorder_iterator operator++(int) {
      preorder_iterator res(*this);
      ++(*this);
      return res;
    }

    preorder_iterator operator--(int) {
      preorder_iterator res(*this);
      --(*this);
      return res;
    }
  };
  ///------------------------------------------------------------------------///
  template <typename iT>
  class inorder_iterator {
  private:
    Node* cur = nullptr;

    template <typename tT, typename tCompare>
    friend class Tree;

    template <class tT>
    static inorder_iterator begin_iter(const Tree<tT>* tree) {
      return (Node::min_node(tree->sentinel.root()));
    }

    template <class tT>
    static inorder_iterator end_iter(const Tree<tT>* tree) {
      return (const_cast<Sentinel<tT>*>(&tree->sentinel));
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

    reference operator*() const {
      return static_cast<DNode<iT>*>(cur)->data;
    }

    pointer operator->() const {
      return static_cast<DNode<iT>*>(cur)->data;
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
  using iterator = inorder_iterator<T>;
  using const_iterator = inorder_iterator<const T>;

  iterator begin() {
    return iterator::begin_iter(this);
  }

  const_iterator begin() const {
    return const_iterator::begin_iter(this);
  }

  iterator end() {
    return iterator::end_iter(this);
  }

  const_iterator end() const {
    return const_iterator::end_iter(this);
  }

  ///------------------------------------------------------------------------///

  iterator add(T data) {
    bool is_success = false;
    Node *result = sentinel.add(std::move(data), comp, is_success);
    n_node++;
    return is_success ? iterator(result) : end();
  }

  iterator min() const {
    return (Node::min_node(root_node()));
  }

  iterator max() const {
    return (Node::max_node(root_node()));
  }

  iterator find(const T& x, Compare compare = Compare{}) const {
    DNode<T>* cur = sentinel.root()->find(x, compare);
    return cur ? iterator(cur) : iterator::end_iter(this);
  }

  bool remove(const T& x, Compare compare = Compare{}) {
    if(sentinel.root()->remove(x, compare))
    {
      n_node--;
      return true;
    }
    return false;
  }
};
