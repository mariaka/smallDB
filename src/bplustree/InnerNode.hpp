
#ifndef SIMPLEDB_BPLUSTREE_INNERNODE_HPP
#define SIMPLEDB_BPLUSTREE_INNERNODE_HPP

#include "Node.hpp"

#include "buffer.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <ios>
#include <ostream>
#include <memory>

namespace simpledb {

    template <typename K, typename V, typename C, uint64_t DEGREE>
    class LeafNode;

    template <typename K, typename V, typename C, uint64_t DEGREE>
    class InnerNode : public Node<K, V, C> {
    public:
        InnerNode() = delete;
        ~InnerNode() = delete;

        InnerNode(const InnerNode & orig) = delete;
        InnerNode& operator=(const InnerNode & orig) = delete;

        void init();
        void init(PageId firstChild);

        void insert(K key, PageId pageId);
        PageId lookup(K key);
        PageId leftmost();

        uint64_t size();
        bool hasFreeSpace();

        void splitRoot(PageId pageId[2], InnerNode<K, V, C, DEGREE> *node[2]);
        void split(PageId newPageId, InnerNode<K, V, C, DEGREE> *newNode, InnerNode<K, V, C, DEGREE> *parentNode);

        template<uint64_t LEAF_DEGREE>
        void visualize(PageId thisPageId, std::shared_ptr<BufferFrame> bufferFrame, std::ostream& out, BufferManager& bufferManager, LeafNode<K, V, C, LEAF_DEGREE> *unused);

    private:
        using KeyArray = std::array<K, 2 * DEGREE>;
        using ChildrenArray = std::array<PageId, 2 * DEGREE + 1 >;

        uint64_t _count; // number of keys

        KeyArray& keys() {
            return *reinterpret_cast<KeyArray*> (reinterpret_cast<char *> (this) + sizeof (*this));
        }

        typename KeyArray::iterator keysEnd() {
            return (keys().begin() + _count);
        }

        ChildrenArray& children() {
            return *reinterpret_cast<ChildrenArray *> (reinterpret_cast<char *> (this) + sizeof (*this) + sizeof (KeyArray));
        }

        typename ChildrenArray::iterator childrenEnd() {
            return (children().begin() + _count + 1);
        }
    };
}

#include "InnerNode.tpp"

#endif	/* SIMPLEDB_BPLUSTREE_INNERNODE_HPP */
