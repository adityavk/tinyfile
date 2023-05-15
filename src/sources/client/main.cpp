#include "tiny-file/TinyFile.h"
#include <iostream>
#include <memory>
#include <chrono>
#include "SnappySerializedCompressor.h"
#include "FileManager.h"
#include "CommandLineUtils.h"

using namespace std;

constexpr char DEFAULT_OUT_DIR[] = "output";
constexpr char NO_SERVICE_ARG_NAME[] = "no_service";
constexpr char STATE_ARG_NAME[] = "state";
constexpr char FILENAME_ARG_NAME[] = "file";
constexpr char FILENAMES_ARG_NAME[] = "files";
constexpr char SERVICE_PATH_ARG_NAME[] = "service_path";
constexpr char OUTDIR_ARG_NAME[] = "out_dir";
constexpr char SHM_SEGMENT_SIZE_ARG_NAME[] = "sms_size";
constexpr char STATE_ARG_VALUE_SYNC[] = "SYNC";
constexpr char STATE_ARG_VALUE_ASYNC[] = "ASYNC";

using SnappyPtr = SnappySerializedCompressor*;

vector<char> compressFile(const vector<char>& inputData, SnappyPtr compressorPtr, bool useService) {
    if (useService) {
        return tf_compress(inputData);
    } else {
        return compressorPtr->compress(inputData);
    }
}

void processFile(const string& filePath, const FileManager& fileManager, SnappyPtr compressorPtr, bool useAsync, bool useService) {
    const auto fileBuffer = fileManager.readFile(filePath);
    string fileSuffix = !useService ? "_NoService" : (useAsync ? "_Async" : "");
    if (!useAsync) {
        chrono::steady_clock::time_point start = chrono::steady_clock::now();
        
        auto compressedData = compressFile(fileBuffer, compressorPtr, useService);
        chrono::steady_clock::time_point finish = chrono::steady_clock::now();
        cout << "CST = " << chrono::duration_cast<chrono::microseconds>(finish - start).count() << " us\n";
        
        fileManager.saveCompressedOutput(compressedData, filePath, fileSuffix);
    } else {
        chrono::steady_clock::time_point start = chrono::steady_clock::now();
        tf_compress_async(fileBuffer, [fileManager, filePath, fileSuffix, start](vector<char> compressedData) {
            chrono::steady_clock::time_point finish = chrono::steady_clock::now();
            cout << "CST = " << chrono::duration_cast<chrono::microseconds>(finish - start).count() << " us\n";
            fileManager.saveCompressedOutput(compressedData, filePath, fileSuffix);
        });
    }
}

void freeSnappy(SnappyPtr snappyPtr) {
    if (snappyPtr) {
        delete snappyPtr;
        snappyPtr = nullptr;
    }
}

int main(int argc, char* argv[]) {
    const auto args = parseArgs(argc, argv);
    const auto stateArgIt = args.find(STATE_ARG_NAME);
    bool useAsync = false;
    if (stateArgIt == args.end()) {
        cerr<<"Client's sync/async state not specified, use --state=[SYNC|ASYNC] option to specify!\n";
        return 1;
    } else {
        string stateArgValue = stateArgIt->second;
        if (stateArgValue == STATE_ARG_VALUE_ASYNC) {
            useAsync = true;
        } else if (stateArgValue != STATE_ARG_VALUE_SYNC) {
            cerr<<"Invalid sync/async argument, use --state=[SYNC|ASYNC] option to specify!\n";
            return 1;
        }
    }

    bool useService = (args.find(NO_SERVICE_ARG_NAME) == args.end());
    uint16_t shmSize;

    SnappyPtr snappyPtr = nullptr;
    
    if (!useService) {
        if (!parseUnsignedLongArg(args, SHM_SEGMENT_SIZE_ARG_NAME, shmSize)) {
            exit(1);
        }
        snappyPtr = new SnappySerializedCompressor(shmSize);
    }
    auto fileNameIt = args.find(FILENAME_ARG_NAME);
    auto fileNamesIt = args.find(FILENAMES_ARG_NAME);
    
    if (fileNameIt == args.end() && fileNamesIt == args.end()) {
        // No files to process, exit
        cerr<<"No input files given, use --file or --files option!\n";
        freeSnappy(snappyPtr);
        return 1;
    }

    string outDir = parseStringArg(args, OUTDIR_ARG_NAME, DEFAULT_OUT_DIR);
    cout<<"Compressed files will be saved in "<<outDir<<"\n";
    FileManager fileManager(outDir);
    if (useService) {
        bool success = tf_initialize(parseStringArg(args, SERVICE_PATH_ARG_NAME));
        if (!success) {
            cout<<"Failed to register with the service, check if service is running. If service is running in a different directory, pass its path using --service_path argument \n";
            freeSnappy(snappyPtr);
            return 1;
        }
    }

    if (fileNameIt != args.end()) {
        processFile(fileNameIt->second, fileManager, snappyPtr, useAsync, useService);
    } else {
        for (const auto& filePath: fileManager.readLines(fileNamesIt->second)) {
            processFile(filePath, fileManager, move(snappyPtr), useAsync, useService);
        }
    }

    if (useAsync) {
        // Do something random for sometime
        unsigned long period = 2e6;
        unsigned long total = period * 10;
        for (unsigned long i = 0; i < total; ++i) {
            if (i % period == 0) {
                cout<<"Continuing random task\n";
            }
        }
        cout<<"Waiting on async requests\n";
        tf_wait_all_async_requests();
        cout<<"Async requests finished\n";
    }
    
    if (useService) {
        bool success = tf_terminate();
        if (!success) {
            cout<<"Failed to deregister with the service\n";
        }
        
    }
    
    freeSnappy(snappyPtr);
    return 0;
}
