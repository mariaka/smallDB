
#ifndef SIMPLEDB_BPLUSTREE_LEAFNODE_HPP
#define SIMPLEDB_BPLUSTREE_LEAFNODE_HPP

#include "InnerNode.hpp"
#include "Node.hpp"

#include "buffer.hpp"
#include "data/TID.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <ios>
#include <ostream>

namespace simpledb {

    template <typename K, typename V, typename C, uint64_t DEGREE>
    class LeafNode : public Node<K, V, C> {
    public:
        LeafNode() = delete;
        ~LeafNode() = delete;

        LeafNode(const LeafNode& orig) = delete;
        LeafNode& operator=(const LeafNode& orig) = delete;

        void init();

        void insert(K key, V value);
        V lookup(K key);
        int64_t lookupIndex(K key);
        bool erase(K key);

        uint64_t size();
        bool hasFreeSpace();
        PageId nextPage();

        K& key(uint64_t k);
        V& value(uint64_t k);

        template<uint64_t INNER_DEGREE>
        void split(PageId newPageId, LeafNode<K, V, C, DEGREE> *newNode, InnerNode<K, V, C, INNER_DEGREE> *parentNode);

        void visualize(PageId thisPageId, std::shared_ptr<BufferFrame> bufferFrame, std::ostream& out, BufferManager& bufferManager);

    private:
        using KeyArray = std::array<K, 2 * DEGREE>;
        using ValueArray = std::array<V, 2 * DEGREE>;

        PageId _next; // next page or invalid page id
        uint64_t _count; // number of keys

        KeyArray& keys() {
            return *reinterpret_cast<KeyArray*> (reinterpret_cast<char *> (this) + sizeof (*this));
        }

        typename KeyArray::iterator keysEnd() {
            return (keys().begin() + _count);
        }

        ValueArray& values() {
            return *reinterpret_cast<ValueArray *> (reinterpret_cast<char *> (this) + sizeof (*this) + sizeof (KeyArray));
        }

        typename ValueArray::iterator valuesEnd() {
            return (values().begin() + _count);
        }
    };
}

#include "LeafNode.tpp"

#endif	/* SIMPLEDB_BPLUSTREE_LEAFNODE_HPP */
