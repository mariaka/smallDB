
#ifndef SIMPLEDB_BUFFER_BUFFERFRAME_HPP
#define	SIMPLEDB_BUFFER_BUFFERFRAME_HPP

#include "BufferFrameNode.hpp"
#include "PageId.hpp"

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/tss.hpp>

#include <list>

namespace simpledb {

    /**
     * Buffer frame holds meta information for a frame in memory and handles its locks.
     * @param pageId page and segment indentifying the frame on disk
     */
    class BufferFrame {
    public:

        BufferFrame(PageId pageId) : _pageId(pageId), _state(FrameState::clean), _queueState(QueueState::none), _data(nullptr), _queueNode(), _mutex(), _uniqueLock(), _sharedLock() {
        };

        ~BufferFrame() {
        };

        BufferFrame(const BufferFrame& orig) = delete;
        BufferFrame& operator=(const BufferFrame& orig) = delete;

        PageId pageId() const {
            return _pageId;
        };

        bool isClean() const {
            return (_state == FrameState::clean);
        }

        void setClean() {
            assert(!isDeleted());
            _state = FrameState::clean;
        }

        bool isDirty() const {
            return (_state == FrameState::dirty);
        }

        void setDirty() {
            assert(!isDeleted());
            _state = FrameState::dirty;
        }

        bool isDeleted() const {
            return (_state == FrameState::deleted);
        }

        void setDeleted() {
            assert(isClean());
            _state = FrameState::deleted;
        }

        bool isInFifoQueue() const {
            return (_queueState == QueueState::fifo);
        }

        bool isInLruQueue() const {
            return (_queueState == QueueState::lru);
        }

        void setFifoQueue() {
            _queueState = QueueState::fifo;
        }

        void setLruQueue() {
            _queueState = QueueState::lru;
        }

        void setNoQueue() {
            _queueState = QueueState::none;
        }

        /**
         * A buffer frame should offer a method giving access to the buffered page.
         * Except for the buffered page, BufferFrame objects can also store
         * control information (page ID, dirtyness, ...).
         * @return void pointer to the data of the frame
         */
        void* getData() const {
            return _data;
        }

        void setData(void* data) {
            _data = data;
        }

        std::list<BufferFrameNode>::iterator& queueNode() {
            return _queueNode;
        }

        void setQueueNode(std::list<BufferFrameNode>::iterator queueNode) {
            _queueNode = queueNode;
        }

        /**
         * Blocks until a lock can be obtained.
         * @param exclusive true, for exclusive (write) lock; false, for shared (read) lock
         */
        void lock(bool exclusive);

        /**
         * Returns all previously obtained locks.
         */
        void unlock();

    private:

        /**
         * Indicates whether the frame is in a consistent state with the disk (clean)
         * or must be written back to disk before flushing (dirty) or is already
         * evicted (deleted).
         */
        enum class FrameState {
            clean, dirty, deleted
        };

        /**
         * Indicates in which queue the replacement manager holds the frame node:
         * - none: in no queue
         * - fifo: in fifo queue
         * - lru: in lru queue
         */
        enum class QueueState {
            none, fifo, lru
        };

        PageId _pageId; // page id identifying the segment and page on disk
        // uint64_t _LSN;
        FrameState _state;
        QueueState _queueState; // indicates in which queue the replacement manager holds the frame node
        void *_data; // pointer to allocated memory
        std::list<BufferFrameNode>::iterator _queueNode; // reference to frame node in the queues of the replacement manager
        boost::shared_mutex _mutex; // shared mutex for concurrent access
        boost::thread_specific_ptr<boost::unique_lock<boost::shared_mutex>> _uniqueLock; // exclusive (write) lock
        boost::thread_specific_ptr<boost::shared_lock<boost::shared_mutex>> _sharedLock; // shared (read) lock
    };
}

#endif	/* SIMPLEDB_BUFFER_BUFFERFRAME_HPP */
