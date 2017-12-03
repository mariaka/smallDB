
#ifndef SIMPLEDB_FILE_FILEMANAGER_HPP
#define	SIMPLEDB_FILE_FILEMANAGER_HPP

#include "buffer/PageId.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_guard.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace simpledb {

    /**
     * File manager handles all file operations.
     * @param path the path where all files reside
     */
    class FileManager {
    public:

        explicit FileManager(std::string path) : _path(path) {
        };
        ~FileManager();

        FileManager(const FileManager& orig) = delete;
        FileManager& operator=(const FileManager& orig) = delete;

        /**
         * Reads a page from a file to memory.
         * Files are opened transparently.
         * @param pageId the page id identifies the file and page
         * @param PAGE_SIZE the size of a page in bytes
         * @param data the pointer to the memory region for writing
         */
        void read(PageId pageId, const uint64_t PAGE_SIZE, void *data);

        /**
         * Writes a page from memory to a file.
         * Files are opened transparently.
         * @param pageId the page id identifies the file and page
         * @param PAGE_SIZE the size of a page in bytes
         * @param data
         */
        void write(PageId pageId, const uint64_t PAGE_SIZE, void *data);

        /**
         * Creates a file for the given segment id.
         * This method is thread-safe.
         * @param segmentId the segment id
         */
        void create(uint64_t segmentId);

        /**
         * Deletes the file with the given segment id.
         * This method is thread-safe.
         * @param segmentId the segment id
         */
        void remove(uint64_t segmentId);

        /**
         * Truncates the segment file to the given length.
         * @param segmentId the segment id
         * @param PAGE_SIZE the page size
         * @param size the new file size in bytes
         */
        void truncate(uint64_t segmentId, const uint64_t PAGE_SIZE, uint64_t size);

    private:
        std::string _path; // the path where all files reside
        std::unordered_map<uint64_t, int> _fileHandles; // map: segment id -> file handle
        boost::mutex _mutex; // mutex for concurrent access

        /**
         * Opens a file.
         * This method is thread-safe.
         * @param segmentId the segment id
         */
        void open(uint64_t segmentId);

        /**
         * Closes a file.
         * This method is thread-safe.
         * @param segmentId the segment id
         */
        void close(uint64_t segmentId);

        /**
         * Closes a file.
         * @param segmentId the segment id
         */
        void closeUnsyncronized(uint64_t segmentId);

        /**
         * Checks if a file is already openend.
         * @param segmentId the segment id
         * @return true, if the file is already opened; false, otherwise
         */
        bool isOpen(uint64_t segmentId) {
            return (_fileHandles.count(segmentId) > 0);
        };

        /**
         * Calculates the file name of a segment.
         * @param segmentId the segment id
         * @return the file name of the segment
         */
        std::string getFileName(uint64_t segmentId) {
            return (_path + std::to_string(segmentId));
        };
    };
}

#endif	/* SIMPLEDB_FILE_FILEMANAGER_HPP */

