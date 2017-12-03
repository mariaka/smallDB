
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "buffer.hpp"
#include "data.hpp"
#include "file.hpp"
#include "segment.hpp"

using namespace simpledb;
using namespace std;

const unsigned initialSize = 100; // in (slotted) pages
const unsigned maxInserts = 1000ul * 1000ul;
const unsigned maxDeletes = 1000;
const unsigned maxUpdates = 1000;
const double loadFactor = .8; // percentage of a page that can be used to store the payload
const vector<string> testData = {
    "640K ought to be enough for anybody",
    "Beware of bugs in the above code; I have only proved it correct, not tried it",
    "Tape is Dead. Disk is Tape. Flash is Disk.",
    "for seminal contributions to database and transaction processing research and technical leadership in system implementation",
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce iaculis risus ut ipsum pellentesque vitae venenatis elit viverra. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Curabitur ante mi, auctor in aliquet non, sagittis ac est. Phasellus in viverra mauris. Quisque scelerisque nisl eget sapien venenatis nec consectetur odio aliquam. Maecenas lobortis mattis semper. Ut lacinia urna nec lorem lacinia consectetur. In non enim vitae dui rhoncus dictum. Sed vel fringilla felis. Curabitur tincidunt justo ac nulla scelerisque accumsan. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Cras tempor venenatis orci, quis vulputate risus dapibus id. Aliquam elementum congue nulla, eget tempus justo fringilla sed. Maecenas odio erat, commodo a blandit quis, tincidunt vel risus. Proin sed ornare tellus. Donec tincidunt urna ac turpis rutrum varius. Etiam vehicula semper velit ut mollis. Aliquam quis sem massa. Morbi ut massa quis purus ullamcorper aliquet. Sed nisi justo, fermentum id placerat eu, dignissim eu elit. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Suspendisse interdum laoreet commodo. Nullam turpis velit, tristique in sodales sit amet, molestie et diam. Quisque blandit velit quis augue sodales vestibulum. Phasellus ut magna non arcu egestas volutpat. Etiam id ultricies ligula. Donec non lectus eget risus lobortis pretium. Sed rutrum augue eu tellus scelerisque sit amet interdum massa volutpat. Maecenas nunc ligula, blandit quis adipiscing eget, fermentum nec massa. Vivamus in commodo nunc. Quisque elit mi, consequat eget vestibulum lacinia, ultrices eu purus. Vestibulum tincidunt consequat nulla, quis tempus eros volutpat sed. Aliquam elementum massa vel ligula bibendum aliquet non nec purus. Nunc sollicitudin orci sed nisi eleifend molestie. Praesent scelerisque vehicula quam et dignissim. Suspendisse potenti. Sed lacus est, aliquet auctor mollis ac, iaculis at metus. Aenean at risus sed lectus volutpat bibendum non id odio. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Mauris purus lorem, congue ac tristique sit amet, gravida eu neque. Nullam lacus tellus, venenatis a blandit ac, consequat sed massa. Mauris ultrices laoreet lorem. Nam elementum, est vel elementum commodo, enim tellus mattis diam, a bibendum mi enim vitae magna. Aliquam nisi dolor, aliquam at porta sit amet, tristique id nulla. In purus leo, tristique eget faucibus id, pharetra vel diam. Nunc eleifend commodo feugiat. Mauris sed diam quis est dictum rutrum in eu erat. Suspendisse potenti. Duis adipiscing nisl eu augue dignissim sagittis. Praesent vitae nisl dolor. Duis interdum, dolor a viverra imperdiet, lorem lectus luctus sem, sit amet rutrum augue dolor id erat. Vestibulum ac orci condimentum velit mollis scelerisque eu eu est. Aenean fringilla placerat enim, placerat adipiscing felis feugiat quis. Cras sed."
};

