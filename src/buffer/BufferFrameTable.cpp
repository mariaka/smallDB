
#include "BufferFrameTable.hpp"

namespace simpledb {

    BufferFrameTableBucket& BufferFrameTable::findBucket(PageId pageId) {
        return _table[hash(pageId) & (_table.size() - 1)];
    }

    size_t BufferFrameTable::hash(PageId pageId) {
        return Hash()(reinterpret_cast<void *> (&pageId), sizeof (PageId));
    }
}