/*
В этой задаче мы поработаем с shared_ptr в многопоточной среде.

Нужно будет реализовать кэш бэкенда электронной библиотеки. На диске есть множество книг — архивированных текстовых файлов. Пользователь запрашивает книгу с заданным названием. Надо её считать, распаковать и вернуть. Чаще всего пользователи запрашивают одни и те же книги, поэтому их хочется кэшировать. При этом запросы приходят в несколько потоков.

Для доступа к книгам используется интерфейс ICache, объявленный в файле Common.h. У него всего один метод GetBook(), который возвращает shared_ptr на книгу (используется синоним BookPtr). Реализация этого метода должна обеспечить правильное кэширование.

Книга представлена интерфейсом IBook. Два его метода GetName() и GetContent() позволяют получить название и текст книги соответственно.

Наконец, считывание и распаковка книги делаются с помощью интерфейса IBooksUnpacker. Его метод UnpackBook() по переданному названию книги возвращает распакованную книгу как объект IBook.

Для создания объекта ICache используется функция MakeCache(), которую вам необходимо реализовать. Она принимает объект IBooksUnpacker, а также объект ICache::Settings - настройки кэша. В нашей задаче настройки содержат всего один параметр max_memory — максимальный суммарный размер всех книг в кэше в байтах, — но в реальности их может быть больше. Именно поэтому мы оформили настройки в виде структуры. Размером книги считается размер её текста в байтах.

Кэширование производится методом вытеснения давно неиспользуемых элементов (Least Recently Used, LRU). Каждый элемент кэша имеет ранг. При вызове метода GetBook(), если книга с таким названием уже есть в кэше, её ранг поднимается до максимального (строго больше, чем у всех остальных). Если такой книги нет в кэше, то она добавляется в кэш, и её ранг, опять же, выставляется в максимальный. При этом, если общий размер книг превышает ограничение max_memory, из кэша удаляются книги с наименьшим рангом, пока это необходимо. Возможно, пока он полностью не будет опустошён. Если же размер запрошенной книги уже превышает max_memory, то после вызова метода кэш остаётся пустым, то есть книга в него не добавляется.

Метод GetBook() может вызываться одновременно из нескольких потоков, поэтому необходимо обеспечить ему безопасность работы в таких условиях.

Используемые на практике реализации LRU кэшей позволяют искать элементы по ключу и удалять давно неиспользуемые элементы за константное время. В данной задаче этого не требуется. Искать давно неиспользуемые элементы можно перебором всех имеющихся.

Помимо Common.h вам даны два файла: main.cpp и Solution.cpp. Первый содержит неполный набор тестов для функции MakeCache, второй — заготовку решения. Реализуйте функцию MakeCache в файле Solution.cpp и сдайте этот файл в тестирующую систему.
*/

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
