#include "BloomFilter.h"
#include <iostream>
#include <fstream>
#include <sstream>
template <std::size_t N>
BloomFilter<N>::BloomFilter(unsigned int num_hashes):num_hashes(num_hashes), bits(), server(new CDNServer)
{
    for (std::size_t i = 0; i < num_hashes; ++i) {
        seeds.push_back(31 + i * 37);
    }
}

template <std::size_t N>
BloomFilter<N>::BloomFilter(const BloomFilter& other):num_hashes(other.num_hashes), bits(other.bits), seeds(other.seeds)
{
    server = new CDNServer(*other.server);
}

template <std::size_t N>
BloomFilter<N>::BloomFilter(BloomFilter&& other) noexcept:num_hashes(other.num_hashes), bits(std::move(other.bits)), seeds(std::move(other.seeds))
{
    server = other.server;
    other.server = nullptr;
}

template <std::size_t N>
BloomFilter<N>::~BloomFilter()
{
    delete server;
    server = nullptr;
}

template <std::size_t N>
void BloomFilter<N>::add(const std::string& item)
{
    for(auto seed:seeds)
    {
        size_t hash_value = hash(item, seed) % N;
        bits.set(hash_value);
    }
}

template <std::size_t N>
void BloomFilter<N>::add(std::string&& file_name)
{
    //open the file
    std::ifstream file(file_name);
    if(!file.is_open())
    {
        std::cerr << "Error: Cannot open file: "<< file_name << std::endl;
        return;
    }

    std::string line, word;
    while(std::getline(file, line))
    {
        std::stringstream ss(line);
        while(std::getline(ss, word, ','))
            add(word);
    }
    file.close();
}

template <std::size_t N>
bool BloomFilter<N>::possiblyContains(const std::string& item) const
{
    for (auto seed:seeds)
    {
        size_t hash_value = hash(item, seed) % N;
        if(!bits.test(hash_value))
            return false;
    }
    return true;
}

template <std::size_t N>
bool BloomFilter<N>::possiblyContains(std::string&& item) const
{
   return possiblyContains(static_cast<const std::string&>(item));
}

template <std::size_t N>
bool BloomFilter<N>::certainlyContains(const std::string& item) const
{
    if(!possiblyContains(item))
        return false;
    if(!server->checkWord(item))
        return false;
    return true;
}

template <std::size_t N>
bool BloomFilter<N>::certainlyContains(std::string&& item) const
{
    return certainlyContains(static_cast<const std::string&>(item));
}

template <std::size_t N>
void BloomFilter<N>::reset()
{
    bits.reset();
}

template <std::size_t N>
BloomFilter<N>& BloomFilter<N>::operator&(const BloomFilter& other)
{
    if (this->num_hashes != other.num_hashes) {
        throw std::invalid_argument("Cannot merge Bloom filters with different num_hashes");
    }
    this->bits &= other.bits;
    if (this->server && other.server) {
        std::unordered_set<std::string> intersection;
        for (const auto& word : this->server->getWords()) {
            if (other.server->checkWord(word)) {
                intersection.insert(word);
            }
        }
        this->server->clearWords();
        for (const auto& word : intersection) {
            this->server->addWord(word);
        }
    }
}

template <std::size_t N>
BloomFilter<N>& BloomFilter<N>::operator|(const BloomFilter& other)
{
    if (this->num_hashes != other.num_hashes) {
        throw std::invalid_argument("Cannot merge Bloom filters with different num_hashes");
    }
    this->bits |= other.bits;
    if (this->server && other.server) {
        for (const auto& word : other.server->getWords()) {
            this->server->addWord(word);
        }
    }
    return *this;
}

template <std::size_t N>
bool BloomFilter<N>::operator()(const std::string& item) const
{
    return possiblyContains(item);
}

template <std::size_t N>
std::ostream& operator<<(std::ostream& os, const BloomFilter<N>& bloom_filter)
{
    os << bloom_filter.bits;
    return os;
}

template <std::size_t N>
std::istream& operator>>(std::istream& is, BloomFilter<N>& bloom_filter) {
    is >> bloom_filter.bits;
    return is;
}



