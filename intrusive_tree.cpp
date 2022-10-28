//#include "intrusive_tree.h"
//
//#include <cassert>
//
//using namespace intrusive;
//
//node_base::node_base(node_base* parent) : parent(parent) {}
//
//node_base::~node_base() {
//  unlink();
//}
//
//bool node_base::is_right() {
//  return parent->right == this;
//}
//
////void node_base::set_child(node_base *child, node_base* set)
////{
////  assert(right == child || left == child);
////  if(right == child)
////    right = set;
////  else
////    left = set;
////  set->parent = this;
////}
//
//void node_base::unlink_parent(node_base* set)
//{
//  if (is_right())
//    parent->right = set;
//  else
//    parent->left = set;
//  set->parent = parent;
//}
//
//void node_base::unlink() {
//  if (parent) {
//    if(right != nullptr && left == nullptr)
//    {
//      unlink_parent(right);
//      //parent->set_child(this, right);
//      right = nullptr;
//    }
//    else if(right == nullptr && left != nullptr)
//    {
//      unlink_parent(left);
//      //parent->set_child(this, left);
//      left = nullptr;
//    }
//    else
//    {
//      node_base *next = right;
//      next = min_node(next);
//      next->unlink();
//      this->swap(*next);
//    }
//    parent = nullptr;
//  }
//}
//
//node_base* node_base::min_node(node_base* cur) {
//  while (cur->left)
//    cur = cur->left;
//  return cur;
//}
//
//node_base* node_base::max_node(node_base* cur) {
//  while (cur->right)
//    cur = cur->right;
//  return cur;
//}
//
//node_base* node_base::next_node(node_base* cur) {
//  if (cur->right)
//    return min_node(cur->right);
//  node_base* next = cur->parent;
//  while (next->parent != nullptr && cur == next->right) {
//    cur = next;
//    next = next->parent;
//  }
//  return next;
//}
//
//node_base* node_base::prev_node(node_base* cur) {
//  if (cur->left)
//    return max_node(cur->left);
//  node_base* prev = cur->parent;
//  while (prev->parent != nullptr && cur == prev->left) {
//    cur = prev;
//    prev = prev->parent;
//  }
//  return prev;
//}
//
//node_base* node_base::next() {
//  return next_node(this);
//}
//
//node_base* node_base::prev() {
//  return prev_node(this);
//}
//
//void node_base::swap(node_base& other) {
//  std::swap(parent, other.parent);
//  std::swap(left, other.left);
//  std::swap(right, other.right);
//}