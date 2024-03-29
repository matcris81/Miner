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
#include <chrono>
#include <thread>
#include <cstdio>
#include <atomic>

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
    instream.close();
    return data;
}

std::vector<std::vector<uint8_t>> read256Binary(std::string file_path) {
    std::vector<std::vector<uint8_t>> data;

    FILE *file = fopen(file_path.c_str(), "rb");
    if (!file) {
        std::cerr << "Error opening file: " << std::strerror(errno) << '\n';
        return data;
    }

    std::vector<uint8_t> buffer(256000);
    size_t bytes_read = 0;
    while ((bytes_read = fread(buffer.data(), sizeof(uint8_t), buffer.size(), file)) > 0) {
        if (bytes_read < buffer.size()) {
            buffer.resize(bytes_read);
        }
        data.push_back(buffer);
    }

    if (ferror(file)) {
        std::cerr << "Error reading file: " << std::strerror(errno) << '\n';
    }
    fclose(file);

    return data;
}

void writeToFile(std::string hash, std::vector<uint8_t> bytes, std::string location)
{
    std::ofstream fout(location + "/" + hash, std::ios_base::app | std::ios::binary);
    fout.write((char *)&bytes[0], bytes.size() * sizeof(uint8_t));
    fout.close();
}

//   create index directory and write to file
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
    tf::Task tfmine = taskflow.for_each_index(0, size, itemsPerTask, [&](int index) {
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
        }
    });

    tf::Task exit = taskflow.emplace([]() {});
    entry.precede(tfmine);
    tfmine.precede(exit);
    executor.run(taskflow).wait();

    return {digeststring, messageString};
}

std::string rename_spvlist(std::string key_word_hash, std::string target) {
    // read in bytes from tmp file
    unsigned char digest[32];
    std::vector<uint8_t> spvBinary;
    spvBinary = readBinary(key_word_hash + "/tmp");

    // hash all the bytes together into one
    unsigned char* spvBytes = spvBinary.data();
    std::copy(spvBinary.begin(), spvBinary.end(), spvBytes);
    sha256(spvBytes, spvBinary.size(), digest);
    std::string digest_string = bytesToHex(std::vector<uint8_t>(digest, digest + 32));
    std::string digest_tmp = key_word_hash + "/tmp";
    std::string digest_path = key_word_hash + "/" + digest_string;

    // rename the file with all the bytes
    if(rename(digest_tmp.c_str(), digest_path.c_str()) != 0) {
        std::cout << "Error renaming" << std::endl;
    }

    return digest_string;
}

std::string make_chunk(std::string target, std::string key_word_hash, std::string chunks_hashed) {
    std::string chunk_hash = "6c87f68371b28954707ebb92afee7ccffb74c6f71ec8fea8a98cf6104289585b";

    // mine prefix onto "chunk" hashed
    std::vector<std::string> target_chunk_mined;
    target_chunk_mined = miner(target, chunk_hash);

    // write all chunks hashed to file that is target and hash mined
    writeToFile(target_chunk_mined[0], hexToBytes(chunks_hashed), key_word_hash);
    
    return target_chunk_mined[0];
}

std::string make_root(std::string target, std::string key_word_hash, std::string all_chunks) {
    std::string root_hash = "4813494d137e1631bba301d5acab6e7bb7aa74ce1185d456565ef51d737677b2";
    
    //mine prefix onto "root" hashed
    std::vector<std::string> target_root_mined;
    target_root_mined = miner(target, root_hash);
    // write all chunk file names to target_root_miner[0]
    writeToFile(target_root_mined[0], hexToBytes(all_chunks), key_word_hash);
    
    return target_root_mined[0];
}

struct for_archiving mining(std::string data, std::string target, std::string user_hash, std::string key_word, std::string key_word_hash , int file_or_txt)
{

    std::cout << target << std::endl;
    std::string data_hash, pref_ex, longkeyWordspvlist, all_chunks;
    std::string emptynonce = "0000000000000000";
    int weight = 0;
    char tmp[256];
    getcwd(tmp, 256);
    std::string path;
    path = tmp;
    path += "/" + data;

    user_hash = generateHash(user_hash);

    std::filesystem::create_directories(key_word_hash);
    writeTextToFile(key_word_hash, key_word, key_word_hash);
    std::string hashes;

    unsigned char digest[32];
    unsigned char shaInput[256010];
    std::string root_file_name;
    std::string chunks_hashed;
    std::string hash;
    std::vector<std::string> digestString;

