
#ifndef SIMPLEDB_DATA_SPSLOT_HPP
#define	SIMPLEDB_DATA_SPSLOT_HPP

#include <cstdint>

namespace simpledb {

    /**
     * Slot implementation for slotted pages.
     */
    class SPSlot {
    public:
        SPSlot() = delete;
        ~SPSlot() = delete;

        SPSlot(const SPSlot& orig) = delete;
        SPSlot& operator=(const SPSlot& orig) = delete;

        /**
         * @return true, if the item addressed by the slot is on the current page;
         * false, if it was redirected (then the data of the item contains the TID of the redirect)
         */
        bool isOnPage() const {
            return _onPage;
        }

        /**
         * @return true, if the item was redirected from another page
         * (then the original TID is prepended to the data item); false, otherwise
         */
        bool isRedirected() const {
            return _redirected;
        }

        /**
         * @return true, if the slot is free and doesn't point to a data item
         */
        bool isFree() const {
            return _offset == 0 && _length == 0;
        }

        /**
         * @return the offset of the data item relative to the beginning of the page
         */
        uint64_t offset() const {
            return _offset;
        }

        /**
         * @return the length of the data item
         */
        uint64_t length() const {
            return _length;
        }

        void setOnPage(bool onPage) {
            _onPage = onPage;
        }

        void setRedirected(bool redirected) {
            _redirected = redirected;
        }

        void setOffset(uint64_t offset) {
            _offset = offset;
        }

        void setLength(uint64_t length) {
            _length = length;
        }

    private:
        bool _onPage;
        bool _redirected;
        uint64_t _offset;
        uint64_t _length;
    };
}

#endif	/* SIMPLEDB_DATA_SPSLOT_HPP */
