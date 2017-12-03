
#ifndef SIMPLEDB_BPLUSTREE_ROOTNODE_HPP
#define SIMPLEDB_BPLUSTREE_ROOTNODE_HPP

#include "buffer.hpp"

#include <cstdint>

namespace simpledb {

    class MetaNode {
    public:
        MetaNode() = delete;
        ~MetaNode() = delete;

        MetaNode(const MetaNode& orig) = delete;
        MetaNode& operator=(const MetaNode& orig) = delete;

        void init(uint64_t nextFreePageId);
        uint64_t nextFreePageId();

    private:
        uint64_t _nextFreePageId;
    };

    static_assert(sizeof (MetaNode) <= BufferManager::PAGE_SIZE, "");
}

#endif	/* SIMPLEDB_BPLUSTREE_ROOTNODE_HPP */

