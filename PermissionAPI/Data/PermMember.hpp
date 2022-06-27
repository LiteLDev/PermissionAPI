#pragma once
#include "Foundation.hpp"

class PermMembers : public std::vector<xuid_t> {

    using Base = std::vector<xuid_t>;

public:

    PermMembers()
        : Base() {
    }
    PermMembers(const Base& base)
        : Base(base) {
    }
    PermMembers(Base&& base)
        : Base(base) {
    }
    PermMembers(const PermMembers& other) = default;
    PermMembers(PermMembers&& other) = default;

    bool contains(const xuid_t& xuid) const {
        for (auto& member : *this) {
            if (member == xuid) {
                return true;
            }
        }
        return false;
    }

    size_t count(const xuid_t& xuid) const {
        size_t result = 0;
        for (auto& member : *this) {
            if (member == xuid) {
                result++;
            }
        }
        return result;
    }

    xuid_t& push_back(const xuid_t& xuid) {
        if (contains(xuid)) {
            throw std::out_of_range("Failed to add the member: the member already exists");
        }
        Base::push_back(xuid);
        return this->back();
    }

    template <typename ... Args>
    xuid_t& emplace_back(Args&& ... args) {
        return this->push_back(xuid_t(std::forward<Args>(args)...));
    }

    PermMembers& operator=(const Base& other) {
        return (PermMembers&)(((Base&)*this) = other);
    }

};