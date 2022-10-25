#include "tree.h"

Node::Node(Node* parent) : parent(parent) {}

Node::~Node() {
  unlink();
}

bool Node::is_right() {
  if (parent->rght == this)
    return true;
  return false;
}

void Node::unlink() {
  if (parent) {
    if (is_right())
      parent->rght = nullptr;
    else
      parent->left = nullptr;
  }
}

Node* Node::min_node(Node* cur) {
  while (cur->left)
    cur = cur->left;
  return cur;
}

Node* Node::max_node(Node* cur) {
  while (cur->rght)
    cur = cur->rght;
  return cur;
}

Node* Node::next_node(Node* cur) {
  if (cur->rght)
    return min_node(cur->rght);
  Node* next = cur->parent;
  while (next->parent != nullptr && cur == next->rght) {
    cur = next;
    next = next->parent;
  }
  return next;
}

Node* Node::prev_node(Node* cur) {
  if (cur->left)
    return max_node(cur->left);
  Node* prev = cur->parent;
  while (prev->parent != nullptr && cur == prev->left) {
    cur = prev;
    prev = prev->parent;
  }
  return prev;
}

Node* Node::next() {
  return next_node(this);
}

Node* Node::prev() {
  return prev_node(this);
}
