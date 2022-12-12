#include <iostream>
#include <regex>
#include <vector>
#include <sstream>
#include <iomanip>
#include "taskflow/taskflow/taskflow.hpp"
#include "../archiving/libarchive-master/libarchive/archive.h"
#include "../archiving/libarchive-master/libarchive/archive_entry.h"
#include <cstring>
#include <string.h>
#include "sha2.c"
#include <cmath>
#include <fstream>
#include <filesystem>
#include <fcntl.h>

namespace fs = std::filesystem;

const std::regex hex_pattern("[a-f0-9]*");

void write_archive(const char *outname, std::vector<std::string> filenames)
{
    struct archive *a;
    struct archive_entry *entry;
    struct stat st;
    char buff[8192];
    int len;
    int fd;

    a = archive_write_new();
    archive_write_add_filter_gzip(a);
    archive_write_set_format_pax_restricted(a); // Note 1
    archive_write_open_filename(a, outname);
    for(std::string filename : filenames)
    {
        stat(filename.c_str(), &st);
        entry = archive_entry_new(); // Note 2
        archive_entry_set_pathname(entry, filename.c_str());
        archive_entry_set_size(entry, st.st_size); // Note 3
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_write_header(a, entry);
        fd = open(filename.c_str(), O_RDONLY);
        len = read(fd, buff, sizeof(buff));
        while (len > 0)
        {
            archive_write_data(a, buff, len);
            len = read(fd, buff, sizeof(buff));
            
        }
        close(fd);
        archive_entry_free(entry);
    }
    archive_write_close(a); // Note 4
    archive_write_free(a);  // Note 5
}

std::vector<uint8_t> hexToBytes(std::string hex)
{
    if (!std::regex_match(hex, hex_pattern))
    {
        // not hex
        return {};
    }
    else if (hex.length() % 2 != 0)
    {
        // odd length
        std::cout << "entered here" << std::endl;
        return {};
    }

    std::vector<uint8_t> binary;
    for (unsigned int i = 0; i < hex.length(); i += 2)
    {
        std::string b = hex.substr(i, 2);
        uint8_t byte = (uint8_t)std::stoi(b.c_str(), nullptr, 16);
        binary.push_back(byte);
    }
    return binary;
}

bool isHex(std::string hex)
{
    if (std::regex_match(hex, hex_pattern))
    {
        return true;
    }
    return false;
}

std::string bytesToHex(std::vector<uint8_t> bytes)
{
    std::stringstream ss;
    ss << std::hex;
    for (int i(0); i < bytes.size(); ++i)
        ss << std::setw(2) << std::setfill('0') << (int)bytes[i];
    return ss.str();
}

// create data directory and write to file
void writeDigestToFile(std::string filename, const unsigned char* message) {
    fs::create_directories("data");

    std::filesystem::path cwd = std::filesystem::current_path();
    std::string filePath = cwd.string() + "/data/";

    std::ofstream outfile (filePath + filename + ".txt");
    outfile << message << std::endl;
    outfile.close();
}

// create index directory and write to file
void createIndexdirectory(std::string digest, std::string filename) {
    fs::create_directories("index");
    
    std::filesystem::path cwd = std::filesystem::current_path();
    std::string filePath = cwd.string() + "/index/" + filename;

    std::ofstream outfile (filePath, std::ios_base::app);
    std::vector<uint8_t> digest_vec = hexToBytes(digest);
    std::string digesthashbytes(digest_vec.begin(), digest_vec.end());

    outfile << digesthashbytes << std::endl;
}

std::vector<uint8_t> loadFileBinary(std::string filepath) {
    std::ifstream infile(filepath, std::ios::in | std::ios::binary);
    std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    return fileData;
}

