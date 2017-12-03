
#ifndef SIMPLEDB_DATA_SPSEGMENT_HPP
#define SIMPLEDB_DATA_SPSEGMENT_HPP

#include "Record.hpp"
#include "SlotId.hpp"
#include "SPIterator.hpp"
#include "SPPage.hpp"
#include "SPSlot.hpp"
#include "TID.hpp"

#include "buffer.hpp"
#include "segment.hpp"

#include <cstdint>
#include <memory>
#include <tuple>

namespace simpledb {

    /**
     * Slotted pages segment implementation.
     */
    class SPSegment {
    public:
        using iterator = SPIterator;

        /**
         * Constructs a new slotted pages segment.
         * @param segmentId the segment id
         * @param segmentManager the segment manager
         * @param bufferManager the buffer manager
         */
        SPSegment(uint64_t segmentId, std::shared_ptr<SegmentManager> segmentManager, std::shared_ptr<BufferManager> bufferManager);
        ~SPSegment() = default;

        SPSegment(const SPSegment& orig) = delete;
        SPSegment& operator=(const SPSegment& orig) = delete;

        /**
         * Inserts a new record into the slotted page segment.
         * @param record the record to insert
         * @return the TID identifying the location where the record was stored
         */
        TID insert(const Record& record);

        /**
         * Deletes a record from the slotted page segment.
         * @param tid the TID identifying the record to delete.
         * @return true, if the record did exist; false, otherwise
         */
        bool remove(TID tid);

        /**
         * Looks up the read-only record identified by the supplied TID.
         * @param tid the TID identifying the record to look up.
         * @return the read-only record identified by the supplied TID.
         */
        Record lookup(TID tid);

        /**
         * Updates the record identified by the supplied TID with the data of the supplied record.
         * @param tid the TID identifying the record.
         * @param record the record holding the new data
         * @return true, if the record identified by the did exist; false, otherwise
         */
        bool update(TID tid, const Record& record);

        /**
         * @return iterator to iterate over all slotted page records
         */
        std::unique_ptr<iterator> range();

    private:
        uint64_t _segmentId; // the segment id
        std::shared_ptr<SegmentManager> _segmentManager; // the segment manager
        std::shared_ptr<BufferManager> _bufferManager; // the buffer manager

        /**
         * Searches for a page with enough space to store a record with the supplied size.
         * @param size the needed size in bytes
         * @return a tuple of page id, buffer frame pointer and page pointer
         */
        std::tuple<uint64_t, std::shared_ptr<BufferFrame>, SPPage*> searchFreeSpace(uint64_t size);

        /**
         * Extends a segment to the supplied size.
         * @param size the size in number of pages
         */
        void allocate(uint64_t size);
    };
}

#endif	/* SIMPLEDB_DATA_SPSEGMENT_HPP */

