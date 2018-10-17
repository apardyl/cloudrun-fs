#ifndef CLOUDRUN_FS_LOCKMAP_H
#define CLOUDRUN_FS_LOCKMAP_H

#include <unordered_map>
#include <memory>
#include <condition_variable>

template<typename T>
class LockMap {
private:
    std::unordered_map<T, std::shared_ptr<std::condition_variable>> lock_map;
    std::mutex global_lock;
public:
    void lock(const T &object);

    void unlock(const T &object);
};

template<typename T>
void LockMap<T>::lock(const T &object) {
    std::unique_lock<std::mutex> g(global_lock);
    if (lock_map.count(object) > 0) {
        do {
            std::shared_ptr<std::condition_variable> ptr_cv = lock_map[object];
            ptr_cv->wait(g);
        } while (lock_map.count(object) > 0);
    }
    lock_map[object] = std::make_shared<std::condition_variable>();
}

template<typename T>
void LockMap<T>::unlock(const T &object) {
    std::lock_guard<std::mutex> g(global_lock);
    auto iter = lock_map.find(object);
    if (iter != lock_map.end()) {
        (*iter).second->notify_all();
        lock_map.erase(iter);
    }
}

template<typename T>
class LockMapGuard {
private:
    LockMap<T> &m;
    const T &obj;
public:
    LockMapGuard(LockMap<T> &lockMap, const T &object) : m(lockMap), obj(object) {
        m.lock(obj);
    }

    ~LockMapGuard() {
        m.unlock(obj);
    }
};


#endif //CLOUDRUN_FS_LOCKMAP_H
