#pragma once

#include <cmath>
#include <vector>

class ScapegoatTree
{
    struct Node
    {
        int key;
        std::size_t subtree_size = 1;
        Node * left = nullptr;
        Node * right = nullptr;
        Node * parent = nullptr;

        Node() = default;

        Node(int key)
            : key(key)
        {
        }

        ~Node()
        {
            delete left;
            delete right;
        }
    };

    Node * m_root = nullptr;
    std::size_t m_tree_size = 0;
    std::size_t m_tree_max_size = 0;
    const double m_alpha = 0.6;

    static unsigned h_alpha(std::size_t size, double alpha);
    static Node * search(Node * start, int key);
    static Node * insert_node(Node * node, Node * new_node, std::size_t & cur_depth);
    static Node * find_scapegoat(Node * start, double alpha);
    static void remove_node(Node * node, Node *& root);
    static void get_values(Node * start, std::vector<int> & current);
    static void retie(Node * new_parent, Node *& place_to_set, Node * new_child);
    static Node *& parent_or_root(Node * node, Node *& root);
    static Node * flatten_tree(Node * start, Node * head);
    static Node * build_balanced_tree(Node * head, std::size_t size);
    static void rebuild_tree(Node * scapegoat, Node *& root);

public:
    ScapegoatTree() = default;
    ScapegoatTree(double alpha);

    bool contains(int value) const;
    bool insert(int value);
    bool remove(int value);

    std::size_t size() const;
    bool empty() const;

    std::vector<int> values() const;

    ~ScapegoatTree()
    {
        delete m_root;
    }
};
