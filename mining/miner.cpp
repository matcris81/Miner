#include <iostream>
#include <regex>
#include <vector>
#include <sstream>
#include <iomanip>
#include "../taskflow/taskflow/taskflow.hpp"
#include "../libarchive-master/libarchive/archive.h"
#include "../libarchive-master/libarchive/archive_entry.h"
#include "SHA256.cpp"
#include <cstring>
#include <string.h>
#include "sha2.c"
#include <cmath>
#include <fstream>
#include <filesystem>
#include <fcntl.h>
#include <string>
#include <unordered_map>
#include "../functions.cpp"
#include "../archiving/archiver.cpp"
#include "../cpp-httplib/httplib.h"
#include "json.hpp"

const std::regex hex_pattern("[a-f0-9]*");

struct for_archiving
{
    std::string key_word_hash;
    std::string target;
    std::string key_word_const;
    std::vector<std::string> datas;
};

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
    for (std::string filename : filenames)
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

bool isHex(std::string input)
{
    if (!std::regex_match(input, hex_pattern))
        return false;
    return true;
}

std::vector<uint8_t> hexToBytes(std::string hex)
{
    if (!isHex(hex) || hex.length() % 2 != 0)
    {
        return {};
    }

    std::vector<uint8_t> binary;
    for (unsigned int i = 0; i < hex.length(); i += 2)
    {
        std::string thing = hex.substr(i, 2);
        uint8_t byte = (uint8_t)std::stoi(thing.c_str(), nullptr, 16);
        binary.push_back(byte);
    }

    return binary;
}

void writeTextToFile(std::string hash, std::string input, std::string location)
{
    const char *c = input.c_str();
    std::ofstream fout(location + "/" + hash, std::ios::out);
    fout.write(c, strlen(c));
    fout.close();
}