int main() {
    std::vector<uint8_t> prefix;
    std::string target = "21e8";
    std::vector<std::string> hashes;
    std::vector<uint8_t> outp, dig;
    std::string emptynonce = "0000000000000000";
    std::string data = "beans";
    // std::cout << "Provide a prefix" << std::endl;
    // std::getline(std::cin, target);
    std::string key_word = "6618b92af2414c01da2abc2488374ea10d6167120c5dcbea544c35ee51b36bdf";
    std::string user_hash = "297195ee715967e02f937b0c6375aebae111d6582d59beb8212d76a4ace3906d";
    std::string data_hash = "6eb228fb5b59ab49a45a48bdd9e5f0ed65b43afb52ec72cc825a567986630827"; 

    std::string hash = key_word + user_hash + data_hash;
    // std::cout << "Now a hash: " << std::endl;

    // while (std::getline(std::cin, hash) && !hash.empty())
    // {
    //     if (isHex(hash) && hash.length() == 64)
    //     {
    //         hashes.push_back(hash);
    //     }
    //     else
    //     {
    //         std::cout << "Invalid hash, must be (64char/32byte) hexadecimal" << std::endl;
    //     }

    //     std::cout << "More hashes? (hit 'return' to skip): ";
    // }

    bool oddPrefix = false;
    uint8_t oddPrefixEnd = 0;

    if (target.length() % 2 != 0)
    {
        oddPrefix = true;
        prefix = hexToBytes(target.substr(0, target.length() - 1));
        oddPrefixEnd = (target.back() >= 'a') ? (target.back() - 'a' + 10) : (target.back() - '0');
        oddPrefixEnd = oddPrefixEnd << 4;
    }
    else
        prefix = hexToBytes(target);

    std::string raw = target;

    // for (int i = 0; i < hashes.size(); i++) {
    //     raw += hashes[i];
    // }
    raw += hash;

    if (oddPrefix) {
        raw += "0";
    }

    uint64_t start_nonce = 0;
    int itemsPerTask = 1, size = 12;
    bool found = false;

    std::string digeststring;

    tf::Taskflow taskflow;
    tf::Executor executor;

    tf::Task entry = taskflow.emplace([]() {});
    tf::Task tfmine = taskflow.for_each_index(0, size, itemsPerTask, [&](int index)
                                              {
        std::string rawtomine = raw + emptynonce;
        std::vector<uint8_t> rawconst = hexToBytes(rawtomine);
        unsigned char message[rawconst.size()];

        for (int i = 0; i < rawconst.size(); i++) {
            message[i] = rawconst[i];
        }

        unsigned char digest[32];
        unsigned int len = rawconst.size();

        unsigned char *noncePtr = message + rawconst.size() - 8;
        uint64_t *n = (long long unsigned int *) noncePtr;
        (*n) = index * (ULLONG_MAX / 99.3) + start_nonce;

        while (!found) {
                (*n)++;
                sha256(message, len, digest);
                if (memcmp(digest, &prefix[0], prefix.size()) == 0) {
                    if (!oddPrefix || (digest[prefix.size()] ^ oddPrefixEnd) < 16) {
                        std::vector<uint8_t> digestvec;
                        std::vector<uint8_t> messagevec;
                        for(int i = 0; i < rawconst.size(); i++) {
                            messagevec.push_back(message[i]);
                        }
                        std::string messagestring = bytesToHex(messagevec);
                        for(int i =0; i < 32; i++) {
                            digestvec.push_back(digest[i]);
                        }
                        digeststring = bytesToHex(digestvec);
                        writeDigestToFile(digeststring, message);
                        found = true;
                    }
                }
        } });

    tf::Task exit = taskflow.emplace([]() {});
    entry.precede(tfmine);
    tfmine.precede(exit);
    executor.run(taskflow).wait();

    createIndexdirectory(digeststring, key_word);
    const char* datachar = data.c_str();
    writeDigestToFile(data_hash, reinterpret_cast<const unsigned char *>(datachar));

    std::filesystem::path cwd = std::filesystem::current_path();
    std::string directory = "6618b92af2414c01da2abc2488374ea10d6167120c5dcbea544c35ee51b36bdf";
    const char* index = directory.c_str();
    std::vector<std::string> x;
    x.push_back(directory);
    
    write_archive("tarFileTest.tar", x);

    return 0;
}