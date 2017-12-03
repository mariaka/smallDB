//
//  main.cpp
//  Database
//
//  Created by Maria Meier on 11.04.14.
//  Copyright (c) 2014 Maria Meier. All rights reserved.
//

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <queue>
#include <chrono>

struct ElemInRun {
    uint64_t run;
    uint64_t value;
};

class CompareElemInRun {
public:

    bool operator()(const ElemInRun& e1, const ElemInRun& e2) {
        return (e1.value > e2.value);
    }
};

void print(uint64_t *begin, std::string text = "") {
    return;
    for (uint64_t *i = begin; i < begin + 10; ++i) {

        std::cout << *i;
        std::cout << " ";
    }
    std::cout << " (" << text << ")" << std::endl;
}

void makeTmpFileMapping(size_t fileSize, int &fd, char *&fname, uint64_t *&map) {
    // create temp file
    if ((fd = mkstemp(fname)) < 0) {
        std::cerr << "cannot open file '" << fname << "': " << strerror(errno) << std::endl;
        exit(-1);
    }

    // enlarge temp file to supplied file size
    if (ftruncate(fd, fileSize) < 0) {
        std::cerr << "Cannot enlarge file '" << fname << "' to " << fileSize << " bytes: " << strerror(errno) << std::endl;
        exit(-1);
    }

    // map temp file to memory
    map = static_cast<uint64_t *> (mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
}

/**
 * Sorts 64 bit unsigned integer values stored in a file in memory.
 *
 * @param fdInput file descriptor of the input file
 * @param size number of values
 * @param fdOutput file descriptor of the output file
 */
void internalSort(int fdInput, uint64_t size, int fdOutput) {
    const size_t file_size = size * sizeof (uint64_t); // file size in bytes

    // map input file to memory
    uint64_t * const in = static_cast<uint64_t *> (mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fdInput, 0)); // input file memory mapping
    if (in < 0) {
        perror("Cannot map input file to memory");
        return;
    }
    if (madvise(in, file_size, MADV_SEQUENTIAL)) { // improve performance by prefetching pages in sequential order
        perror("madvise failed");
        return;
    }

    // enlarge output file to file size
    if (ftruncate(fdOutput, file_size) < 0) {
        std::cerr << "Cannot enlarge output file to " << file_size << " bytes: " << strerror(errno) << std::endl;
        return;
    }

    // map output file to memory
    uint64_t * const out = static_cast<uint64_t *> (mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, fdOutput, 0)); // output file memory mapping
    if (out < 0) {
        perror("Cannot map output file to memory");
        return;
    }

    // copy input to output file
    memcpy(out, in, file_size);

    munmap(in, file_size); // we don't need the input file anymore

    // sort in memory
    std::sort(out, out + size);

    print(out, "out-internal");

    // unmap output file
    munmap(out, file_size);
}

/**
 * Sorts 64 bit unsigned integer values stored in a file using external merge sort.
 *
 * @param fdInput file descriptor of the input file
 * @param size number of values
 * @param fdOutput file descriptor of the output file
 * @param memSize used bytes of main memory
 */
