#pragma once
#include <cassert>
#include <cstddef>
#include <stdexcept>

#include "tree.h"

template <typename Left, typename Right, typename CompareLeft = std::less<Left>,
          typename CompareRight = std::less<Right>>
class bimap {

  template <typename Data, typename Pair, typename CompareData,
            typename ComparePair>
  struct CompareNode;

  template <typename Data, typename Pair, typename CompareData,
            typename ComparePair>
  struct Node {
    Data data;
    Node(Data data) : data(std::move(data)) {}
    typename Tree<Node<Pair, Data, ComparePair, CompareData>,
                  CompareNode<Pair, Data, ComparePair, CompareData>>::iterator
        pair;
  };

  template <typename Data, typename Pair, typename CompareData,
            typename ComparePair>
  struct CompareNode {
    CompareData comp;
    CompareNode(decltype(comp) comp) : comp(comp) {}
    bool operator()(const Node<Data, Pair, CompareData, ComparePair>& a,
                    const Node<Data, Pair, CompareData, ComparePair>& b) const {
      return comp.operator()(a.data, b.data);
    }
  };

  using l_node = Node<Left, Right, CompareLeft, CompareRight>;
  using r_node = Node<Right, Left, CompareRight, CompareLeft>;
  using compare_l_node = CompareNode<Left, Right, CompareLeft, CompareRight>;
  using compare_r_node = CompareNode<Right, Left, CompareRight, CompareLeft>;

  Tree<l_node, compare_l_node> left_tree;
  Tree<r_node, compare_r_node> right_tree;

public:
  using left_t = Left;
  using right_t = Right;

  // using node_t = Node;

  template <typename Base, typename Pair, typename CompareBase,
            typename ComparePair>
  struct base_iterator {
    typename Tree<Node<Base, Pair, CompareBase, ComparePair>,
                  CompareNode<Base, Pair, CompareBase, ComparePair>>::iterator
        i_cur;

    explicit base_iterator(decltype(i_cur) i_cur) : i_cur(i_cur) {}

    // Элемент на который сейчас ссылается итератор.
    // Разыменование итератора end_left() неопределено.
    // Разыменование невалидного итератора неопределено.
    Base const& operator*() const {
      return (*i_cur).data;
    }
    Base const* operator->() const {
      return (*i_cur).data;
    }
    //
    //    // Переход к следующему по величине left'у.
    //    // Инкремент итератора end_left() неопределен.
    //    // Инкремент невалидного итератора неопределен.
    base_iterator& operator++() {
      ++i_cur;
      return *this;
    }
    base_iterator operator++(int) {
      base_iterator res(*this);
      ++(*this);
      return res;
    }
    //
    //    // Переход к предыдущему по величине left'у.
    //    // Декремент итератора begin_left() неопределен.
    //    // Декремент невалидного итератора неопределен.
    //          left_iterator &operator--();
    //          left_iterator operator--(int);

    bool operator==(base_iterator const& b) {
      return i_cur == b.i_cur;
    }
    bool operator!=(base_iterator const& b) {
      return i_cur != b.i_cur;
    }
  };

private:
  using base_iter_left = base_iterator<Left, Right, CompareLeft, CompareRight>;
  using base_iter_right = base_iterator<Right, Left, CompareRight, CompareLeft>;

public:
  struct right_iterator; // По аналогии с left_iterator
  struct left_iterator : base_iter_left {
    bimap<Left, Right, CompareLeft, CompareRight> const* my_bimap;
    left_iterator(typename Tree<l_node, compare_l_node>::iterator i_cur,
                  bimap<Left, Right, CompareLeft, CompareRight> const* my_bimap)
        : base_iter_left(i_cur), my_bimap(my_bimap) {}

    // left_iterator ссылается на левый элемент некоторой пары.
    // Эта функция возвращает итератор на правый элемент той же пары.
    // end_left().flip() возращает end_right().
    // end_right().flip() возвращает end_left().
    // flip() невалидного итератора неопределен.
    right_iterator flip() const {
      if (base_iter_left::i_cur.is_end()) {
        return {my_bimap->right_tree.end(), my_bimap};
      }
      return right_iterator((*base_iter_left::i_cur).pair, my_bimap);
    }
  };
  struct right_iterator : base_iter_right {
    bimap<Left, Right, CompareLeft, CompareRight> const* my_bimap;
    right_iterator(
        typename Tree<r_node, compare_r_node>::iterator i_cur,
        bimap<Left, Right, CompareLeft, CompareRight> const* my_bimap)
        : base_iter_right(i_cur), my_bimap(my_bimap) {}

    // left_iterator ссылается на левый элемент некоторой пары.
    // Эта функция возвращает итератор на правый элемент той же пары.
    // end_left().flip() возращает end_right().
    // end_right().flip() возвращает end_left().
    // flip() невалидного итератора неопределен.
    left_iterator flip() const {
      if (base_iter_right::i_cur.is_end()) {
        return {my_bimap->left_tree.end(), my_bimap};
      }
      return left_iterator((*base_iter_right::i_cur).pair, my_bimap);
    }
  };

  // Создает bimap не содержащий ни одной пары.
  bimap(CompareLeft compare_left = CompareLeft(),
        CompareRight compare_right = CompareRight())
      : left_tree(std::move(compare_l_node(compare_left))),
        right_tree(std::move(compare_r_node(compare_right))) {}

