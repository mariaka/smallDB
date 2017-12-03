
#ifndef SIMPLEDB_DATA_SLOTID_HPP
#define SIMPLEDB_DATA_SLOTID_HPP

#include <cstdint>

namespace simpledb {
    /**
     * Slot Id to address a slot of a slotted page.
     */
    using SlotId = uint64_t;
}

#endif	/* SIMPLEDB_DATA_SLOTID_HPP */
