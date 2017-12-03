
#ifndef SIMPLEDB_DATA_TID_HPP
#define SIMPLEDB_DATA_TID_HPP

#include "SlotId.hpp"

#include "buffer/PageId.hpp"

#include <functional>

namespace simpledb {

    /**
     * The TID unqiuely identifies a tuple.
     */
    class TID {
    public:

        /**
         * Constructs a new tuple id.
         * @param pageId the page id
         * @param slotId the slot id
         */
        TID(PageId pageId, SlotId slotId);
        ~TID() = default;

        TID(const TID& orig) = default;
        TID& operator=(const TID& orig) = default;

        /**
         * @return the page id
         */
        PageId pageId() const {
            return _pageId;
        }

        /**
         * Sets the page id.
         * @param pageId the page id
         */
        void setPageId(PageId pageId) {
            _pageId = pageId;
        }

        /**
         * @return the slot id
         */
        SlotId slotId() const {
            return _slotId;
        }

        /**
         * Sets the slot id.
         * @param slotId the slot id
         */
        void setSlotId(SlotId slotId) {
            _slotId = slotId;
        }

        /**
         * Compares the TID to another TID.
         * Two TIDs are considered equal, if the have the same page id and slot id, i.e. the address the same tuple.
         * @param other the other TID
         * @return true, if the TIDs are equal; false, otherwise
         */
        bool operator==(const TID &other) const {
            return (
                    this->_pageId == other._pageId &&
                    this->_slotId == other._slotId
                    );
        };

    private:
        PageId _pageId; // the page id
        SlotId _slotId; // the slot id
    };
}

namespace std {

    template<>
    struct hash<simpledb::TID> {

        /**
         * Simple hash implementation for the TID.
         * @param tid the TID
         * @return a hash value for the TID
         */
        size_t operator()(const simpledb::TID& tid) const {
            return (tid.pageId().segment + tid.pageId().page + tid.slotId());
        }
    };
}

#endif	/* SIMPLEDB_DATA_TID_HPP */
