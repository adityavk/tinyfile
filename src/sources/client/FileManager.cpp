#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "FileManager.h"

using namespace std;
namespace fs = std::filesystem;

FileManager::FileManager(const string& outDir): outDirPath(outDir) {
    if (!fs::is_directory(outDirPath) && !fs::exists(outDirPath)) {
        fs::create_directory(outDirPath);
    }
}

void FileManager::saveCompressedOutput(const vector<char>& output, const string& inputFilePath, const string& suffix) const {
    fs::path inputFile(inputFilePath);
    string outputFileName = inputFile.stem().string() + suffix + ".compressed" + inputFile.extension().string();
    const auto filePath = outDirPath / outputFileName;
    ofstream fout(filePath, ios::out | ios::binary);
    fout.write(output.data(), output.size());
    fout.close();
}

vector<char> FileManager::readFile(const string& filePath) const {
    fs::path p = filePath;
    if (!fs::is_regular_file(p)) {
        cerr<<"Provided path doesn't exist or is not a regular file, returning...\n";
        return {};
    }

    ifstream fin(filePath, ifstream::binary);
    return vector<char>((istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
}

vector<string> FileManager::readLines(const string& filePath) const {
    ifstream fin(filePath);
    string line;
    vector<string> lines;
    while (getline(fin, line)) {
        lines.push_back(line);
    }
    return lines;
}
