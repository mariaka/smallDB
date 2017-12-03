
#ifndef SIMPLEDB_BUFFER_BUFFERMANAGER_HPP
#define	SIMPLEDB_BUFFER_BUFFERMANAGER_HPP

#include "file.hpp"
#include "BufferFrame.hpp"
#include "BufferFrameTable.hpp"
#include "BufferFrameTableBucket.hpp"
#include "BufferReplacementManager.hpp"

#include <unistd.h>

#include <cstdint>
#include <string>
#include <memory>

namespace simpledb {

    /**
     * Buffer Manager that manages buffer frames and controls concurrent access to these frames.
     */
    class BufferManager {
    public:
        static constexpr uint64_t PAGE_SIZE = 4096 * 1; // the size of a frame in bytes

        /**
         * Creates a new instance that manages size frames and operates files in the folder path.
         * @param path the path of the files the buffer manager operates on
         * @param size the number of frames the buffer manager holds in memory
         */
        BufferManager(std::string path, uint64_t size) : _size(size), _buffer(initBuffer()), _fileManager(path), _table(size), _replacementManager(_size, PAGE_SIZE, _buffer) {
        }

        /**
         * Writes all dirty pages back to disk and frees the allocated memory for the buffer frames.
         */
        ~BufferManager();

        BufferManager(const BufferManager& orig) = delete;
        BufferManager& operator=(const BufferManager& orig) = delete;

        /**
         * Retrieves a frame given a segment ID and a page ID and indicating whether the page will be held exclusively by this thread or not.
         * Pages are stored on disk in files with the same name as the segment ID (e.g. 1).
         * This method is thread-safe.
         * @param segmentId the segment ID
         * @param pageId the page ID
         * @param exclusive true, for exclusive (write) access; false, for shared (read) access
         * @return pointer to the buffer frame
         */
        std::shared_ptr<BufferFrame> fixPage(uint64_t segmentId, uint64_t pageId, bool exclusive);

        /**
         * Return a frame to the buffer manager indicating whether it is dirty or not.
         * If dirty, the page manager writes it back to disk. It does not
         * have to write it back immediately, but must not write it back before
         * unfixPage is called.
         * This method is thread-safe.
         * @param frame the frame to unfix
         * @param isDirty true, if the frame is dirty (has been changed); false, otherwise
         */
        void unfixPage(std::shared_ptr<BufferFrame> frame, bool isDirty);

        /**
         * @deprecated use PAGE_SIZE instead
         */
        uint64_t pageSize() {
            return PAGE_SIZE;
        }

    private:
        uint64_t _size; // the number of frames the buffer manager holds in memory
        void *_buffer; // the allocated memory region
        FileManager _fileManager; // manages reading and writing to disk
        BufferFrameTable _table; // hash table for all frames in memory
        BufferReplacementManager _replacementManager; // implements page replacement strategy

        /**
         * Allocates memory to hold the buffer frames.
         * Memory is aligned to the page size
         * @return pointer to the allocated memory region
         */
        void* initBuffer();

        /**
         * Evicts a buffer frame.
         * Replacement is done by the replacement manager. Dirty pages are written to disk.
         * This method is thread-safe.
         * @return pointer to the freed memory region
         */
        void* evictPage();

        /**
         * Checks if a frame is dirty and writes it to disk.
         * The frame must be locked exclusively by the caller first.
         * @param frame the frame to clean
         */
        void cleanFrame(BufferFrame* frame);
    };
}

#endif	/* SIMPLEDB_BUFFER_BUFFERMANAGER_HPP */