std::vector<uint8_t> readBinary(std::string file_path)
{
    std::ifstream instream(file_path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
    return data;
}

void writeToFile(std::string hash, std::vector<uint8_t> bytes, std::string location)
{
    std::ofstream fout(location + "/" + hash, std::ios_base::app | std::ios::binary);
    fout.write((char *)&bytes[0], bytes.size() * sizeof(uint8_t));
    fout.close();
}

// create index directory and write to file
void insertSPVList(std::string digest, std::string filename, std::string keyword)
{
    std::string filePath = keyword + "/" + filename;
    std::ofstream outfile(filePath, std::ios::out | std::ios_base::app);
    std::vector<uint8_t> digest_vec = hexToBytes(digest);
    std::string digesthashbytes(digest_vec.begin(), digest_vec.end());
    outfile << digesthashbytes << std::endl;
    outfile.close();
}

std::vector<std::string> miner(std::string target, std::string hash)
{
    std::string messageString;
    std::string digeststring;
    std::vector<uint8_t> prefix;
    std::string emptynonce = "0000000000000000";
    uint64_t start_nonce = 0;
    int itemsPerTask = 1, size = 12;
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

    raw += hash;

    if (oddPrefix)
    {
        raw += "0";
    }

    tf::Taskflow taskflow;
    tf::Executor executor;

    tf::Task entry = taskflow.emplace([]() {});
    tf::Task tfmine = taskflow.for_each_index(0, size, itemsPerTask, [&](int index)
                                              {
        std::string rawtomine = raw + emptynonce;
        std::vector<uint8_t> rawconst = hexToBytes(rawtomine);
        unsigned char message[rawconst.size() + 100];

        for (int i = 0; i < rawconst.size(); i++) {
            message[i] = rawconst[i];
        }

        unsigned char digest[32];
        unsigned int len = rawconst.size();

        unsigned char *noncePtr = message + rawconst.size() - 8;
        uint64_t *n = (long long unsigned int *) noncePtr;
        (*n) = index * (ULLONG_MAX / 100) + start_nonce;
        while (true) {
                (*n)++;
                sha256(message, len, digest);
                if (memcmp(digest, &prefix[0], prefix.size()) == 0) {
                    if (!oddPrefix || (digest[prefix.size()] ^ oddPrefixEnd) < 16) {
                            digeststring = bytesToHex(std::vector<uint8_t>(digest, digest + 32));
                            messageString = bytesToHex(std::vector<uint8_t>(message, message + rawconst.size()));
                            break;
                        }
                    }
                } });

    tf::Task exit = taskflow.emplace([]() {});
    entry.precede(tfmine);
    tfmine.precede(exit);
    executor.run(taskflow).wait();

    return {digeststring, messageString};
}

struct for_archiving mining(std::string data, std::string target, std::string user_hash, std::string key_word, int file_or_txt)
{
    std::string data_hash, spvlistHash, pref_ex, longkeyWordspvlist, key_word_hash;
    std::string emptynonce = "0000000000000000";
    int weight = 0;

    user_hash = generateHash(user_hash);
    key_word_hash = generateHash(key_word);

    // Initialize variables

    // Enter username
    // std::cout << "Username: " << std::endl;
    // std::getline(std::cin, user_hash);
    // user_hash = generateHash(user_hash);

    // // Enter key word
    // std::cout << "Key word: " << std::endl;
    // std::getline(std::cin, key_word);
    // std::string outputKeyword = key_word;
    // key_word = generateHash(key_word);

    std::filesystem::create_directories(key_word_hash);
    writeTextToFile(key_word_hash, key_word, key_word_hash);

    // std::cout << "Provide a prefix: (enter to exit or provide an .extension, e.g \".png\")" << std::endl;
    // std::getline(std::cin, target);
    // if(target == "")
    //     break;

    // std::cout << "Enter data or filename without extension: (enter to exit)" << std::endl;
    // std::getline(std::cin, data);
    // if (data == "")
    //     break;

    // substring the extension
    // if (data.find(".") == std::string::npos) {
    unsigned char digest[32];
    unsigned char shaInput[256010];
    std::vector<uint8_t> buffer;
    if (file_or_txt == 1)
    {
        std::string file = data.substr(0, data.find("."));
        std::string extension = data.substr(data.find("."), data.length());
        size_t endOfTarget = file.length();
        target = file.substr(0, endOfTarget);
        target = generateHash(target).substr(0, 4);
        buffer = readBinary(data);
        std::vector<std::string> allDigests;
        int left_over = buffer.size() % 256000;
        int number_of_chunks = floor(buffer.size() / 256000);
        if (buffer.size() > 256000)
        {
            for (int i = 0; i <= number_of_chunks; i++)
            {
                std::vector<uint8_t> tmpBuffer(&buffer[i], &buffer[i + 256000]);
                // turn to vector uint to char array
                std::copy(tmpBuffer.begin(), tmpBuffer.end(), shaInput);
                sha256(shaInput, tmpBuffer.size(), digest);
                allDigests.push_back(bytesToHex(std::vector<uint8_t>(digest, digest + 32)));
            }
            if (number_of_chunks % 2 != 0)
            {
                std::vector<uint8_t> tmpBuffer(&buffer[number_of_chunks * 2560000], &buffer[(number_of_chunks * 2560000) + left_over]);
                std::copy(tmpBuffer.begin(), tmpBuffer.end(), shaInput);
                sha256(shaInput, tmpBuffer.size(), digest);
                allDigests.push_back(bytesToHex(std::vector<uint8_t>(digest, digest + 32)));
            }
            int amount_of_hashes = allDigests.size();
            while (allDigests.size() != 1)
            {
                std::string tmpString;
                if (amount_of_hashes == 0 && allDigests.size() != 2)
                {
                    tmpString = allDigests[0] + allDigests[0];
                    amount_of_hashes = allDigests.size();
                }
                else
                {
                    tmpString = allDigests[0] + allDigests[1];
                }
                const unsigned char *tmp_sha_input = reinterpret_cast<const unsigned char *>(tmpString.c_str());
                sha256(tmp_sha_input, tmpString.length(), digest);
                allDigests.push_back(bytesToHex(std::vector<uint8_t>(digest, digest + 32)));
                allDigests.erase(allDigests.begin(), allDigests.begin() + 2);
                amount_of_hashes--;
            }
        }
        else
        {
            std::copy(buffer.begin(), buffer.end(), shaInput);
            sha256(shaInput, buffer.size(), digest);
        }
        data_hash = bytesToHex(std::vector<uint8_t>(digest, digest + 32));
    }
    else
    {
        data_hash = generateHash(data);
    }

    std::string hash = key_word_hash + user_hash + data_hash;
    std::vector<std::string> digestString = miner(target, hash);
    longkeyWordspvlist += digestString[0];
    writeToFile(digestString[0], hexToBytes(digestString[1]), key_word_hash);
    if (file_or_txt == 1)
    {
        writeToFile(data_hash, buffer, key_word_hash);
    }
    else
    {
        writeTextToFile(data_hash, data, key_word_hash);
    }
    weight += pow(16, target.length());

    size_t new_pref_len = 0;

    // Determine how long the longest prefix must be
    if (fmod(log2(weight) / log2(16), 1.0) == 0.0)
    {
        new_pref_len = ceil(log2(weight) / log2(16)) + 1;
    }
    else
    {
        new_pref_len = ceil(log2(weight) / log2(16));
    }

    pref_ex = "21e8000000000000";
    pref_ex = pref_ex.substr(0, new_pref_len);

    spvlistHash = generateHash(longkeyWordspvlist);
    std::vector<std::string> digestMsg = miner(pref_ex, spvlistHash);

    insertSPVList(longkeyWordspvlist, spvlistHash, key_word_hash);
    writeToFile(digestMsg[0], hexToBytes(spvlistHash), key_word_hash);

    std::vector<std::string> datas;
    for (const auto &entry : std::filesystem::directory_iterator(key_word_hash))
    {
        datas.push_back(entry.path());
    }

    std::string keyWordConst = key_word_hash + ".tar";
    // write_archive(keyWordConst.c_str() , datas);
    // std::filesystem::remove_all(key_word_hash);
    // archiver(key_word_hash, target);

    std::string key_hash_target[] = {key_word_hash, target, keyWordConst};

    struct for_archiving ready;
    ready.key_word_hash = key_word_hash;
    ready.target = target;
    ready.key_word_const = keyWordConst;
    ready.datas = datas;

    return ready;
}


int main()
{
    httplib::Server server;

    if (!server.is_valid())
    {
        std::cout << "server has an error..." << std::endl;
        return -1;
    }
    struct for_archiving writing_file;

    server.Post("/mine", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        std::string user = req.get_param_value("input_1");
        std::string key_word = req.get_param_value("input_2");
        writing_file.key_word_hash = generateHash(key_word);
        // std::string key_word;
        std::string target;
        std::string data;
        std::string archive_ready;
        if(req.has_param("input_5")) {
            target = req.get_param_value("input_3");
            data = req.get_param_value("input_4");
            archive_ready = req.get_param_value("input_5");
            std::cout << "archive outside: " << archive_ready << std::endl;
            std::thread([&, archive_ready]() {
            writing_file = mining(data, target, user, writing_file.key_word_hash, 0);
            if(archive_ready == "true"){
                write_archive(writing_file.key_word_const.c_str() , writing_file.datas);
                archiver(writing_file.key_word_hash, writing_file.target);
                // std::filesystem::remove_all(writing_file.key_word_hash);
            }}).detach();
       } else {
            data= req.get_param_value("input_3");
            archive_ready = req.get_param_value("input_4");
            std::thread([&, archive_ready]() { 
            writing_file = mining(data, "", user, writing_file.key_word_hash, 1);
            if(archive_ready == "true"){
                std::cout << "archive ready: " << archive_ready << std::endl;
                write_archive(writing_file.key_word_const.c_str() , writing_file.datas);
                archiver(writing_file.key_word_hash, writing_file.target);
                // std::filesystem::remove_all(writing_file.key_word_hash);
            }}).detach();
       }

       res.set_content(writing_file.key_word_hash, "text/plain");
       res.status = 200; });

    server.Get("/send_data", [&](const httplib::Request &req, httplib::Response &res){
        std::string key_word_hash = req.get_param_value("key_word_hash");        
        nlohmann::json data_send;
            std::cout <<"key_word_hash: "<< key_word_hash << std::endl;
        data_send[key_word_hash] = key_word_hash;
        for (const auto &entry : std::filesystem::directory_iterator(key_word_hash))
        {
            std::string file_path = entry.path();
            std::string file_name = file_path.substr(file_path.find_last_of('/'), file_name.length());
            std::vector<uint8_t> fileContents;
            fileContents = readBinary(file_path);
            data_send.push_back({file_name, fileContents});
        }
            // std::filesystem::remove_all(writing_file.key_word_hash);
            std::string json_str = data_send.dump();
            res.set_content(json_str, "application/json"); 
    });

    server.Options("/(.*)", [&](const httplib::Request & /*req*/, httplib::Response &res)
                   {
        res.set_header("Access-Control-Allow-Methods", " POST, GET, OPTIONS");
        res.set_header("Content-Type", "text/html; charset=utf-8");
        res.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Connection", "close"); });

    server.listen("0.0.0.0", 5557);

    return 0;
}