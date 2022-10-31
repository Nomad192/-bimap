#pragma once

#include "bimap_details.h"
#include "intrusive_tree.h"

#include <cassert>
#include <cstddef>
#include <stdexcept>

template <typename Left, typename Right, typename CompareLeft = std::less<Left>,
          typename CompareRight = std::less<Right>>
class bimap {
  using left_t = Left;
  using right_t = Right;
  using left_tag = details::left_tag;
  using right_tag = details::right_tag;
  using node_t = details::node_t<Left, Right>;

  template <typename Base, typename Compare, typename Tag>
  using intrusive_tree =
      intrusive::intrusive_tree<details::key_t<Base, Tag>, Compare, Tag>;

  using l_comparator_t = details::comparator_t<Left, CompareLeft, left_tag>;
  using r_comparator_t = details::comparator_t<Right, CompareRight, right_tag>;
  using l_tree_t = intrusive_tree<Left, l_comparator_t, left_tag>;
  using r_tree_t = intrusive_tree<Right, r_comparator_t, right_tag>;

  details::sentinel_t sentinel; /// only this order, sentinel first
  l_tree_t left_tree;
  r_tree_t right_tree;
  size_t n_node = 0;

  template <typename Base, typename Pair, typename CompareBase,
            typename ComparePair, typename TagBase, typename TagPair>
  struct base_iterator {
    typename intrusive_tree<Base, CompareBase, TagBase>::iterator it_tree;

    explicit base_iterator(
        typename intrusive_tree<Base, CompareBase, TagBase>::iterator it_tree)
        : it_tree(it_tree) {}

    using difference_type = ptrdiff_t;
    using value_type = Base;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;

    // Элемент на который сейчас ссылается итератор.
    // Разыменование итератора end_left() неопределено.
    // Разыменование невалидного итератора неопределено.
    Base const& operator*() const {
      return ((*it_tree).key);
    }
    Base const* operator->() const {
      return &(*it_tree).key;
    }

    // Переход к следующему по величине left'у.
    // Инкремент итератора end_left() неопределен.
    // Инкремент невалидного итератора неопределен.
    base_iterator& operator++() {
      ++it_tree;
      return *this;
    }
    base_iterator operator++(int) {
      base_iterator res(*this);
      ++(*this);
      return res;
    }

    bool operator==(base_iterator const& b) const {
      return it_tree == b.it_tree;
    }
    bool operator!=(base_iterator const& b) const {
      return it_tree != b.it_tree;
    }

    // Переход к предыдущему по величине left'у.
    // Декремент итератора begin_left() неопределен.
    // Декремент невалидного итератора неопределен.
    base_iterator& operator--() {
      --it_tree;
      return *this;
    }
    base_iterator operator--(int) {
      base_iterator res(*this);
      --(*this);
      return res;
    }

    // left_iterator ссылается на левый элемент некоторой пары.
    // Эта функция возвращает итератор на правый элемент той же пары.
    // end_left().flip() возращает end_right().
    // end_right().flip() возвращает end_left().
    // flip() невалидного итератора неопределен.
    base_iterator<Pair, Base, ComparePair, CompareBase, TagPair, TagBase>
    flip() const {
      if (it_tree.is_end())
        return base_iterator<Pair, Base, ComparePair, CompareBase, TagPair,
                             TagBase>(
            typename intrusive_tree<Pair, ComparePair, TagPair>::iterator(
                static_cast<details::sentinel_t*>(it_tree.get_node())));

      return base_iterator<Pair, Base, ComparePair, CompareBase, TagPair,
                           TagBase>(
          typename intrusive_tree<Pair, ComparePair, TagPair>::iterator(
              static_cast<details::node_t<Left, Right>*>(&(*it_tree))));
    }
  };

public:
  using left_iterator =
      base_iterator<Left, Right, l_comparator_t, r_comparator_t,
                    details::left_tag, details::right_tag>;
  using right_iterator =
      base_iterator<Right, Left, r_comparator_t, l_comparator_t,
                    details::right_tag, details::left_tag>;

  void swap(bimap& other) {
    left_tree.swap(other.left_tree);
    right_tree.swap(other.right_tree);
    std::swap(n_node, other.n_node);
  }

  // Возващает итератор на минимальный по порядку left.
  left_iterator begin_left() const {
    return left_iterator(left_tree.begin());
  }
  // Возващает итератор на следующий за последним по порядку left.
  left_iterator end_left() const {
    return left_iterator(left_tree.end());
  }

  // Возващает итератор на минимальный по порядку right.
  right_iterator begin_right() const {
    return right_iterator(right_tree.begin());
  }
  // Возващает итератор на следующий за последним по порядку right.
  right_iterator end_right() const {
    return right_iterator(right_tree.end());
  }

