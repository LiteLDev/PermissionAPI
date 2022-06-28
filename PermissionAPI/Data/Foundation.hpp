#pragma once
#include <Global.h>

template <typename T>
class PermVector : public std::vector<T> {

    using Base = std::vector<T>;

public:

    PermVector()
        : Base() {
    }
    PermVector(const Base& base)
        : Base(base) {
    }
    PermVector(Base&& base)
        : Base(base) {
    }
    PermVector(const PermVector<T>& other) = default;
    PermVector(PermVector<T>&& other) = default;

    bool contains(const T& xuid) const {
        for (auto& member : *this) {
            if (member == xuid) {
                return true;
            }
        }
        return false;
    }

    size_t count(const T& xuid) const {
        size_t result = 0;
        for (auto& member : *this) {
            if (member == xuid) {
                result++;
            }
        }
        return result;
    }

    T& push_back(const T& xuid) {
        if (contains(xuid)) {
            throw std::out_of_range("Failed to add the element: the element already exists");
        }
        Base::push_back(xuid);
        return this->back();
    }

    template <typename ... Args>
    T& emplace_back(Args&& ... args) {
        return this->push_back(T(std::forward<Args>(args)...));
    }

    PermVector<T>& operator=(const Base& other) {
        return (PermVector<T>&)(((Base&)*this) = other);
    }

};

template <typename T>
class PermContainer : public std::vector<T> {

using Base = std::vector<T>;

public:

    PermContainer()
        : Base() {
    }
    PermContainer(const Base& base)
        : Base(base) {
    }
    PermContainer(Base&& base)
        : Base(base) {
    }
    PermContainer(const PermContainer<T>& other) = default;
    PermContainer(PermContainer<T>&& other) = default;

    bool contains(const std::string& name) const {
        for (auto& group : *this) {
            if (group.name == name) {
                return true;
            }
        }
        return false;
    }

    Base::iterator find(const std::string& name) {
        for (auto& it = this->begin(); it != this->end(); it++) {
            if (group.name == name) {
                return it;
            }
        }
        return this->end();
    }

    Base::const_iterator find(const std::string& name) const {
        for (auto& it = this->begin(); it != this->end(); it++) {
            if (group.name == name) {
                return it;
            }
        }
        return this->end();
    }

    size_t count(const std::string& name) const {
        size_t result = 0;
        for (auto& group : *this) {
            if (group.name == name) {
                result++;
            }
        }
        return result;
    }

    template <typename ... Args>
   T& getOrCreate(Args&& ... args) {
        for (auto& group : *this) {
            if (group.name == name) {
                return group;
            }
        }
        auto& group = this->emplace_back(args...);
        return group;
    }

    T& at(const std::string& name) {
        for (auto& group : *this) {
            if (group.name == name) {
                return group;
            }
        }
        throw std::out_of_range("Failed to get the element: the element does not exist");
    }
    const T& at(const std::string& name) const {
        for (auto& group : *this) {
            if (group.name == name) {
                return group;
            }
        }
        throw std::out_of_range("Failed to get the element: the element does not exist");
    }

    T& push_back(const T& group) {
        if (contains(group.name)) {
            throw std::out_of_range("Failed to add the element: the element with the same name already exists");
        }
        Base::push_back(group);
        return this->back();
    }

    template <typename ... Args>
    T& emplace_back(Args&& ... args) {
        Base::emplace_back(std::forward<Args>(args)...);
        if (count(this->back().name) > 1) {
            this->pop_back();
            throw std::out_of_range("Failed to add the element: the element with the same name already exists");
        }
        return this->back();
    }

    void remove(const std::string& name) {
        for (auto it = this->begin(); it != this->end(); it++) {
            if (it->name == name) {
                this->erase(it);
                return;
            }
        }
        throw std::out_of_range("Failed to remove the element: the target element does not exist");
    }

    T& operator[](const std::string& name) {
        return this->getOrCreate(name);
    }

    PermContainer<T>& operator=(const PermContainer<T>& other) = default;
    PermContainer<T>& operator=(PermContainer<T>&& other) = default;
    PermContainer<T> & operator=(const Base& other) {
        return (PermContainer<T>&)(((Base&)*this) = other);
    }

};