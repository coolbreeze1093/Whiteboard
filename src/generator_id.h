#include <random>
#include <mutex>
#include <set>

class IdGenerator final
{
public:
    uint32_t generateId()
    {
        static std::mt19937 rng{std::random_device{}()};
        static std::uniform_int_distribution<uint32_t> dist(1, 0xFFFFFFFEu);

        uint32_t id;
        while (true)
        {
            std::size_t is_free = 0;
            id = dist(rng);
            {
                std::lock_guard<std::mutex> lock(sessions_mutex_);
                is_free = sessions_.count(id);
            }

            if (is_free == 0)
            {
                break;
            }
        }
        sessions_.insert(id);
        return id;
    }
    void releaseId(uint32_t id)
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_.erase(id);
    }

private:
    std::mutex sessions_mutex_;
    std::set<uint32_t> sessions_;
};