    if (file_or_txt == 1)
    {
        std::string file = data.substr(0, data.find("."));
        std::string extension = data.substr(data.find("."), data.length());
        size_t endOfTarget = data.length();
        target = extension.substr(1, endOfTarget);
        target = generateHash(target).substr(0, 4);
        std::ifstream in_file(data, std::ios::binary);
        in_file.seekg(0, std::ios::end);
        long int file_size = in_file.tellg();
        long int number_of_chunks = floor(file_size / 256000);
        int left_over = file_size % 256000;
        bool large_file = false;
        std::vector<std::vector<uint8_t>> data_chunks;
        data_chunks = read256Binary(path);
        std::vector<std::string> spv_hash_chunks;
        int i = 0;
        while (data_chunks.size() > 1) {
            bool root = false;
            if(number_of_chunks >= 8000) {
                for(int i = 0; i < 8000; i++){
                    large_file = true;
                    std::copy(data_chunks[0].begin(), data_chunks[0].end(), shaInput);
                    sha256(shaInput, data_chunks[0].size(), digest);

                    writeToFile(bytesToHex(std::vector<uint8_t>(digest, digest + 32)), data_chunks[0], key_word_hash);
                    writeToFile("tmp", std::vector<uint8_t>(digest, digest + 32), key_word_hash);
                    data_chunks.erase(data_chunks.begin());
                }
                // rename the file and get the name
                chunks_hashed = rename_spvlist(key_word_hash, target);
                all_chunks += make_chunk(target, key_word_hash, chunks_hashed);
            } else if (number_of_chunks <= 0) {
                break;
            } else {
                for(int j = 0; j < number_of_chunks + 1; j++) {
                    std::copy(data_chunks[0].begin(), data_chunks[0].end(), shaInput);
                    sha256(shaInput, data_chunks[0].size(), digest);

                    writeToFile(bytesToHex(std::vector<uint8_t>(digest, digest + 32)), data_chunks[0], key_word_hash);
                    writeToFile("tmp", std::vector<uint8_t>(digest, digest + 32), key_word_hash);
                    data_chunks.erase(data_chunks.begin());
                }
                chunks_hashed = rename_spvlist(key_word_hash, target);
                if(large_file) {
                    all_chunks += make_chunk(target, key_word_hash, chunks_hashed);
                    root_file_name = make_root(target, key_word_hash, all_chunks);
                } else {
                    root_file_name = make_root(target, key_word_hash, chunks_hashed);
                }
            }
            // add to string to make spv list
            number_of_chunks -= 8000;
            i++;
        }
        hash = key_word_hash + user_hash + root_file_name;
        digestString = miner(target, hash);
    } else {
        data_hash = generateHash(data);
        std::string hash = key_word_hash + user_hash + data_hash;
        std::cout << "Target" << target << std::endl;
        digestString = miner(target, hash);
    }
    std::string spvlistHash;
    spvlistHash = generateHash(chunks_hashed);


    writeToFile(digestString[0], hexToBytes(digestString[1]), key_word_hash);
    if (file_or_txt == 0) {
        writeTextToFile(data_hash, data, key_word_hash);
    }

    weight += pow(16, target.length());

    size_t new_pref_len = 0;

    // Determine how long the longest prefix must be
    if (fmod(log2(weight) / log2(16), 1.0) == 0.0) {
        new_pref_len = ceil(log2(weight) / log2(16)) + 1;
    } else {
        new_pref_len = ceil(log2(weight) / log2(16));
    }

    pref_ex = "21e8000000000000";
    pref_ex = pref_ex.substr(0, new_pref_len);

    std::vector<std::string> digestMsg = miner(pref_ex, spvlistHash);

    writeToFile(digestMsg[0], hexToBytes(digestString[0]), key_word_hash);

    std::vector<std::string> datas;
    for (const auto &entry : std::filesystem::directory_iterator(key_word_hash)) {
        datas.push_back(entry.path());
    }

    std::string keyWordConst = key_word_hash + ".tar";

    std::string key_hash_target[] = {key_word_hash, target, keyWordConst};

    struct for_archiving ready;
    ready.key_word_hash = key_word_hash;
    ready.target = target;
    ready.key_word_const = keyWordConst;
    ready.datas = datas;

    return ready;
}

std::string findFile(const std::string &dir, const std::string &file) {  
    for (const auto &entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().filename() == file) {
            std::string file_contents;
            std::vector<uint8_t> file_bytes;
            file_bytes = readBinary(dir+ "/" +file);
            file_contents = bytesToHex(file_bytes);
            return file_contents;
        }
    }
    return "not found";
}