void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {

    uint64_t *in;
    size_t file_size = size * sizeof (uint64_t); // file size in bytes

    // map input file to memory
    in = static_cast<uint64_t *> (mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fdInput, 0));
    madvise(in, file_size, MADV_SEQUENTIAL); // improve performance by prefetching pages in sequential order

    char tmpv[] = "/tmp/extsort_XXXXXX";
    char* tmpvp = tmpv;
    int fd_tmp;
    uint64_t *tmp;
    makeTmpFileMapping(file_size, fd_tmp, tmpvp, tmp);

    // Run Formation Phase
    std::cout << "Run Formation Phase" << std::endl;
    // TODO: make run_size a multiple of the page size // sysconf(_SC_PAGE_SIZE)
    uint64_t run_size = memSize / sizeof (uint64_t); // number of values in a run

    uint64_t *in_it, *tmp_it; // pointers to the start of the current run
    uint64_t run_length; // length of a run in number of values
    print(in, "in");

    // iterate over all runs
    int i;
    for (i = 1, in_it = in, tmp_it = tmp; in_it < in + size; in_it += run_length, tmp_it += run_length, ++i) {
        run_length = std::min(in_it + run_size, in + size) - in_it; // cut off last run at in + size
        memcpy(tmp_it, in_it, run_length * sizeof (uint64_t));

        print(tmp, "tmp");
        std::sort(tmp_it, tmp_it + run_length);
        //msync(tmp_it, run_length * sizeof(uint64_t), MS_ASYNC);
        print(tmp, "tmp");
        std::cout << "Run: " << std::setw(3) << i << "/" << (size + run_size - 1) / run_size << '\r';
        std::cout.flush();
    }
    std::cout << std::endl;

    char tmpv2[] = "/tmp/extsort_XXXXXX";
    char* tmpvp2 = tmpv2;
    int fd_tmp2;
    uint64_t *tmp2;

    makeTmpFileMapping(file_size, fd_tmp2, tmpvp2, tmp2);

    uint64_t * tmp_array[2] = {tmp, tmp2};

    // Merge Phases
    const int k = 32; // TODO: profile, find optimal parameter; is dynamic better?
    std::priority_queue<ElemInRun, std::vector<ElemInRun>, CompareElemInRun> pq{CompareElemInRun
        {}};
    std::cout << "Merge Phases (k = " << k << ")" << std::endl;

    print(tmp, "tmp1");
    print(tmp2, "tmp2");

    int phase, run_mult;
    for (phase = 0, run_mult = 1; run_size * run_mult < size; ++phase, run_mult *= k) {
        std::cout << "Phase " << phase + 1 << std::endl;

        uint64_t * const mergeIn = tmp_array[phase % 2];
        uint64_t *mergeOut = tmp_array[(phase + 1) % 2];

        int j = 1;
        for (uint64_t * mergeStart = mergeIn; mergeStart < mergeIn + size; mergeStart += k * run_size * run_mult, ++j) {
            std::cout << "Merge: " << std::setw(3) << j << "/" << (size + k * run_size * run_mult - 1) / (k * run_size * run_mult) << '\r';
            std::cout.flush();

            uint64_t * nextRunValue[k]; // pointers to the next element for merging in each runs
            // calculate beginning of each run for merging
            for (uint64_t i = 0; i < k; ++i) {
                nextRunValue[i] = mergeStart + i * run_size * run_mult;
                if (nextRunValue[i] >= mergeIn + size) {
                    nextRunValue[i] = nullptr;
                }
            }

            // init priority queue with first element from all runs
            // TODO: use list init for performance improvement
            for (uint64_t i = 0; i < k; ++i) {
                if (nextRunValue[i]) {
                    pq.push({i, *nextRunValue[i]});
                    ++nextRunValue[i];
                }
            }

            // merge k runs
            // TODO: profile, is it better to bulk load data into queue?
            while (!pq.empty()) {
                // get next value
                ElemInRun elem = pq.top();
                pq.pop();

                // write value
                *mergeOut = elem.value;
                ++mergeOut;

                // get new value from corresponding run, if
                // - nextRunValue hasn't reached the next run, and
                // - nextRunValue hasn't reached the end of the input file
                if (nextRunValue[elem.run] < mergeStart + (elem.run + 1) * run_size * run_mult && nextRunValue[elem.run] < mergeIn + size) {
                    pq.push({elem.run, *nextRunValue[elem.run]});
                    ++nextRunValue[elem.run];
                }

                print(tmp, "tmp1");
                print(tmp2, "tmp2");
            }
        }
        std::cout << std::endl;
    }

    // copy to output file
    // TODO: performance improvement: if only one run, copy directly to output file, i.e. don't use a temp file
    if (ftruncate(fdOutput, file_size) < 0) {
        std::cerr << "Cannot enlarge output file to " << file_size << " bytes: " << strerror(errno) << std::endl;
        exit(-1);
    }
    uint64_t *out = static_cast<uint64_t *> (mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, fdOutput, 0));
    memcpy(out, tmp_array[phase % 2], file_size);
    print(out, "out");

    // cleanup
    munmap(in, file_size);
    munmap(out, file_size);
    munmap(tmp, file_size);
    munmap(tmp2, file_size);
    close(fd_tmp);
    close(fd_tmp2);
    remove(tmpv);
    remove(tmpv2);
}

/**
 * Compares two files. The files must have the same length.
 *
 * @param fdFile1 file descriptor for the first file
 * @param fdFile2 file descriptor for the second file
 * @param size number of 64 bit unsigned values
 * @return true, if the files have the same content; false, otherwise
 */
bool compareFiles(int fdFile1, int fdFile2, uint64_t size) {
    size_t file_size = size * sizeof (uint64_t); // file size in bytes

    // map files into memory
    int fdFiles[2] = {fdFile1, fdFile2};
    uint64_t * files[2];

    for (int i = 0; i < 2; ++i) {
        // TODO: error handling
        files[i] = static_cast<uint64_t *> (mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fdFiles[i], 0));
        madvise(files[i], file_size, MADV_SEQUENTIAL); // improve performance by prefetching pages in sequential order
    }

    bool equal = (memcmp(files[0], files[1], file_size) == 0);

    for (int i = 0; i < 2; ++i) {
        // TODO: error handling
        munmap(files[i], file_size);
    }

    return equal;
}