int main(int argc, char** argv) {
    // Check arguments
    if (argc != 1) {
        cerr << "usage: " << argv[0] << endl;
        return (EXIT_FAILURE);
    }

    // create tmp dir
    const char *tmpDir = "/tmp/slottedtest/";
    {
        if (mkdir(tmpDir, 0700) < 0) {
            perror("Could not create tmp directory");
        }
    }

    {
        // Bookkeeping
        unordered_map<TID, unsigned> values; // TID -> testData entry
        unordered_map<unsigned, unsigned> usage; // pageID -> bytes used within this page

        // Setup everything
        std::shared_ptr<FileManager> fm = std::make_shared<FileManager>(tmpDir);
        std::shared_ptr<BufferManager> bm = std::make_shared<BufferManager>(tmpDir, 100);
        std::shared_ptr<SegmentManager> sm = std::make_shared<SegmentManager>(tmpDir, bm, fm);
        uint64_t segmentId = sm->create();
        SPSegment sp(segmentId, sm, bm);
        const unsigned pageSize = bm->pageSize();

        std::default_random_engine randomGenerator(88172645463325252ull);
        std::uniform_int_distribution<uint64_t> distributionTestData(0, testData.size() - 1);
        std::bernoulli_distribution distribution10(0.1);
        auto randomData = std::bind(distributionTestData, randomGenerator);
        auto random10 = std::bind(distribution10, randomGenerator);

        // Insert some records
        for (unsigned i = 0; i < maxInserts; ++i) {
            // Select string/record to insert
            uint64_t r = randomData();
            const string s = testData[r];

            // Check that there is space available for 's'
            bool full = true;
            for (unsigned p = 0; p < initialSize; ++p) {
                if (usage[p] + s.size() < loadFactor * pageSize) {
                    full = false;
                    break;
                }
            }
            if (full)
                break;

            // Insert record
            TID tid = sp.insert(Record(s.size(), s.c_str()));
            assert(values.find(tid) == values.end()); // TIDs should not be overwritten
            values[tid] = r;
            unsigned pageId = tid.pageId().page; // extract the pageId from the TID
            assert(pageId < initialSize); // pageId should be within [0, initialSize)
            usage[pageId] += s.size();
        }

        // Lookup & delete some records
        auto it_del = values.begin();
        for (unsigned i = 0; i < maxDeletes; ++i) {
            // Select operation
            bool del = random10();

            // Select victim
            TID tid = it_del->first;
            unsigned pageId = tid.pageId().page;
            const std::string& value = testData[(it_del->second) % testData.size()];
            unsigned len = value.size();

            // Lookup
            Record rec = sp.lookup(tid);
            assert(rec.length() == len);
            assert(memcmp(rec.getData(), value.c_str(), len) == 0);

            ++it_del;
            if (it_del == values.end()) {
                it_del = values.begin();
            }

            if (del) { // do delete
                assert(sp.remove(tid));
                values.erase(tid);
                usage[pageId] -= len;
            }
        }

        // Update some values ('usage' counter invalid from here on)
        auto it_update = values.begin();
        for (unsigned i = 0; i < maxUpdates; ++i) {
            // Select victim
            TID tid = it_update->first;

            // Select new string/record
            uint64_t r = randomData();
            const string s = testData[r];

            // Replace old with new value
            assert(sp.update(tid, Record(s.size(), s.c_str())));
            values[tid] = r;
            //cout << "update (" << tid.pageId().page << "," << tid.slotId() << "): " << s.size() << endl;

            ++it_update;
            if (it_update == values.end()) {
                it_update = values.begin();
            }
        }

        // Lookups
        for (auto p : values) {
            TID tid = p.first;
            const std::string& value = testData[p.second];
            unsigned len = value.size();
            Record rec = sp.lookup(tid);
            assert(rec.length() == len);
            assert(memcmp(rec.getData(), value.c_str(), len) == 0);
            //cout << rec.length() << " == " << len << endl;
        }

        sm->remove(segmentId);
    }

    // delete tmp files and dir
    if (remove((std::string(tmpDir) + "segments").c_str()) < 0) {
        perror("Could not delete temp folder");
    }
    if (remove(tmpDir) < 0) {
        perror("Could not delete temp folder");
    }

    cout << "test successful" << endl;

    return (EXIT_SUCCESS);
}
