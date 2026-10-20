// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"

ToolRegistry::ToolRegistry() {
    buckets_.resize(Initalbucketsize);
}
// TODO: create your initial buckets
ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::size_t hashnum = 0;
    for (char c: key) {
        hashnum = (hashnum * 31) + static_cast<unsigned char>(c);
    }
    return hashnum;
}
void ToolRegistry::rehash(std::size_t new_bucket_size) {
    std::vector<std::vector<std::unique_ptr<Tool>>> BIGbucket(new_bucket_size);
    for (auto& b : buckets_) {
        for (auto& tool : b) {
            std::size_t index = hash(tool->name())% new_bucket_size;
            BIGbucket[index].push_back(std::move(tool));

        }
    }
    buckets_ = std::move(BIGbucket);
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if (tool == nullptr) {
        return false;
    }


    if (find(tool->name()) != nullptr) {
        return false;
    }
    if (  (double (elements_+1) / double (buckets_.size())) > loadfactor){
        rehash(buckets_.size() *2);

    }
    std::size_t H = hash(tool->name());
    std::size_t index = H % buckets_.size();

    buckets_[index].push_back(std::move(tool));
    elements_++;

    return true;   // TODO
}

Tool* ToolRegistry::find(std::string_view name) const {
    std::size_t H = hash(name);
    std::size_t index = H % buckets_.size();
    for (auto& N : buckets_[index] ) {

        if (N->name() == name) {
            return N.get();
        }

    }
    return nullptr;   // TODO
}

std::size_t ToolRegistry::size() const noexcept {
    return elements_;   // TODO
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (auto& bucket: buckets_) {
        for (auto& ptr: bucket) {
            fn(*ptr);
        }

    }
    // TODO
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();   // TODO
}

double ToolRegistry::load_factor() const noexcept {
    if (buckets_.empty()) {
        return  0;
    }

    return double(elements_) / double (buckets_.size());// TODO
}

std::size_t ToolRegistry::max_probe_length() const {
    std::size_t longestBucket = 0;
    for (auto& bucket: buckets_) {
        if (bucket.size()> longestBucket) {
            longestBucket = bucket.size();
        }
    }
    return longestBucket;   // TODO
}
