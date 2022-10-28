#pragma once
#include <cassert>
#include <cstddef>
#include <stdexcept>

#include "intrusive_tree.h"

namespace details{

struct left_tag{};
struct right_tag{};

template <typename Key, typename Tag>
struct key_t : public intrusive::node<Tag> {
  Key key;

  explicit key_t(Key &&key) : key(std::move(key)) {}

  explicit operator const Key&() const { return key; }
};

template <typename Left, typename Right>
struct node_t : public key_t<Left, left_tag>, public key_t<Right, right_tag> {
  node_t(Left &&left, Right &&right) : key_t<Left, left_tag>(std::move(left)), key_t<Right, right_tag>(std::move(right)) {}
};

template <typename Base, typename Comparator, typename Tag>
struct comparator_t : public Comparator {
  bool operator()(key_t<Base, Tag> &a, key_t<Base, Tag> &b) {
    return Comparator::operator()(a.key, b.key);
  }
};

struct sentinel_t : public intrusive::node<left_tag>, public intrusive::node<right_tag> {};
//
//template <typename Left, typename Right, typename CompareLeft>
//struct CompareNode {
//  CompareLeft comp;
//  explicit CompareNode(decltype(comp) comp) : comp(comp) {}
//  bool operator()(const node_t<Left, Right>& a,
//                  const node_t<Left, Right>& b) const {
//    return comp(a.key, b.key);
//  }
//};

} //namespace details






template <typename Left, typename Right, typename CompareLeft = std::less<Left>,
          typename CompareRight = std::less<Right>>
class bimap {
//  using left_t = details::key_t<Left, details::left_tag>;
//  using right_t = details::key_t<Right, details::right_tag>;
  using node_t = details::node_t<Left, Right>;
  using left_tag = details::left_tag;
  using right_tag = details::right_tag;

  template <typename Compare, typename Tag>
  using intrusive_tree = intrusive::intrusive_tree<node_t, Compare, Tag>;

  using l_comparator_t = details::comparator_t<Left, CompareLeft, left_tag>;
  using r_comparator_t = details::comparator_t<Right, CompareRight, right_tag>;
  using l_tree_t = intrusive_tree<l_comparator_t, left_tag>;
  using r_tree_t = intrusive_tree<r_comparator_t, right_tag>;


  details::sentinel_t sentinel;
  l_tree_t left_tree;
  r_tree_t right_tree;

//  using compare_l_node = details::CompareNodeLeft<Left, Right, CompareLeft>;
//  using compare_r_node = details::CompareNodeRight<Left, Right, CompareRight>;
//

//
//  intrusive_set<compare_l_node> left_tree;
//  intrusive_set<compare_r_node> right_tree;
//
//
//
  template <typename Base, typename Pair, typename CompareBase,
            typename ComparePair, typename TagBase, typename TagPair>
  struct base_iterator {
    typename intrusive_tree<CompareBase, TagBase>::iterator it_tree;

    explicit base_iterator(decltype(it_tree) it_tree) : it_tree(it_tree) {}

    using difference_type = ptrdiff_t;
    using value_type = Base;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;

