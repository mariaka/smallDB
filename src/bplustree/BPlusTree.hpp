
#ifndef SIMPLEDB_BPLUSTREE_BTREE_HPP
#define SIMPLEDB_BPLUSTREE_BTREE_HPP

#include "BPlusTreeIterator.hpp"
#include "InnerNode.hpp"
#include "LeafNode.hpp"
#include "MetaNode.hpp"
#include "Node.hpp"

#include "buffer.hpp"
#include "segment.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>
#include <utility>

namespace simpledb {

    template <typename K, typename V, typename C = std::less<K>>
    class BPlusTree {
    public:
        using key_type = K;
        using value_type = V;
        using size_type = uint64_t;
        using comparator = C;

        static constexpr uint64_t INNER_DEGREE = (BufferManager::PAGE_SIZE - sizeof (InnerNode<K, V, C, 0>) - sizeof (PageId)) / (2 * (sizeof (K) + sizeof (PageId)));
        static_assert(sizeof (InnerNode<K, V, C, 0>) + 2 * INNER_DEGREE * sizeof (K) + (2 * INNER_DEGREE + 1) * sizeof (PageId) <= BufferManager::PAGE_SIZE, "");
        //static constexpr uint64_t INNER_DEGREE = 2; // = k

        static constexpr uint64_t LEAF_DEGREE = (BufferManager::PAGE_SIZE - sizeof (LeafNode<K, V, C, 0>)) / (2 * (sizeof (K) + sizeof (V)));
        static_assert(sizeof (LeafNode<K, V, C, 0>) + 2 * LEAF_DEGREE * sizeof (K) + 2 * LEAF_DEGREE * sizeof (V) <= BufferManager::PAGE_SIZE, "");
        //static constexpr uint64_t LEAF_DEGREE = 2;

        using iterator = BPlusTreeIterator<BPlusTree<K, V, C>, false>;
        using exclusive_iterator = BPlusTreeIterator<BPlusTree<K, V, C>, true>;

        /**
         * Constructs a new b+ tree segment.
         * @param segmentId the segment id
         * @param segmentManager the segment manager
         * @param bufferManager the buffer manager
         */
        BPlusTree(uint64_t segmentId, std::shared_ptr<SegmentManager> segmentManager, std::shared_ptr<BufferManager> bufferManager);
        ~BPlusTree() = default;

        BPlusTree(const BPlusTree& orig) = delete;
        BPlusTree& operator=(const BPlusTree& orig) = delete;

        /**
         * Inserts a new key-value par in the b+tree.
         * @param key the key
         * @param value the value
         */
        void insert(K key, V value);

        /**
         * Looks up the corresponding value to the supplied key.
         * @param key the key
         * @return the corresponding value, or the default value, if the key does not exist
         */
        V lookup(K key);

        /**
         * @param key the key
         * @return returns an iterator to iterate from the given key to the end of the tree
         */
        iterator lookupRange(K key);

        /**
         * Deletes a key-value pair from the b+tree.
         * @param key the key
         * @return true, if the key was found and the key-value pair deleted; false, otherwise
         */
        bool erase(K key);

        /**
         * @return the number of entries in the b+tree
         */
        size_type size();

        /**
         * Outputs a visual representation of the b+tree in DOT format.
         * @param out the output stream.
         */
        void visualize(std::ostream& out);

    private:
        static constexpr uint64_t META_PAGE_ID = 0;
        static constexpr uint64_t ROOT_PAGE_ID = 1;
        static constexpr uint64_t FIRST_LEAF_PAGE_ID = 2;

        static constexpr uint64_t MIN_PAGE_NUMBER = 3;

        uint64_t _segmentId; // the segment id
        std::shared_ptr<SegmentManager> _segmentManager; // the segment manager
        std::shared_ptr<BufferManager> _bufferManager; // the buffer manager

        std::shared_ptr<BufferFrame> lookupPage(K key, bool exclusive, bool leftmost);
        void allocate(uint64_t size);
        PageId newPage();
    };
}

#include "BPlusTree.tpp"

#endif	/* SIMPLEDB_BPLUSTREE_BTREE_HPP */
