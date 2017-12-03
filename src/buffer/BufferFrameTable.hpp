
#ifndef SIMPLEDB_BUFFER_BUFFERFRAMETABLE_HPP
#define	SIMPLEDB_BUFFER_BUFFERFRAMETABLE_HPP

#include "BufferFrame.hpp"
#include "BufferFrameTableBucket.hpp"

#include "hash.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace simpledb {

    /**
     * Hash table for alle buffer frames in memory identified by the page id.
     */
    class BufferFrameTable {
    public:

        /**
         * @param size number of frames to manage
         */
        BufferFrameTable(uint64_t size) : _table(exp2(static_cast<uint64_t> (log2(TABLE_OVERHEAD_FACTOR * size - 1) + 1))) {
            assert((_table.size() & (_table.size() - 1)) == 0); // size is power of 2

        }

        ~BufferFrameTable() {
        }

        BufferFrameTable(const BufferFrameTable& orig) = delete;
        BufferFrameTable& operator=(const BufferFrameTable& orig) = delete;

        /**
         * Finds the correct bucket for a pageId.
         * @param pageId
         * @return reference to the correct bucket
         */
        BufferFrameTableBucket& findBucket(PageId pageId);

        std::vector<BufferFrameTableBucket>::iterator begin() {
            return _table.begin();
        }

        std::vector<BufferFrameTableBucket>::iterator end() {
            return _table.end();
        }

    private:
        static constexpr int TABLE_OVERHEAD_FACTOR = 3; // scaling factor for table size

        std::vector<BufferFrameTableBucket> _table; // vector of buckets

        /**
         * Calculates a hash value for a page id.
         * @param pageId
         * @return the hash value for the page id
         */
        size_t hash(PageId pageId);
    };
}
#endif	/* SIMPLEDB_BUFFER_BUFFERFRAMETABLE_HPP */

