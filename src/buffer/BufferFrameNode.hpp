
#ifndef SIMPLEDB_BUFFER_BUFFERFRAMENODE_HPP
#define	SIMPLEDB_BUFFER_BUFFERFRAMENODE_HPP

namespace simpledb {
    class BufferFrame;
}

#include "PageId.hpp"

namespace simpledb {

    /**
     * Node for a buffer frame in replacement manager queues.
     */
    struct BufferFrameNode {
        bool inUse; // true, if the memory region of data is used
        PageId page; // identifies segment and page on disk
        void *data; // pointer to memory region
        BufferFrame* frame; // pointer to the corresponding buffer frame
        // TODO: consolidate frame with inUse?

        BufferFrameNode(bool inUse, PageId page, void* data, BufferFrame* frame) : inUse(inUse), page(page), data(data), frame(frame) {
        };
    };
}

#endif	/* SIMPLEDB_BUFFER_BUFFERFRAMENODE_HPP */

