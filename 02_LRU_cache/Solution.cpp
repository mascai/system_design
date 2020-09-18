#include "Common.h"

#include <string>
#include <list>
#include <unordered_map>
#include <mutex>
#include <iostream>


using namespace std;



class LruCache : public ICache {
public:
    LruCache(
            shared_ptr<IBooksUnpacker> books_unpacker,
            const Settings& settings
    ): books_unpacker_(std::move(books_unpacker)), max_size_(settings.max_memory) {}

    BookPtr GetBook(const string& book_name) override {
        lock_guard<mutex> guard(m_);
        if (auto it = book_names_.find(book_name); it != book_names_.end()) {
            cache_.splice(cache_.begin(), cache_, it->second); // replace element without iterators invalidation
            return cache_.front();
        }
        auto book = books_unpacker_->UnpackBook(book_name);
        if (book->GetContent().size() > max_size_) {
            cache_.clear();
            book_names_.clear();
            cur_size = 0;
            return book;
        }
        cur_size += book -> GetContent().size();
        if (cur_size > max_size_) {
            while (!cache_.empty() && cur_size > max_size_) {
                cur_size -= cache_.back().get()->GetContent().size();
                string name = cache_.back().get()->GetName();
                cache_.pop_back();
                book_names_.erase(name);
            }
        }
        cache_.push_front(move(book));
        book_names_[book_name] = cache_.begin();
        return cache_.front();
    }
private:
    mutable mutex m_;
    size_t cur_size = 0;
    size_t max_size_; // max size of cache in bytes
    shared_ptr<IBooksUnpacker> books_unpacker_;
    list<BookPtr> cache_;
    unordered_map<string, list<BookPtr>::iterator> book_names_;
};


unique_ptr<ICache> MakeCache(
        shared_ptr<IBooksUnpacker> books_unpacker,
        const ICache::Settings& settings
) {
    mutex m_;
    lock_guard<mutex> guard(m_);
    return make_unique<LruCache>(books_unpacker, settings);
}