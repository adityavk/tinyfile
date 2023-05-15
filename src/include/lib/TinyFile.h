#ifndef TINY_FILE_H
#define TINY_FILE_H

#include <functional>
#include <string>
#include <vector>

using TFCompressionCallback = std::function<void(std::vector<char>)>;

extern bool tf_initialize(std::string relativeServicePath = "");

extern bool tf_terminate();

extern std::vector<char> tf_compress(std::vector<char> inputData);

extern void tf_compress_async(std::vector<char> inputData, TFCompressionCallback callback);

extern void tf_wait_all_async_requests();

#endif //TINY_FILE_H

