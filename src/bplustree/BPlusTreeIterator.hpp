
#ifndef SIMPELDB_BPLUSTREE_BPLUSTREEITERATOR_HPP
#define SIMPELDB_BPLUSTREE_BPLUSTREEITERATOR_HPP

#include "LeafNode.hpp"

#include "buffer.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>

namespace simpledb {

    template<typename B, bool EXCLUSIVE>
    class BPlusTreeIterator {
    public:
        using value_type = typename B::value_type;

        BPlusTreeIterator(int64_t onPageIt, std::shared_ptr<BufferFrame> bufferFrame, std::shared_ptr<BufferManager> bufferManager);
        ~BPlusTreeIterator();

        BPlusTreeIterator(const BPlusTreeIterator& orig) = delete;
        BPlusTreeIterator& operator=(const BPlusTreeIterator& orig) = delete;

        BPlusTreeIterator(BPlusTreeIterator&& orig) = default;
        BPlusTreeIterator& operator=(BPlusTreeIterator&& orig) = default;

        void dirty();
        bool isValid() const;

        std::pair<typename B::key_type, typename B::value_type> operator*() const;
        //value_type& operator*() const;
        //value_type* operator->() const;
        BPlusTreeIterator<B, EXCLUSIVE>& operator++();

    private:
        using leaf_type = LeafNode<typename B::key_type, typename B::value_type, typename B::comparator, B::LEAF_DEGREE>;

        std::shared_ptr<BufferManager> _bufferManager;
        std::shared_ptr<BufferFrame> _bufferFrame;
        bool _dirty;
        int64_t _onPageIt;

        leaf_type* leaf() const {
            return reinterpret_cast<leaf_type*> (_bufferFrame->getData());
        }
    };
}

#include "BPlusTreeIterator.tpp"

#endif	/* SIMPELDB_BPLUSTREE_BPLUSTREEITERATOR_HPP */

