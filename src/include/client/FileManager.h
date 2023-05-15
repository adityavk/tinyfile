#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>
#include "filesystem.hpp"

class FileManager {
private:
    std::filesystem::path outDirPath;
public:
    FileManager(const std::string& outDir);

    void saveCompressedOutput(const std::vector<char>& output, const std::string& inputFilePath, const std::string& suffix = "") const;
    
    std::vector<char> readFile(const std::string& filePath) const;
    std::vector<std::string> readLines(const std::string& filePath) const;
    
    ~FileManager() = default;
};


#endif //FILE_MANAGER_H
