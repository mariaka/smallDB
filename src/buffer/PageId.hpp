
#ifndef SIMPLEDB_BUFFER_PAGEID_HPP
#define	SIMPLEDB_BUFFER_PAGEID_HPP

#include <cstdint>

namespace simpledb {

    /**
     * PageId encapsulates the segment id, identifiying the file on disk, and
     * page id, identifiying the page in the file on disk.
     */
    struct PageId {
        uint64_t segment;
        uint64_t page;

        PageId(uint64_t segment, uint64_t page) : segment(segment), page(page) {
        };

        PageId() : PageId(0, 0) {
        };

        bool operator==(const PageId &other) const {
            return (this->segment == other.segment && this->page == other.page);
        };

        bool isValid() const {
            return (segment != 0);
        }
    };
}

#endif	/* SIMPLEDB_BUFFER_PAGEID_HPP */