int main(int argc, const char * argv[]) {
    using namespace std::chrono;

    bool showUsage = false;
    if (argc != 4 && argc != 5) {
        showUsage = true;
    }

    const char * const inputFile = argv[1]; // input filename
    const char * const outputFile = argv[2]; // output filename
    const uint64_t memSize = atoll(argv[3]) * 1000 * 1000; // memory buffer size in bytes
    bool runInMemory = false; // run additional in-memory sorting
    const char *inMemoryOutputFile = nullptr;
    if (argc >= 5) {
        runInMemory = true;
        inMemoryOutputFile = argv[4];
    }
    if (memSize <= 0) { // minimum memory size: 1 Mb
        showUsage = true;
    }

    if (showUsage) {
        std::cerr << "Usage: " << argv[0] << " <inputFile> <outputFile> <memoryBufferInMB> [<inMemorySortOutputFile>]" << std::endl;
        std::cerr << "Sorting verification is done by comparing to in-memory sorting (only if inMemorySortOutputFile is specified)" << std::endl;
        return -1;
    }

    struct stat statInput; // input file attributes
    if (stat(inputFile, &statInput) < 0) {
        std::cerr << "Cannot read input file '" << inputFile << "' attributes: " << strerror(errno) << std::endl;
        return -1;
    }

    const off_t fileSize = statInput.st_size; // input file size in bytes
    const uint64_t size = fileSize / sizeof (uint64_t); // number of input values
    // TODO: check file size is multiple of sizeof(uint64_t)

    const int fdInput = open(inputFile, O_RDONLY); // input file descriptor
    if (fdInput < 0) {
        std::cerr << "Cannot open input file '" << inputFile << "': " << strerror(errno) << std::endl;
        return -1;
    }

    // do in-memory sorting
    int fdOutputInMemory = 0;
    if (runInMemory) {
        fdOutputInMemory = open(inMemoryOutputFile, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR); // output file descriptor
        if (fdOutputInMemory < 0) {
            std::cerr << "Cannot open output file '" << inMemoryOutputFile << "': " << strerror(errno) << std::endl;
            return -1;
        }

        const high_resolution_clock::time_point sortIntStart = high_resolution_clock::now();
        const clock_t sortIntStartClock = clock();

        internalSort(fdInput, size, fdOutputInMemory);

        const clock_t sortIntEndClock = clock();
        const high_resolution_clock::time_point sortIntEnd = high_resolution_clock::now();

        const microseconds sortIntTime = duration_cast<microseconds> (sortIntEnd - sortIntStart);
        const uint64_t sortIntTimeClock = static_cast<uint64_t> (sortIntEndClock - sortIntStartClock) * 1000 * 1000 / CLOCKS_PER_SEC;

        std::cout << "In-Memory Sort Time:      " << sortIntTime.count() << " microseconds" << std::endl;
        std::cout << "In-Memory Sort Time:      " << sortIntTimeClock << " microseconds (CPU clocks)" << std::endl;
    }

    // do external sorting
    const int fdOutput = open(outputFile, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR); // output file descriptor
    if (fdOutput < 0) {
        std::cerr << "Cannot open output file '" << outputFile << "': " << strerror(errno) << std::endl;
        return -1;
    }

    const high_resolution_clock::time_point sortExtStart = high_resolution_clock::now();
    const clock_t sortExtStartClock = clock();

    externalSort(fdInput, size, fdOutput, memSize);

    const clock_t sortExtEndClock = clock();
    const high_resolution_clock::time_point sortExtEnd = high_resolution_clock::now();

    const microseconds sortExtTime = duration_cast<microseconds> (sortExtEnd - sortExtStart);
    const uint64_t sortExtTimeClock = static_cast<uint64_t> (sortExtEndClock - sortExtStartClock) * 1000 * 1000 / CLOCKS_PER_SEC;

    std::cout << "External Merge Sort Time: " << sortExtTime.count() << " microseconds" << std::endl;
    std::cout << "External Merge Sort Time: " << sortExtTimeClock << " microseconds (CPU clocks)" << std::endl;

    if (runInMemory) {
        // verify sorting by comparing external and in-memory sorting
        if (compareFiles(fdOutput, fdOutputInMemory, size)) {
            std::cout << "Sorting correct!" << std::endl;
        } else {
            std::cout << "Sorting incorrect!" << std::endl;
        }

        close(fdOutputInMemory);
    }

    close(fdInput);
    close(fdOutput);

    return 0;
}
