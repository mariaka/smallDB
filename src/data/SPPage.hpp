
#ifndef SIMPLEDB_DATA_SPPAGE_HPP
#define	SIMPLEDB_DATA_SPPAGE_HPP

#include "Record.hpp"
#include "SlotId.hpp"
#include "SPSlot.hpp"
#include "TID.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
//#include <iostream>
#include <tuple>

namespace simpledb {

    /**
     * Slotted page implementation.
     */
    class SPPage {
    public:
        SPPage() = delete;
        ~SPPage() = delete;

        SPPage(const SPPage& orig) = delete;
        SPPage& operator=(const SPPage& orig) = delete;

        /**
         * Initializes the page.
         * @param pageSize the size of the page
         */
        void init(uint64_t pageSize);

        /**
         * @param slotId the slot id
         * @return the pointer to the slot identified by the supplied slot id
         */
        SPSlot* getSlot(SlotId slotId) {
            assert(slotId < _firstFreeSlot);
            return (reinterpret_cast<SPSlot *> (reinterpret_cast<char *> (this) + sizeof (SPPage) + slotId * sizeof (SPSlot)));
        }

        /**
         * @param slot the slot
         * @return the pointer to the data of a slot
         */
        char* getItem(SPSlot* slot) {
            return (reinterpret_cast<char *> (this) + slot->offset());
        }

        /**
         * @param slot the slot
         * @return the length of the data of a slot
         */
        uint64_t getLength(SPSlot* slot) {
            return (slot->length());
        }

        /**
         * Must only be called if the slot is a redirect (isOnPage == false).
         * @param slot the slot
         * @return the TID of the redirected record of a slot
         */
        TID getRedirectedTID(SPSlot* slot) {
            assert(!slot->isOnPage() && !slot->isRedirected());
            assert(slot->length() == sizeof (TID));

            return (*(reinterpret_cast<TID *> (getItem(slot))));
        }

        /**
         * Checks for free space for a new entry on the page.
         * @param size the needed space in bytes
         * @return true, if there is a enough space to insert the new entry on the page; false, otherwise
         */
        bool hasFreeSpace(uint64_t size);

        /**
         * Checks if the page has enough free space to update an entry on page.
         * @param size the needed space in bytes
         * @return true, if there is enough free space to update an entry on page; false, otherwise
         */
        bool hasUpdateSpace(uint64_t size);

        /**
         * Reserves the next free slot and returns information about it.
         * @return a tuple of the slot id and slot pointer
         */
        std::tuple<uint64_t, SPSlot*> nextSlot();

        /**
         * Insert a new entry into the page.
         * @param record the new record
         * @param redirected indicates if the entry is a redirected entry
         * @param originalTid if the entry is a redirected entry, the original TID
         * @return the slot id where the entry was inserted
         */
        SlotId insert(const Record& record, bool redirected = false, TID originalTid = TID(PageId(0, 0), 0));

        /**
         * Removes an entry from the page by updating the slot accordingly.
         * @param slotId the slot id
         */
        void remove(SlotId slotId);

        /**
         * Updates a record in place by copying the data to space of the old record and updating the slot.
         * @param slotId the slot id
         * @param record the new record
         */
        void updateInPlace(SlotId slotId, const Record& record);

        /**
         * Updates a record on page by copying the data to new space on the page and updating the slot.
         * @param slotId the slot id
         * @param record the new record
         */
        void updateOnPage(SlotId slotId, const Record& record);

        uint64_t firstFreeSlot() const {
            return _firstFreeSlot;
        }

    private:
        static constexpr uint64_t MINIMAL_RECORD_LENGTH = sizeof (TID); // minimal record length

        // uint64_t _lsn;
        uint64_t _slotCount; // the number of used slots on the page
        uint64_t _firstFreeSlot; // the number of the next free slot
        uint64_t _dataStart; // offset to the beginning of the data block
        uint64_t _freeSpace; // the free space on the page in bytes
    };
}

#endif	/* SIMPLEDB_DATA_SPPAGE_HPP */