  // Создает bimap не содержащий ни одной пары.
  explicit bimap(CompareLeft compare_left = CompareLeft(),
                 CompareRight compare_right = CompareRight())
      : left_tree(&sentinel,
                  std::move(l_comparator_t(std::move(compare_left)))),
        right_tree(&sentinel,
                   std::move(r_comparator_t(std::move(compare_right)))) {}

  // Конструкторы от других и присваивания
  bimap(bimap const& other)
      : left_tree(&sentinel, static_cast<l_comparator_t>(other.left_tree)),
        right_tree(&sentinel, static_cast<r_comparator_t>(other.right_tree))
  ///,n_node(0) - because insert
  {
    for (auto it = other.begin_left(); it != other.end_left(); it++) {
      this->insert(*it, *(it.flip()));
    }
  }
  bimap(bimap&& other) noexcept
      : left_tree(&sentinel, static_cast<l_comparator_t>(other.left_tree)),
        right_tree(&sentinel, static_cast<r_comparator_t>(other.right_tree)),
        n_node(other.n_node) {
    left_tree.swap(other.left_tree);
    right_tree.swap(other.right_tree);
  }

  bimap& operator=(bimap const& other) {
    if (this != &other)
      bimap(other).swap(*this);
    return *this;
  }
  bimap& operator=(bimap&& other) noexcept {
    if (this != &other)
      bimap(std::move(other)).swap(*this);
    return *this;
  }

  // Деструктор. Вызывается при удалении объектов bimap.
  // Инвалидирует все итераторы ссылающиеся на элементы этого bimap
  // (включая итераторы ссылающиеся на элементы следующие за последними).
  ~bimap() {
    auto it = begin_left();

    while (it != end_left()) {
      it = erase_left(it);
    }
  }

private:
  template <typename lpf = left_t, typename rpf = right_t>
  left_iterator add(lpf&& left, rpf&& right) {
    if (left_tree.find(details::key_t<lpf&&, left_tag>(
            std::forward<lpf>(left))) == left_tree.end() &&
        right_tree.find(details::key_t<rpf&&, right_tag>(
            std::forward<rpf>(right))) == right_tree.end()) {
      auto* new_node =
          new node_t{std::forward<lpf>(left), std::forward<rpf>(right)};
      typename l_tree_t::iterator iter_left_tree = left_tree.insert(*new_node);
      right_tree.insert(*new_node);

      n_node++;

      return left_iterator(iter_left_tree);
    }
    return end_left();
  }

public:
  // Вставка пары (left, right), возвращает итератор на left.
  // Если такой left или такой right уже присутствуют в bimap, вставка не
  // производится и возвращается end_left().
  left_iterator insert(Left const& left, Right const& right) {
    return add(left, right);
  }
  left_iterator insert(Left const& left, Right&& right) {
    return add(left, std::move(right));
  }
  left_iterator insert(Left&& left, Right const& right) {
    return add(std::move(left), right);
  }
  left_iterator insert(Left&& left, Right&& right) {
    return add(std::move(left), std::move(right));
  }

  // Удаляет элемент и соответствующий ему парный.
  // erase невалидного итератора неопределен.
  // erase(end_left()) и erase(end_right()) неопределены.
  // Пусть it ссылается на некоторый элемент e.
  // erase инвалидирует все итераторы ссылающиеся на e и на элемент парный к e.
  left_iterator erase_left(left_iterator it) {
    auto* pointer = static_cast<node_t*>(&(*(it.it_tree)));
    it++;
    n_node--;

    delete pointer;
    return it;
  }

  // Аналогично erase, но по ключу, удаляет элемент если он присутствует, иначе
  // не делает ничего Возвращает была ли пара удалена
  bool erase_left(left_t const& left) {
    left_iterator l_iter = find_left(left);
    if (l_iter != end_left()) {
      erase_left(l_iter);
      return true;
    }
    return false;
  }

  right_iterator erase_right(right_iterator it) {
    auto* pointer = static_cast<node_t*>(&(*(it.it_tree)));
    it++;
    n_node--;

    delete pointer;
    return it;
  }

  bool erase_right(right_t const& right) {
    right_iterator r_iter = find_right(right);
    if (r_iter != end_right()) {
      erase_right(r_iter);
      return true;
    }
    return false;
  }

  // erase от ренжа, удаляет [first, last), возвращает итератор на последний
  // элемент за удаленной последовательностью
  left_iterator erase_left(left_iterator first, left_iterator last) {
    while (first != last)
      first = erase_left(first);

    return last;
  }
  right_iterator erase_right(right_iterator first, right_iterator last) {
    while (first != last)
      first = erase_right(first);

    return last;
  }

  // Возвращает итератор по элементу. Если не найден - соответствующий end()
  left_iterator find_left(left_t const& left) const {
    return left_iterator(
        left_tree.find(details::key_t<left_t const&, details::left_tag>{left}));
  }
  right_iterator find_right(right_t const& right) const {
    return right_iterator(right_tree.find(
        details::key_t<right_t const&, details::right_tag>{right}));
  }

