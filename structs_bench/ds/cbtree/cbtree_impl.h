#include <algorithm>
#include <iostream>
#include <atomic>
#include <memory>
#include <sstream>

using namespace std;

template <typename K, typename V>
struct CBTreeNode {
    K key;
    V value;
    CBTreeNode* left;
    CBTreeNode* right;
    CBTreeNode* parent;
    int w;
    CBTreeNode(K key, V value): key(key), value(value){
        left = right = parent = nullptr;
        w = 0;
    }
    inline int c() {
        return w - (left == nullptr ? 0 : left->w) - (right == nullptr ? 0 : right->w);
    }
};

template<typename skey_t, typename sval_t, typename Comp = std::less<skey_t>, int rotation_eps=1>
class CBTree {

	public:
    CBTree(skey_t max_key, sval_t no_value): max_key(max_key), no_value(no_value) {
		root = new NodeType(max_key, no_value);
        one_plus_eps = 1 + 1.0 / (1 << rotation_eps);
	}
	sval_t insertIfAbsent(skey_t key, sval_t val) {
		NodeTypePtr parent_node;
        NodeTypePtr node = search(key, &parent_node);
        if (node != nullptr)
            return node->value;
        else {
            node = new NodeType(key, val);
            if (less(key, parent_node))
                parent_node->left = node;
            else
                parent_node->right = node;
            node->w++;
            return this->no_value;
        }
	}
	sval_t find(skey_t x) {
        NodeTypePtr node = search(x, nullptr);
        if (node != nullptr)
            return node->value;
		return this->no_value;
	}
	private:
    typedef CBTreeNode<skey_t, sval_t> NodeType;
    typedef NodeType* NodeTypePtr;
    const skey_t max_key;
    const sval_t no_value;
    NodeTypePtr root;
    double one_plus_eps;
    static bool greater(const skey_t& key, const NodeTypePtr node) {
		return node && Comp()(node->key, key);
	}

	static bool less(const skey_t& key, const NodeTypePtr node) {
		return (node == nullptr) || Comp()(key, node->key);
	}
    static bool equal(const skey_t& key, const NodeTypePtr node) {
		return !Comp()(key, node->key) && !Comp()(node->key, key);
	}
    inline NodeTypePtr get_child(NodeTypePtr node, const skey_t &key, bool &move_left) {
        if (less(key, node))
        {
            move_left = true;
            return node->left;
        }
        else if (greater(key, node))
        {
            move_left = false;
            return node->right;
        }
        else
            return nullptr;
    }
	NodeTypePtr search(skey_t key, NodeTypePtr* parent_storage) {
        NodeTypePtr z = root;
        bool first_move_left = false;
        NodeTypePtr y = get_child(z, key, first_move_left);
        NodeTypePtr z_par = nullptr;
        while (z != nullptr) {
            if (y == nullptr)
            {
                z->w++;
                if (equal(key, z))
                    return z;
                if (parent_storage != nullptr)
                    (*parent_storage) = z;
                return y;
            }
            bool second_move_left = false;
            NodeTypePtr x = get_child(y, key, second_move_left);
            if (x == nullptr) {
                z->w++;
                y->w++;
                if (equal(key, y))
                    return y;
                if (parent_storage != nullptr)
                    (*parent_storage) = y;
                return x;
            }
            if (first_move_left == second_move_left) {
                // Single Rotation
                if (!check_single_rotation(y, z, first_move_left))
                {
                    z->w++;
                    y->w++;
                    z_par = y;
                    z = x;
                    y = get_child(z, key, first_move_left);
                    continue;
                }
                if (first_move_left)
                    rotate_right(y, z, z_par);
                else
                    rotate_left(y, z, z_par);
                z = y;
                y = x;
                first_move_left = second_move_left;
            }
            else {
                if (check_double_rotation(x, y, z, first_move_left))
                {
                    if (first_move_left)
                    {
                        rotate_left(x, y, z);
                        rotate_right(x, z, z_par);
                    }
                    else
                    {
                        rotate_right(x, y, z);
                        rotate_left(x, z, z_par);
                    }
                }
                else
                {
                    z->w++;
                    y->w++;
                    z_par = y;
                }
                z = x;
                y = get_child(z, key, first_move_left);
            }

        }
        assert(false);

    }
    inline bool check_single_rotation(NodeTypePtr y, NodeTypePtr z, bool first_move_left) {
        if (z == root)
            return false;
        if (first_move_left)
            return (z->w + (y->right == nullptr ? 0: y->right->w)) < y->w * one_plus_eps;
        else
            return (z->w + (y->left == nullptr ? 0: y->left->w)) < y->w * one_plus_eps;
    }
    inline bool check_double_rotation(NodeTypePtr x, NodeTypePtr y, NodeTypePtr z, bool first_move_left) {
        if (z == root)
            return false;
        if (first_move_left)
            return ((z->w + (x->right == nullptr ? 0: x->right->w)) +
                (y->w + (x->left == nullptr ? 0: x->left->w))) < (y->w + x->w) * one_plus_eps;
        else
            return ((z->w + (x->left == nullptr ? 0: x->left->w)) +
                (y->w + (x->right == nullptr ? 0: x->right->w))) < (y->w + x->w) * one_plus_eps;
    }
    inline NodeTypePtr rotate_right(NodeTypePtr x, NodeTypePtr y, NodeTypePtr z) {
        int y_new_w = y->w - x->w + (x->right == nullptr ? 0 : x->right->w);
        x->w = y->w;
        y->w = y_new_w;
        y->left = x->right;
        x->right = y;
        if (z->left == y)
            z->left = x;
        else
            z->right = x;
    }

    inline NodeTypePtr rotate_left(NodeTypePtr x, NodeTypePtr y, NodeTypePtr z) {
        int y_new_w = y->w - x->w + (x->left == nullptr ? 0 : x->left->w);
        y->right = x->left;
        x->left = y;
        if (z->left == y)
            z->left = x;
        else
            z->right = x;
    }
};