  //  // Конструкторы от других и присваивания
  //  bimap(bimap const &other);
  //  bimap(bimap &&other) noexcept;
  //
  //  bimap &operator=(bimap const &other);
  //  bimap &operator=(bimap &&other) noexcept;
  //
  //  // Деструктор. Вызывается при удалении объектов bimap.
  //  // Инвалидирует все итераторы ссылающиеся на элементы этого bimap
  //  // (включая итераторы ссылающиеся на элементы следующие за последними).
  ~bimap() {}
  //
  // Вставка пары (left, right), возвращает итератор на left.
  // Если такой left или такой right уже присутствуют в bimap, вставка не
  // производится и возвращается end_left().

private:
  left_iterator add(left_t left, right_t right) {
    typename Tree<l_node, compare_l_node>::iterator iter_left_tree =
        left_tree.add(l_node{std::move(left)});

    if (iter_left_tree == left_tree.end()) {
      return end_left();
    }

    typename Tree<r_node, compare_r_node>::iterator iter_right_tree =
        right_tree.add(r_node{std::move(right)});

    if (iter_right_tree == right_tree.end()) {
      assert(!left_tree.remove(*iter_left_tree));
      return end_left();
    }

    (*iter_left_tree).pair = iter_right_tree;
    (*iter_right_tree).pair = iter_left_tree;

    return left_iterator(iter_left_tree, this);
  }

public:
  left_iterator insert(left_t const& left, right_t const& right) {
    return add(left, right);
  }

  left_iterator insert(left_t const& left, right_t&& right) {
    return add(left, std::move(right));
  }
  left_iterator insert(left_t&& left, right_t const& right) {
    return add(std::move(left), right);
  }
  left_iterator insert(left_t&& left, right_t&& right) {
    return add(std::move(left), std::move(right));
  }

  //  // Удаляет элемент и соответствующий ему парный.
  //  // erase невалидного итератора неопределен.
  //  // erase(end_left()) и erase(end_right()) неопределены.
  //  // Пусть it ссылается на некоторый элемент e.
  //  // erase инвалидирует все итераторы ссылающиеся на e и на элемент парный к
  //  e.
  left_iterator erase_left(left_iterator it) {
    left_iterator next_it = it;
    next_it++;
    assert(!left_tree.remove(*it));
    assert(!right_tree.remove(*((*it).pair)));
    return next_it;
  }
  //  // Аналогично erase, но по ключу, удаляет элемент если он присутствует,
  //  иначе
  //  // не делает ничего Возвращает была ли пара удалена
  //  bool erase_left(left_t const &left);
  //
  //  right_iterator erase_right(right_iterator it);
  //  bool erase_right(right_t const &right);
  //
  //  // erase от ренжа, удаляет [first, last), возвращает итератор на последний
  //  // элемент за удаленной последовательностью
  //  left_iterator erase_left(left_iterator first, left_iterator last);
  //  right_iterator erase_right(right_iterator first, right_iterator last);
  //
  // Возвращает итератор по элементу. Если не найден - соответствующий end()
  left_iterator find_left(left_t const& left) const {
    left_iterator iter(left_tree.find(l_node{std::move(left)}), this);
  }
  right_iterator find_right(right_t const& right) const {
    right_iterator iter(right_tree.find(r_node{std::move(right)}), this);
  }

  // Возвращает противоположный элемент по элементу
  // Если элемента не существует -- бросает std::out_of_range
  right_t const& at_left(left_t const& key) const {

    left_iterator iter(left_tree.find(l_node{key}), this);
    if (iter == end_left())
      throw std::out_of_range("cannot find el");
    return *(iter.flip());
  }
  left_t const& at_right(right_t const& key) const {

    right_iterator iter(right_tree.find(r_node{key}), this);
    if (iter == end_left())
      throw std::out_of_range("cannot find el");
    return *(iter.flip());
  }
  //
  //  // Возвращает противоположный элемент по элементу
  //  // Если элемента не существует, добавляет его в bimap и на противоположную
  //  // сторону кладет дефолтный элемент, ссылку на который и возвращает
  //  // Если дефолтный элемент уже лежит в противоположной паре - должен
  //  поменять
  //  // соответствующий ему элемент на запрашиваемый (смотри тесты)
  //  right_t const &at_left_or_default(left_t const &key);
  //  left_t const &at_right_or_default(right_t const &key);
  //
  //  // lower и upper bound'ы по каждой стороне
  //  // Возвращают итераторы на соответствующие элементы
  //  // Смотри std::lower_bound, std::upper_bound.
  //  left_iterator lower_bound_left(const left_t &left) const;
  //  left_iterator upper_bound_left(const left_t &left) const;
  //
  //  right_iterator lower_bound_right(const right_t &left) const;
  //  right_iterator upper_bound_right(const right_t &left) const;

  // Возващает итератор на минимальный по порядку left.
  left_iterator begin_left() const {
    return left_iterator(left_tree.begin(), this);
  }
  // Возващает итератор на следующий за последним по порядку left.
  left_iterator end_left() const {
    return left_iterator(left_tree.end(), this);
  }

  // Возващает итератор на минимальный по порядку right.
  right_iterator begin_right() const {
    return right_iterator(right_tree.begin(), this);
  }
  // Возващает итератор на следующий за последним по порядку right.
  right_iterator end_right() const {
    return right_iterator(right_tree.end(), this);
  }

  // Проверка на пустоту
  bool empty() const {
    assert(left_tree.empty() == right_tree.empty());

    return left_tree.empty();
  }

  // Возвращает размер бимапы (кол-во пар)
  std::size_t size() const {
    assert(left_tree.size() == right_tree.size());

    return left_tree.size();
  }

  //  // операторы сравнения
  //  friend bool operator==(bimap const &a, bimap const &b);
  //  friend bool operator!=(bimap const &a, bimap const &b);
};
