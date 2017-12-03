
#ifndef SIMPLEDB_BPLUSTREE_NODE_HPP
#define SIMPLEDB_BPLUSTREE_NODE_HPP

//#include <cstdint>

namespace simpledb {

    template <typename K, typename V, typename C>
    class Node {
    public:
        Node() = delete;
        ~Node() = delete;

        Node(const Node& orig) = delete;
        Node& operator=(const Node& orig) = delete;

        bool isLeaf() const {
            return _isLeaf;
        }

    protected:
        // uint64_t _lsn; // LSN
        bool _isLeaf;

    private:
    };
}

#include "Node.tpp"

#endif	/* SIMPLEDB_BPLUSTREE_NODE_HPP */