std::vector<uint8_t> fileBytes(const std::string &dir, const std::string &file) {
    std::vector<uint8_t> file_bytes;
    for (const auto &entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().filename() == file) {
            std::string file_contents;
            file_bytes = readBinary(dir+ "/" +file);
            return file_bytes;
        }
    }
    return file_bytes;
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
    std::string greatest_file;

    server.Post("/mine", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        std::string user = req.get_param_value("input_1");
        std::string key_word = req.get_param_value("input_2");
        writing_file.key_word_hash = generateHash(key_word);
        std::string target;
        std::string data;
        std::string archive_ready;
        if(req.has_param("input_5")) {
            target = req.get_param_value("input_3");
            std::cout << "Target " << target << std::endl;
            data = req.get_param_value("input_4");
            archive_ready = req.get_param_value("input_5");
            std::cout << target << std::endl;
            writing_file = mining(data, target, user, key_word, writing_file.key_word_hash, 0);
            if(archive_ready == "true"){
                write_archive(writing_file.key_word_const.c_str() , writing_file.datas);
                std::filesystem::remove_all(writing_file.key_word_hash);
                greatest_file = archiver(writing_file.key_word_hash, writing_file.target);
                std::cout << "Done" << std::endl;
            }
       } else {
            data = req.get_param_value("input_3");
            archive_ready = req.get_param_value("input_4");
            writing_file = mining(data, "", user, key_word , writing_file.key_word_hash, 1);
            if(archive_ready == "true"){
                write_archive(writing_file.key_word_const.c_str() , writing_file.datas);
                std::filesystem::remove_all(writing_file.key_word_hash);
                greatest_file = archiver(writing_file.key_word_hash, writing_file.target);
                std::cout << "Done" << std::endl;
            }
       }

       std::string content;
       content = writing_file.key_word_hash + "/" + greatest_file + " " + writing_file.target;

        res.set_content(content, "text/plain");
        res.status = 200;
    });

    server.Get("/spv_list", [&](const httplib::Request &req, httplib::Response &res){
        res.set_header("Access-Control-Allow-Origin", "*");
        std::string key_word_hash = req.get_param_value("key_word_hash");
        std::string greatest_file = req.get_param_value("greatest");

        // get contents of greatest file
        std::string greatest_file_contents;
        greatest_file_contents = findFile(key_word_hash, greatest_file);

        // get datahash
        std::string full_hash;
        full_hash = findFile(key_word_hash, greatest_file_contents);

        std::string data_hash;
        data_hash = full_hash.substr(132, 64);

        // get root file
        std::string root;
        root = findFile(key_word_hash, data_hash);

        // get spv files
        std::string spv_files;
        spv_files = findFile(key_word_hash, root);

        std::filesystem::remove_all(key_word_hash);
        res.set_content(spv_files, "text/plain"); 

    });

    server.Get("/get_file_bytes", [&](const httplib::Request &req, httplib::Response &res){ 
        res.set_header("Access-Control-Allow-Origin", "*");
        nlohmann::json data_send;
        // get directory and file name
        std::string key_word_hash = req.get_param_value("key_word_hash");
        std::string file = req.get_param_value("file");

        std::vector<uint8_t> bytes;
        bytes = fileBytes(key_word_hash, file);

        // add to json and send back to front
        data_send.push_back({bytes});
        std::string file_path = key_word_hash + "/" + file;
        int result = remove(file_path.c_str());

        // load in all the bytes from file
        std::string json_str = data_send.dump();
        res.set_content(json_str, "text/plain");
    });

    server.Post("/extract_directory", [&](const httplib::Request &req, httplib::Response &res){ 
        res.set_header("Access-Control-Allow-Origin", "*");
        // get keyword 
        std::string keyword = req.get_param_value("directory_name");
        unsigned char shaInput[keyword.length()];
        unsigned char directory_name[32];

        // string to unsigned char
        for (int i = 0; i < keyword.length(); i++) {
            shaInput[i] = keyword[i];
        }

        // convert back to hex
        sha256(shaInput, keyword.length(), directory_name);
        std::vector<uint8_t> vector_directory(directory_name, directory_name + 32);

        //get current path, find file, extract said file
        std::filesystem::path current_path = std::filesystem::current_path();
        for (const auto &entry : std::filesystem::directory_iterator(current_path)) {
            std::string file_name = entry.path().filename();
            if(file_name == bytesToHex(vector_directory) + ".tar") {
                extract(file_name.c_str());
                break;
            }
        }
        
        // send directory name to front end
        res.set_content(bytesToHex(vector_directory), "text/plain");
    });

    server.Get("/directory_contents", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");

        //create json and get directory name
        nlohmann::json directory_contents;
        std::string directory_name = req.get_param_value("directory_name");

        // get all the files, file_contents and add to json
        for (const auto &entry : std::filesystem::directory_iterator(directory_name)) {
            std::vector<uint8_t> file_contents = readBinary(entry.path());
            directory_contents.push_back({entry.path().filename(), file_contents});
        }

        // dump json to string and send back
        std::string data_string;
        data_string = directory_contents.dump();
        std::filesystem::remove_all(directory_name);
        res.set_content(data_string, "text/plain");
    });

    server.Options("/(.*)", [&](const httplib::Request & /*req*/, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Methods", " POST, GET, OPTIONS");
        res.set_header("Content-Type", "text/html; charset=utf-8");
        res.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Connection", "close"); 
    });

    server.listen("0.0.0.0", 5557);


    return 0;
}