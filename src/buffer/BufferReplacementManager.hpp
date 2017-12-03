
#ifndef SIMPLEDB_BUFFER_BUFFERREPLACEMENTMANAGER_HPP
#define	SIMPLEDB_BUFFER_BUFFERREPLACEMENTMANAGER_HPP

#include "BufferFrame.hpp"
#include "BufferFrameNode.hpp"
#include "PageId.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_types.hpp>

#include <list>
#include <memory>

namespace simpledb {

    /**
     * Implements the buffer frame replacement strategy: 2Q
     * All operations are synchronized internally.
     */
    class BufferReplacementManager {
    public:

        /**
         * Creates a new buffer replacement manager.
         * Initializes frame nodes for the supplied memory buffer.
         * @param size number of frames
         * @param pageSize the size of one frame in bytes
         * @param buffer the memory region for the frames in memory
         */
        BufferReplacementManager(uint64_t size, long pageSize, void *buffer);

        ~BufferReplacementManager() {
        };

        BufferReplacementManager(const BufferReplacementManager& orig) = delete;
        BufferReplacementManager& operator=(const BufferReplacementManager& orig) = delete;

        /**
         * Evicts the next frame.
         * Deletes the frame node from a queue and sets the queueState of the frame accordingly.
         * This method is thread-safe.
         * @return the frame node
         */
        BufferFrameNode evictFrame();

        /**
         * Inserts a new frame in the fifo queue.
         * This method is thread-safe.
         * @param frame the new frame
         */
        void newFrame(BufferFrame* frame);

        /**
         * Reprioritizes a frame.
         * The frame is moved to the back of the lru queue.
         * This method is thread-safe.
         * @param frame the frame to reprioritize
         */
        void reprioritizeFrame(BufferFrame* frame);

    private:
        std::list<BufferFrameNode> _fifoQueue; // the fifo queue
        std::list<BufferFrameNode> _lruQueue; // the lru queue

        boost::mutex _mutex; // mutex for concurrent access

        /**
         * Checks if a frame is in the specified queue
         * @param frame the frame to search for
         * @param queue the check to use
         * @return true, if the frame can be found in the queue; false, else
         */
        bool isFrameInQueue(BufferFrame* frame, std::list<BufferFrameNode>& queue);
    };
}

#endif	/* SIMPLEDB_BUFFER_BUFFERREPLACEMENTMANAGER_HPP */