  // Возвращает противоположный элемент по элементу
  // Если элемента не существует -- бросает std::out_of_range
  right_t const& at_left(left_t const& key) const {

    left_iterator iter = find_left(key);
    if (iter == end_left())
      throw std::out_of_range("cannot find el");
    return *(iter.flip());
  }
  left_t const& at_right(right_t const& key) const {

    right_iterator iter = find_right(key);
    if (iter == end_right())
      throw std::out_of_range("cannot find el");
    return *(iter.flip());
  }

  // Возвращает противоположный элемент по элементу
  // Если элемента не существует, добавляет его в bimap и на противоположную
  // сторону кладет дефолтный элемент, ссылку на который и возвращает
  // Если дефолтный элемент уже лежит в противоположной паре - должен поменять
  // соответствующий ему элемент на запрашиваемый (смотри тесты)
  template <typename = std::enable_if<std::is_default_constructible_v<Right>>>
  right_t const& at_left_or_default(left_t const& key) {
    left_iterator l_iter = find_left(key);
    if (l_iter == end_left()) {
      right_t r_data{};
      right_iterator r_iter = find_right(std::move(r_data));
      if (r_iter == end_right()) {
        return *(insert(key, r_data).flip());
      } else {
        erase_right(r_iter);
        return *(insert(key, std::move(r_data)).flip());
      }
    }

    return *(l_iter.flip());
  }

  template <typename = std::enable_if<std::is_default_constructible_v<Left>>>
  left_t const& at_right_or_default(right_t const& key) {
    right_iterator r_iter = find_right(key);
    if (r_iter == end_right()) {
      left_t l_data{};
      left_iterator l_iter = find_left(std::move(l_data));
      if (l_iter == end_left()) {
        return *(insert(l_data, key));
      } else {
        erase_left(l_iter);
        return *(insert(std::move(l_data), key));
      }
    }

    return *(r_iter.flip());
  }

  // lower и upper bound'ы по каждой стороне
  // Возвращают итераторы на соответствующие элементы
  // Смотри std::lower_bound, std::upper_bound.
  left_iterator lower_bound_left(const left_t& key) const {
    return left_iterator{left_tree.lower_bound(
        details::key_t<const left_t&, details::left_tag>{key})};
  }
  left_iterator upper_bound_left(const left_t& key) const {
    return left_iterator{left_tree.upper_bound(
        details::key_t<const left_t&, details::left_tag>{key})};
  }

  right_iterator lower_bound_right(const right_t& key) const {
    return right_iterator{right_tree.lower_bound(
        details::key_t<const right_t&, details::right_tag>{key})};
  }
  right_iterator upper_bound_right(const right_t& key) const {
    return right_iterator{right_tree.upper_bound(
        details::key_t<const right_t&, details::right_tag>{key})};
  }

  // Проверка на пустоту
  bool empty() const {
    assert(left_tree.empty() == right_tree.empty());
    assert(left_tree.empty() == (n_node == 0));

    return n_node == 0;
  }

  // Возвращает размер бимапы (кол-во пар)
  std::size_t size() const {
    return n_node;
  }

  template <typename L, typename R, typename cL, typename cR>
  friend bool operator==(bimap<L, R, cL, cR> const& a, bimap<L, R, cL, cR> const& b);

  template <typename L, typename R, typename cL, typename cR>
  friend bool operator!=(bimap<L, R, cL, cR> const& a, bimap<L, R, cL, cR> const& b);


  bool eq_left(const left_t &a, const left_t &b) const
  {
    if(static_cast<l_comparator_t>(left_tree).comp(a, b))
      return false;
    if(static_cast<l_comparator_t>(left_tree).comp(b, a))
      return false;
    return true;
  }

  bool eq_right(const right_t &a, const right_t &b) const
  {
    if(static_cast<r_comparator_t>(right_tree).comp(a, b))
      return false;
    if(static_cast<r_comparator_t>(right_tree).comp(b, a))
      return false;
    return true;
  }
};

//// операторы сравнения
template <typename L, typename R, typename cL, typename cR>
bool operator==(bimap<L, R, cL, cR> const& a, bimap<L, R, cL, cR> const& b) {
  if (a.size() != b.size())
    return false;

  for (auto it_a = a.begin_left(), it_b = b.begin_left();
       it_a != a.end_left() && it_b != b.end_left(); it_a++, it_b++) {
    if (!a.eq_left(*it_a, *it_b))  //*it_a != *it_b)
      return false;
    if (!a.eq_right(*it_a.flip(), *it_b.flip()))  //*it_a.flip() != *it_b.flip())
      return false;
  }

  return true;
}

template <typename L, typename R, typename cL, typename cR>
bool operator!=(bimap<L, R, cL, cR> const& a, bimap<L, R, cL, cR> const& b) {
  return !(a == b);
}