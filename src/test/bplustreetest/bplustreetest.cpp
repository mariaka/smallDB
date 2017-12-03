
#include "bplustree.hpp"
#include "buffer.hpp"
#include "file.hpp"
#include "segment.hpp"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace simpledb;

/* Comparator functor for uint64_t*/
struct MyCustomUInt64Cmp {

    bool operator()(uint64_t a, uint64_t b) const {
        return a<b;
    }
};

template <unsigned len>
struct Char {
    char data[len];
};

template<unsigned len>
std::ostream& operator<<(std::ostream& out, const Char<len>& chr) {
    out << std::string(chr.data, len);
    return out;
}

/* Comparator functor for char */
template <unsigned len>
struct MyCustomCharCmp {

    bool operator()(const Char<len>& a, const Char<len>& b) const {
        return memcmp(a.data, b.data, len) < 0;
    }
};

typedef std::pair<uint32_t, uint32_t> IntPair;

std::ostream& operator<<(std::ostream& out, const IntPair& intPair) {
    out << intPair.first << ":" << intPair.second;
    return out;
}

/* Comparator for IntPair */
struct MyCustomIntPairCmp {

    bool operator()(const IntPair& a, const IntPair& b) const {
        if (a.first < b.first)
            return true;
        else
            return (a.first == b.first) && (a.second < b.second);
    }
};

template <class T>
const T& getKey(const uint64_t& i);

template <>
const uint64_t& getKey(const uint64_t& i) {
    return i;
}

std::vector<std::string> char20;

template <>
const Char<20>& getKey(const uint64_t& i) {
    std::stringstream ss;
    ss << i;
    std::string s(ss.str());
    char20.push_back(std::string(20 - s.size(), '0') + s);
    return *reinterpret_cast<const Char<20>*> (char20.back().data());
}

std::vector<IntPair> intPairs;

template <>
const IntPair& getKey(const uint64_t& i) {
    intPairs.push_back(std::make_pair(i / 3, 3 - (i % 3)));
    return intPairs.back();
}

template <class T>
const T getSmallestKey(const uint64_t& i) {
    return getKey<T>(0);
}

template <>
const IntPair getSmallestKey(const uint64_t& i) {
    return std::make_pair(0, 0);
}

template <class T, class CMP>
void test(uint64_t n) {
    // create tmp dir
    const char *tmpDir = "/tmp/slottedtest/";
    {
        if (mkdir(tmpDir, 0700) < 0) {
            perror("Could not create tmp directory");
        }
    }

    // Set up stuff, you probably have to change something here to match to your interfaces
    std::shared_ptr<FileManager> fm = std::make_shared<FileManager>(tmpDir);
    std::shared_ptr<BufferManager> bm = std::make_shared<BufferManager>(tmpDir, 1000);
    std::shared_ptr<SegmentManager> sm = std::make_shared<SegmentManager>(tmpDir, bm, fm);
    uint64_t segmentId = sm->create();
    BPlusTree<T, uint64_t, CMP> bTree(segmentId, sm, bm);
    std::map<T, uint64_t, CMP> map;

    // Insert values
    for (uint64_t i = 1; i <= n; ++i) {
        T key = getKey<T>(i);
        bTree.insert(key, i * i);
        map.insert(std::pair<T, uint64_t>(key, i * i));
        if (i == 10) {
            //bTree.visualize(std::cout);
        }
    }
    assert(bTree.size() == n);

    // Check if they can be retrieved
    for (uint64_t i = 1; i <= n; ++i) {
        uint64_t value = bTree.lookup(getKey<T>(i));
        assert(value == i * i);
    }

    // Check range request
    {
        typename BPlusTree<T, uint64_t, CMP>::iterator it = bTree.lookupRange(getSmallestKey<T>(0));
        typename std::map<T, uint64_t>::iterator map_it;
        for (map_it = map.begin(); it.isValid(); ++it, ++map_it) {
            T key = (*it).first;
            T keyMap = (*map_it).first;
            assert(!CMP()(key, keyMap) && !CMP()(keyMap, key));

            uint64_t value = (*it).second;
            uint64_t valueMap = (*map_it).second;
            assert(value == valueMap);
        }
    }

    // Delete some values
    for (uint64_t i = 1; i <= n; ++i) {
        if ((i % 7) == 0) {
            assert(bTree.erase(getKey<T>(i)));
            map.erase(getKey<T>(i));
        }
    }

    // Check if the right ones have been deleted
    for (uint64_t i = 1; i <= n; ++i) {
        uint64_t value = bTree.lookup(getKey<T>(i));
        if ((i % 7) == 0) {
            assert(value == 0);
        } else {
            assert(value == i * i);
        }
    }

    // Check range request
    {
        typename BPlusTree<T, uint64_t, CMP>::iterator it = bTree.lookupRange(getSmallestKey<T>(0));
        typename std::map<T, uint64_t>::iterator map_it;
        for (map_it = map.begin(); it.isValid(); ++it, ++map_it) {
            T key = (*it).first;
            T keyMap = (*map_it).first;
            assert(!CMP()(key, keyMap) && !CMP()(keyMap, key));

            uint64_t value = (*it).second;
            uint64_t valueMap = (*map_it).second;
            assert(value == valueMap);
        }
    }

    // Delete everything
    for (uint64_t i = 1; i <= n; ++i) {
        if ((i % 7) == 0) {
            assert(!bTree.erase(getKey<T>(i)));
        } else {
            assert(bTree.erase(getKey<T>(i)));
        }
    }
    assert(bTree.size() == 0);

    // Check range request
    {
        typename BPlusTree<T, uint64_t, CMP>::iterator it = bTree.lookupRange(getKey<T>(0));
        assert(!it.isValid());
    }

    sm->remove(segmentId);

    // delete tmp files and dir
    if (remove((std::string(tmpDir) + "segments").c_str()) < 0) {
        perror("Could not delete temp folder");
    }
    if (remove(tmpDir) < 0) {

        perror("Could not delete temp folder");
    }
}

int main(int argc, char* argv[]) {
    // Get command line argument
    const uint64_t n = (argc == 2) ? strtoul(argv[1], NULL, 10) : 1 * 1000ul;

    // Test index with 64bit unsigned integers
    test<uint64_t, MyCustomUInt64Cmp>(n);

    // Test index with 20 character strings
    test<Char<20>, MyCustomCharCmp < 20 >> (n);

    // Test index with compound key
    test<IntPair, MyCustomIntPairCmp>(n);


    std::cout << "test successful" << std::endl;

    return (EXIT_SUCCESS);
}
