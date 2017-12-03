
#include "BufferReplacementManager.hpp"

namespace simpledb {

    BufferReplacementManager::BufferReplacementManager(uint64_t size, long pageSize, void *buffer) : _fifoQueue(), _lruQueue(), _mutex() {
        uint64_t i;
        char *node;
        for (i = 0, node = reinterpret_cast<char *> (buffer); i < size; ++i, node += pageSize) {
            _fifoQueue.push_back(BufferFrameNode(false, PageId(0, 0), node, nullptr));
        }
    };

    BufferFrameNode BufferReplacementManager::evictFrame() {
        boost::lock_guard<boost::mutex> lock(_mutex);

        std::list<BufferFrameNode> &queue = (_fifoQueue.empty() ? _lruQueue : _fifoQueue);
        BufferFrameNode node = *(queue.cbegin());
        assert(!queue.empty()); // this should not happen, if number of threads < number of frame slots in memory
        // TODO: block if queue is empty

        if (node.inUse) {
            node.frame->setNoQueue();
        }
        queue.pop_front();
        // Remark: don't use queueNode() in the BufferFrame after this, it's invalid
        return node;
    }

    void BufferReplacementManager::newFrame(BufferFrame* frame) {
        boost::lock_guard<boost::mutex> lock(_mutex);

        _fifoQueue.push_back(BufferFrameNode(true, frame->pageId(), frame->getData(), frame));
        frame->setFifoQueue();
        frame->setQueueNode(--_fifoQueue.end());
    }

    void BufferReplacementManager::reprioritizeFrame(BufferFrame* frame) {
        boost::lock_guard<boost::mutex> lock(_mutex);

        if (frame->isInFifoQueue()) {
            assert(isFrameInQueue(frame, _fifoQueue));
            _lruQueue.splice(_lruQueue.end(), _fifoQueue, frame->queueNode());
            frame->setLruQueue();
        } else if (frame->isInLruQueue()) {
            assert(isFrameInQueue(frame, _lruQueue));
            _lruQueue.splice(_lruQueue.end(), _lruQueue, frame->queueNode());
        } // else: not in any queue
    }

    bool BufferReplacementManager::isFrameInQueue(BufferFrame* frame, std::list<BufferFrameNode>& queue) {
        for (const BufferFrameNode &i : queue) {
            if (frame->pageId() == i.page) {
                return true;
            }
        }
        return false;
    };
}
