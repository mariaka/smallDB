#include <buffer.hpp>

#include <boost/thread/thread.hpp>

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <assert.h>
#include <memory>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;
using namespace simpledb;

BufferManager* bm;
unsigned pagesOnDisk;
unsigned pagesInRAM;
unsigned threadCount;
unsigned* threadSeed;
uint64_t* threadCounter;
volatile bool stop = false;

unsigned randomPage(unsigned threadNum) {
    // pseudo-gaussian, causes skewed access pattern
    unsigned page = 0;
    for (unsigned i = 0; i < 20; i++)
        page += rand_r(&threadSeed[threadNum]) % pagesOnDisk;
    return page / 20;
}

static void scan() {
    // scan all pages and check if the counters are not decreasing
    unsigned counters[pagesOnDisk];
    for (unsigned i = 0; i < pagesOnDisk; i++)
        counters[i] = 0;

    while (!stop) {
        unsigned start = random() % pagesOnDisk;
        unsigned i, page;
        for (page = start, i = 0; i < 10; page = (page + 1) % pagesOnDisk, ++i) {
            std::shared_ptr<BufferFrame> bf = bm->fixPage(1, page, false);
            unsigned newcount = reinterpret_cast<unsigned*> (bf->getData())[0];
            assert(counters[page] <= newcount);
            counters[page] = newcount;
            bm->unfixPage(bf, false);
        }
    }
}

static void readWrite(uint64_t threadNum) {
    // read or write random pages
    uint64_t count = 0;
    for (unsigned i = 0; i < 100000 / threadCount; i++) {
        bool isWrite = rand_r(&threadSeed[threadNum]) % 128 < 10;
        std::shared_ptr<BufferFrame> bf = bm->fixPage(1, randomPage(threadNum), isWrite);

        if (isWrite) {
            count++;
            reinterpret_cast<unsigned*> (bf->getData())[0]++;
        }
        bm->unfixPage(bf, isWrite);
    }

    threadCounter[threadNum] = count;
}

int main(int argc, char** argv) {
    if (argc == 4) {
        pagesOnDisk = atoi(argv[1]);
        pagesInRAM = atoi(argv[2]);
        threadCount = atoi(argv[3]);
    } else {
        cerr << "usage: " << argv[0] << " <pagesOnDisk> <pagesInRAM> <threads>" << endl;
        exit(1);
    }

    threadSeed = new unsigned[threadCount];
    threadCounter = new uint64_t[threadCount];
    for (uint64_t i = 0; i < threadCount; i++)
        threadSeed[i] = i * 97134;

    // allocate tmp file
    const char *tmpDir = "/tmp/buffertest/";
    const char *tmpFile = "/tmp/buffertest/1";
    {
        if (mkdir(tmpDir, 0700) < 0) {
            perror("Could not create tmp directory");
        }
    }
    {
        int file = ::open(tmpFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (file < 0) {
            perror("Could not create tmp file");
            exit(EXIT_FAILURE);
        } else {
            ::close(file);
        }
    }
    if (truncate(tmpFile, pagesOnDisk * sysconf(_SC_PAGE_SIZE)) < 0) {
        perror("Could not allocate tmp file");
        exit(EXIT_FAILURE);
    }

    bm = new BufferManager(tmpDir, pagesInRAM);

    boost::thread_group threads;

    // set all counters to 0
    for (uint64_t i = 0; i < pagesOnDisk; i++) {
        std::shared_ptr<BufferFrame> bf = bm->fixPage(1, i, true);
        reinterpret_cast<unsigned*> (bf->getData())[0] = 0;
        bm->unfixPage(bf, true);
    }

    // start scan thread
    boost::thread scanThread(scan);

    // start read/write threads
    for (uint64_t i = 0; i < threadCount; i++) {
        threads.add_thread(new boost::thread(readWrite, i));
    }

    // wait for read/write threads
    threads.join_all();

    uint64_t totalCount = 0;
    for (uint64_t i = 0; i < threadCount; i++) {
        totalCount += threadCounter[i];
    }

    // wait for scan thread
    stop = true;
    scanThread.join();

    // restart buffer manager
    delete bm;
    bm = new BufferManager(tmpDir, pagesInRAM);

    // check counter
    uint64_t totalCountOnDisk = 0;
    for (uint64_t i = 0; i < pagesOnDisk; i++) {
        std::shared_ptr<BufferFrame> bf = bm->fixPage(1, i, false);
        totalCountOnDisk += reinterpret_cast<unsigned*> (bf->getData())[0];
        bm->unfixPage(bf, false);
    }

    // delete temp folder and file
    if (remove(tmpFile) < 0) {
        perror("Could not delete temp file");
    }
    if (remove(tmpDir) < 0) {
        perror("Could not delete temp folder");
    }

    if (totalCount == totalCountOnDisk) {
        cout << "test successful" << endl;
        delete bm;
        return 0;
    } else {
        cerr << "error: expected " << totalCount << " but got " << totalCountOnDisk << endl;
        delete bm;
        return 1;
    }
}
