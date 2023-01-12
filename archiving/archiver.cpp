#include <iostream>
#include "../libarchive-master/libarchive/archive.h"
#include "../libarchive-master/libarchive/archive_entry.h"
#include <regex>
#include <sstream>
#include <cstddef>
#include <fstream>
#include <map>
#include <cmath>
#include <fcntl.h>
#include "../functions.h"

namespace fs = std::filesystem;

static int
copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;)
    {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK)
        {
            fprintf(stderr, "%s\n", archive_error_string(aw));
            return (r);
        }
    }
}

std::string convertToString(char *a)
{
    std::string s = a;
    return s;
}

static void extract(const char *filename)
{
  struct archive *a;
  struct archive *ext;
  struct archive_entry *entry;
  int flags;
  int r;

  /* Select which attributes we want to restore. */
  flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;

  a = archive_read_new();
  archive_read_support_format_all(a);
  archive_read_support_filter_all(a);
  ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);

  if ((r = archive_read_open_filename(a, filename, 10240)))
    exit(1);
  for (;;) {
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF)
      break;
    if (r < ARCHIVE_OK)
      fprintf(stderr, "%s\n", archive_error_string(a));
    if (r < ARCHIVE_WARN)
      exit(1);
    r = archive_write_header(ext, entry);
    if (r < ARCHIVE_OK)
      fprintf(stderr, "%s\n", archive_error_string(ext));
    else if (archive_entry_size(entry) > 0) {
      r = copy_data(a, ext);
      if (r < ARCHIVE_OK)
        fprintf(stderr, "%s\n", archive_error_string(ext));
      if (r < ARCHIVE_WARN)
        exit(1);
    }
    r = archive_write_finish_entry(ext);
    if (r < ARCHIVE_OK)
      fprintf(stderr, "%s\n", archive_error_string(ext));
    if (r < ARCHIVE_WARN)
      exit(1);
  }
  archive_read_close(a);
  archive_read_free(a);
  archive_write_close(ext);
  archive_write_free(ext);
}

std::string returnGreatestFileName(std::string file_path, std::string prefixString, int &prefix)
{
    std::string greatestFileName;
    for (const auto &dirEntry : std::filesystem::directory_iterator(file_path))
    {
        const std::string &s = dirEntry.path();
        char* compareToThis = "21e8000000000000";
        size_t findLast = s.find_last_of('/') + 1;
        size_t sLength = s.length();
        size_t prefixStringLength = prefixString.length();
        std::string file_name = s.substr(findLast, sLength);
        size_t fileNameLength = file_name.length();
        // std::cout << "x: " << prefixString << std::endl;
        if (file_name.substr(0, 4) == "21e8")
        {
            // std::string tmp = file_name.substr(prefixStringLength, fileNameLength);
            const char *charFile = file_name.c_str();
            int i = 0;
            while (charFile[i] == compareToThis[i])
            {
                i++;
            }
            if (i > prefix)
            {
                prefix = i;
                greatestFileName = file_name;
            }
        }
    }
    return greatestFileName;
}

std::string getFileInfo(std::string filePath, int prefixLen, int x)
{
    std::ifstream indata(filePath);
    std::string finalText = "";

    if (!indata.is_open()) {
        return "";
    }

    if (x == 0 || x == 1) {
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(indata)), std::istreambuf_iterator<char>());
        if(x == 0) {
            finalText = bytesToHex(data).substr(prefixLen + 4, 64);
        } else if (x == 1) {
            finalText = bytesToHex(data);
        }
    } else if (x == 2) {
        std::stringstream buffer;
        buffer << indata.rdbuf();
        finalText = buffer.str();
    }

    //std::cout << "finalText: " << finalText << std::endl;
    return finalText;
}

int prefixLengthIdentifier(std::string fileName, std::string insideFile) {
    int prefixLength = 0;
    const char *nameChar = fileName.c_str();
    const char *insideChar = insideFile.c_str();
    for (int i = 0; i < fileName.length(); i++) {
        if (nameChar[i] != insideChar[i]) {
            break;
        }
        prefixLength++;
    }
    return prefixLength;
}

void archiver(std::string file_path, std::string prefixString)
{
    char tmp[256];
    getcwd(tmp, 256);
    std::string destination = convertToString(tmp);
    std::string file_path_string = file_path + ".tar";
    const char* extracter = file_path_string.c_str();
    extract(extracter);
    int prefix = 0;
    std::string greatest_file_path = file_path + "/" + returnGreatestFileName(file_path, prefixString, prefix);
    std::string newFileContents = getFileInfo(greatest_file_path, prefix, 1);
    std::vector<std::string> archiveFiles;
    std::cout << "greatest_file_path: "<< greatest_file_path << std::endl;

    std::string x = getFileInfo(file_path + "/" + newFileContents, prefix, 1);
    // std::cout << "x: " << getFileInfo(file_path + "/" + newFileContents, prefix, 1)<< std::endl;
    for (size_t i = 0; i < x.length() - 2; i += 64) {
        archiveFiles.push_back(x.substr(i, 64));
    }
    for(auto i: archiveFiles) {
        std::cout << "Archive files: " << i << std::endl;
    }
    sort(archiveFiles.begin(), archiveFiles.end());
    archiveFiles.erase(unique(archiveFiles.begin(), archiveFiles.end()), archiveFiles.end());
    std::multimap<int, std::string, std::greater<int>> hashesOrdered;
    for (std::string spvfile : archiveFiles)
    {
        std::string finalDestination = destination + "/" + file_path + "/" + spvfile;
        std::string xd = getFileInfo(finalDestination, prefix, 1);
        size_t xd_length = xd.length();
        int prefixLength = prefixLengthIdentifier(spvfile, xd);
        if (prefixLength % 2 == 1 && !xd.empty())
        {
            xd = xd.substr((xd_length - 81), 64);
        }
        else if (prefixLength % 2 == 0 && !xd.empty())
        {
            xd = xd.substr((xd_length - 80), 64);
        }
        hashesOrdered.insert({pow(16, prefixLength), xd});
    }

    std::map<int, std::string>::iterator it;
    for (it = hashesOrdered.begin(); it != hashesOrdered.end(); it++) {
        std::string finalFilePath = destination + "/" + file_path;
        finalFilePath = finalFilePath + "/" + (*it).second;
        std::string finalOutput = getFileInfo(finalFilePath, 0, 2);
        std::cout << finalOutput << " -> "<<it->first << std::endl;
    }
    
    std::filesystem::remove_all(file_path);
    exit(0);
}
