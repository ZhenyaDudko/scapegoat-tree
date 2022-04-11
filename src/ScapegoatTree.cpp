#include "ScapegoatTree.h"

#include <stdexcept>

ScapegoatTree::ScapegoatTree(double alpha)
    : m_alpha(alpha)
{
    if (alpha < 0.5 || alpha > 1) {
        throw std::invalid_argument("Invalid alpha for a tree constructor");
    }
}

std::size_t ScapegoatTree::size() const
{
    return m_tree_size;
}

bool ScapegoatTree::empty() const
{
    return m_tree_size == 0;
}

bool ScapegoatTree::contains(int value) const
{
    return search(m_root, value) != nullptr;
}

ScapegoatTree::Node * ScapegoatTree::search(Node * node, int value)
{
    if (node == nullptr || node->key == value) {
        return node;
    }
    if (value < node->key) {
        return search(node->left, value);
    }
    return search(node->right, value);
}

unsigned ScapegoatTree::h_alpha(std::size_t size, double alpha)
{
    return std::log(size) / std::log(1 / alpha);
}

bool ScapegoatTree::insert(int value)
{
    if (contains(value)) {
        return false;
    }
    std::size_t new_node_depth = 0;
    Node * new_node = new Node(value);
    m_root = insert_node(m_root, new_node, new_node_depth);
    m_tree_size++;
    m_tree_max_size = std::max(m_tree_size, m_tree_max_size);
    if (new_node_depth > h_alpha(m_tree_size, m_alpha)) {
        Node * scapegoat = find_scapegoat(new_node, m_alpha);
        rebuild_tree(scapegoat, m_root);
        if (scapegoat == m_root) {
            m_tree_max_size = m_tree_size;
        }
    }
    return true;
}

ScapegoatTree::Node * ScapegoatTree::insert_node(Node * node, Node * new_node, std::size_t & cur_depth)
{
    if (node == nullptr) {
        return new_node;
    }
    node->subtree_size++;
    if (new_node->key < node->key) {
        retie(node, node->left, insert_node(node->left, new_node, ++cur_depth));
    }
    else {
        retie(node, node->right, insert_node(node->right, new_node, ++cur_depth));
    }
    return node;
}

ScapegoatTree::Node * ScapegoatTree::find_scapegoat(Node * node, double alpha)
{
    std::size_t height = 0;
    while (node != nullptr) {
        if (height > h_alpha(node->subtree_size, alpha)) {
            break;
        }
        height++;
        node = node->parent;
    }
    return node;
}

bool ScapegoatTree::remove(int value)
{
    Node * node = search(m_root, value);
    if (node == nullptr) {
        return false;
    }
    remove_node(node, m_root);
    m_tree_size--;
    if (m_tree_size < m_alpha * m_tree_max_size) {
        rebuild_tree(m_root, m_root);
        m_tree_max_size = m_tree_size;
    }
    return true;
}

ScapegoatTree::Node *& ScapegoatTree::parent_or_root(Node * node, Node *& root)
{
    if (node->parent == nullptr) {
        return root;
    }
    if (node->parent->left != nullptr && node->parent->left->key == node->key) {
        return node->parent->left;
    }
    return node->parent->right;
}

void ScapegoatTree::retie(Node * new_parent, Node *& place_to_set, Node * new_child)
{
    if (new_child != nullptr) {
        new_child->parent = new_parent;
    }
    place_to_set = new_child;
}

void ScapegoatTree::remove_node(Node * node, Node *& root)
{
    if (node->left != nullptr && node->right != nullptr) {
        Node * start = node->right;
        while (start->left != nullptr) {
            start = start->left;
        }
        node->key = start->key;
        remove_node(start, root);
        return;
    }
    Node * start = node;
    while (start->parent != nullptr) {
        start->parent->subtree_size--;
        start = start->parent;
    }
    if (node->subtree_size == 1) {
        retie(node->parent, parent_or_root(node, root), nullptr);
    }
    else if (node->left == nullptr) {
        retie(node->parent, parent_or_root(node, root), node->right);
    }
    else if (node->right == nullptr) {
        retie(node->parent, parent_or_root(node, root), node->left);
    }
    node->left = nullptr;
    node->right = nullptr;
    delete node;
}

std::vector<int> ScapegoatTree::values() const
{
    std::vector<int> values;
    values.reserve(m_tree_size);
    get_values(m_root, values);
    return values;
}

void ScapegoatTree::get_values(Node * node, std::vector<int> & values)
{
    if (node == nullptr) {
        return;
    }
    get_values(node->left, values);
    values.emplace_back(node->key);
    get_values(node->right, values);
}

ScapegoatTree::Node * ScapegoatTree::flatten_tree(Node * node, Node * head)
{
    if (node == nullptr) {
        return head;
    }
    node->right = flatten_tree(node->right, head);
    return flatten_tree(node->left, node);
}

ScapegoatTree::Node * ScapegoatTree::build_balanced_tree(Node * head, std::size_t size)
{
    if (size == 0) {
        head->left = nullptr;
        head->subtree_size = 1;
        return head;
    }
    Node * r_node = build_balanced_tree(head, size / 2);
    Node * s_node = build_balanced_tree(r_node->right, size - size / 2 - 1);
    retie(r_node, r_node->right, s_node->left);
    if (s_node->left != nullptr) {
        s_node->subtree_size -= s_node->left->subtree_size;
        r_node->subtree_size += s_node->left->subtree_size;
    }
    retie(s_node, s_node->left, r_node);
    s_node->subtree_size += r_node->subtree_size;
    return s_node;
}

void ScapegoatTree::rebuild_tree(Node * scapegoat, Node *& root)
{
    if (scapegoat == nullptr) {
        return;
    }
    Node temp_node;
    Node * head = flatten_tree(scapegoat, &temp_node);
    Node * parent = scapegoat->parent;
    Node *& place_to_set = parent_or_root(scapegoat, root);
    build_balanced_tree(head, scapegoat->subtree_size);
    retie(parent, place_to_set, temp_node.left);
    temp_node.left = nullptr;
}
