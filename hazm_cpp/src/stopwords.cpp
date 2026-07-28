#include "hazm/stopwords.hpp"
#include "hazm/utils.hpp"

namespace hazm {

std::unordered_set<std::string> stopwords_list(const std::string& path) {
    std::string sp = path.empty() ? data_path("stopwords.dat") : path;
    return load_stopwords(sp);
}

} // namespace hazm