    // Элемент на который сейчас ссылается итератор.
    // Разыменование итератора end_left() неопределено.
    // Разыменование невалидного итератора неопределено.
    Base const& operator*() const {
      return *static_cast<Base *>(*it_tree);
    }
    Base const* operator->() const {
      return static_cast<Base *>(*it_tree);
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
    base_iterator<Pair, Base, ComparePair, CompareBase, TagPair, TagBase> flip() const {
      return {intrusive_tree<ComparePair, TagPair>::iterator(&(*it_tree))};
    }
  };
  using left_iterator = base_iterator<Left, Right, l_comparator_t, r_comparator_t, details::left_tag, details::right_tag>;
  using right_iterator = base_iterator<Right, Left, r_comparator_t, l_comparator_t, details::right_tag, details::left_tag>;

//  void swap(bimap& other) {
//    left_tree.swap(other.left_tree);
//    right_tree.swap(other.right_tree);
//  }

public:

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
      : left_tree(&sentinel, std::move(l_comparator_t(std::move(compare_left)))),
        right_tree(&sentinel, std::move(r_comparator_t(std::move(compare_right)))) {
  }

//
//  // Конструкторы от других и присваивания
//  bimap(bimap const& other)
//      : left_tree(other.left_tree.get_compare()),
//        right_tree(other.right_tree.get_compare()) {
//    for (auto it = other.begin_left(); it != other.end_left(); it++) {
//      this->add(*it, *(it.flip()));
//    }
//  }
//  bimap(bimap&& other) noexcept
//      : left_tree(std::move(other.left_tree)),
//        right_tree(std::move(other.right_tree)) {}
//
//  bimap& operator=(bimap const& other) {
//    if (this != &other)
//      bimap(other).swap(*this);
//    return *this;
//  }
//  bimap& operator=(bimap&& other) noexcept {
//    if (this != &other)
//      bimap(std::move(other)).swap(*this);
//    return *this;
//  }
//
  // Деструктор. Вызывается при удалении объектов bimap.
  // Инвалидирует все итераторы ссылающиеся на элементы этого bimap
  // (включая итераторы ссылающиеся на элементы следующие за последними).
  ~bimap() {
    for (auto it = begin_left(); it != end_left(); it++) {
      delete &(*(it.it_tree));
    }
  }

private:
  left_iterator add(Left left, Right right) {

    auto *new_node = new node_t{std::move(left), std::move(right)};

    typename l_tree_t::iterator iter_left_tree =
        left_tree.insert(*new_node);

    if (iter_left_tree == left_tree.end()) {
      delete new_node;
      return end_left();
    }

    typename r_tree_t ::iterator iter_right_tree =
        right_tree.insert(*new_node);

    if (iter_right_tree == right_tree.end()) {
      left_tree.remove(iter_left_tree);
      delete new_node;
      return end_left();
    }

    return left_iterator(iter_left_tree);
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
//
//  // Удаляет элемент и соответствующий ему парный.
//  // erase невалидного итератора неопределен.
//  // erase(end_left()) и erase(end_right()) неопределены.
//  // Пусть it ссылается на некоторый элемент e.
//  // erase инвалидирует все итераторы ссылающиеся на e и на элемент парный к e.
//  left_iterator erase_left(left_iterator it) {
//    typename Tree<r_node, compare_r_node>::iterator r_it = it.flip().i_cur;
//    delete (*r_it).data;
//    delete (*(it.i_cur)).data;
//    typename Tree<l_node, compare_l_node>::iterator next_it =
//        left_tree.remove(it.i_cur);
//    right_tree.remove(r_it);
//    return {next_it, this};
//  }
//  // Аналогично erase, но по ключу, удаляет элемент если он присутствует, иначе
//  // не делает ничего Возвращает была ли пара удалена
//  bool erase_left(left_t const& left) {
//    left_iterator l_iter = find_left(left);
//    if (l_iter != end_left()) {
//      erase_left(l_iter);
//      return true;
//    }
//    return false;
//  }
//
//  right_iterator erase_right(right_iterator it) {
//    typename Tree<l_node, compare_l_node>::iterator l_it = it.flip().i_cur;
//    delete (*l_it).data;
//    delete (*(it.i_cur)).data;
//    typename Tree<r_node, compare_r_node>::iterator next_it =
//        right_tree.remove(it.i_cur);
//    left_tree.remove(l_it);
//    return {next_it, this};
//  }
//  bool erase_right(right_t const& right) {
//    right_iterator r_iter = find_right(right);
//    if (r_iter != end_right()) {
//      erase_right(r_iter);
//      return true;
//    }
//    return false;
//  }
//
//  // erase от ренжа, удаляет [first, last), возвращает итератор на последний
//  // элемент за удаленной последовательностью
//  left_iterator erase_left(left_iterator first, left_iterator last) {
//    while (first != last)
//      first = erase_left(first);
//
//    return last;
//  }
//  right_iterator erase_right(right_iterator first, right_iterator last) {
//    while (first != last)
//      first = erase_right(first);
//
//    return last;
//  }
//
//  // Возвращает итератор по элементу. Если не найден - соответствующий end()
//  left_iterator find_left(left_t const& left) const {
//    return {left_tree.find(l_node{&left}), this};
//  }
//  right_iterator find_right(right_t const& right) const {
//    return {right_tree.find(r_node{&right}), this};
//  }
//
//  // Возвращает противоположный элемент по элементу
//  // Если элемента не существует -- бросает std::out_of_range
//  right_t const& at_left(left_t const& key) const {
//
//    left_iterator iter(left_tree.find(l_node{&key}), this);
//    if (iter == end_left())
//      throw std::out_of_range("cannot find el");
//    return *(iter.flip());
//  }
//  left_t const& at_right(right_t const& key) const {
//
//    right_iterator iter(right_tree.find(r_node{&key}), this);
//    if (iter == end_right())
//      throw std::out_of_range("cannot find el");
//    return *(iter.flip());
//  }
//
//  // Возвращает противоположный элемент по элементу
//  // Если элемента не существует, добавляет его в bimap и на противоположную
//  // сторону кладет дефолтный элемент, ссылку на который и возвращает
//  // Если дефолтный элемент уже лежит в противоположной паре - должен поменять
//  // соответствующий ему элемент на запрашиваемый (смотри тесты)
//  template <typename = std::enable_if<std::is_default_constructible_v<Right>>>
//  right_t const& at_left_or_default(left_t const& key) {
//    left_iterator l_iter(left_tree.find(l_node{&key}), this);
//    if (l_iter == end_left()) {
//      right_t r_data{};
//      right_iterator r_iter(right_tree.find(r_node{&r_data}), this);
//      if (r_iter == end_right()) {
//        return *(add(key, r_data).flip());
//      } else {
//        left_t* l_data = new left_t(key);
//        delete (*(r_iter.flip().i_cur)).data;
//        (*(r_iter.flip().i_cur)).data = l_data;
//        return *r_iter;
//      }
//    }
//
//    return *(l_iter.flip());
//  }
//  template <typename = std::enable_if<std::is_default_constructible_v<Left>>>
//  left_t const& at_right_or_default(right_t const& key) {
//    right_iterator r_iter(right_tree.find(r_node{&key}), this);
//    if (r_iter == end_right()) {
//      left_t l_data{};
//      left_iterator l_iter(left_tree.find(l_node{&l_data}), this);
//      if (l_iter == end_left()) {
//        left_t const& ll = *add(l_data, key);
//        return ll;
//      } else {
//        right_t* r_data = new right_t(key);
//        delete (*(l_iter.flip().i_cur)).data;
//        (*(l_iter.flip().i_cur)).data = r_data;
//        return *l_iter;
//      }
//    }
//
//    return *(r_iter.flip());
//  }
//
//  // lower и upper bound'ы по каждой стороне
//  // Возвращают итераторы на соответствующие элементы
//  // Смотри std::lower_bound, std::upper_bound.
//  left_iterator lower_bound_left(const left_t& left) const {
//    return {left_tree.find_next(l_node(&left)), this};
//  }
//  left_iterator upper_bound_left(const left_t& left) const {
//    return {left_tree.find_next(l_node(&left)), this};
//  }
//
//  right_iterator lower_bound_right(const right_t& right) const {
//    return {right_tree.find_next(r_node(&right)), this};
//  }
//  right_iterator upper_bound_right(const right_t& right) const {
//    return {right_tree.find_next(r_node(&right)), this};
//  }
//
//  // Проверка на пустоту
//  bool empty() const {
//    assert(left_tree.empty() == right_tree.empty());
//
//    return left_tree.empty();
//  }
//
//  // Возвращает размер бимапы (кол-во пар)
//  std::size_t size() const {
//    assert(left_tree.size() == right_tree.size());
//
//    return left_tree.size();
//  }
//};
//
//// операторы сравнения
//template <typename L, typename R, typename cL, typename cR>
//bool operator==(bimap<L, R, cL, cR> const& a, bimap<L, R, cL, cR> const& b) {
//  if (a.size() != b.size())
//    return false;
//
//  for (auto it_a = a.begin_left(), it_b = b.begin_left();
//       it_a != a.end_left() && it_b != b.end_left(); it_a++, it_b++) {
//    if (*it_a != *it_b)
//      return false;
//    if (*it_a.flip() != *it_b.flip())
//      return false;
//  }
//
//  return true;
//}
//
//template <typename L, typename R, typename cL, typename cR>
//bool operator!=(bimap<L, R, cL, cR> const& a, bimap<L, R, cL, cR> const& b) {
//  return !(a == b);
};
