#pragma once
#include <third-party/Nlohmann/json.hpp>
#include "Foundation.hpp"

struct PermAbility {
    std::string name;
    bool enabled;
    nlohmann::json extra;

    static constexpr const std::string_view abilityNameInvalidChars = " \t\n\r\f\v";
    /**
     * @brief Get the namespace of the ability.
     * 
     * @return std::string  The namespace of the ability.
     */
    inline std::string namespc() const {
        return this->name.substr(0, this->name.find_first_of(':'));
    }

    static bool isValidAbilityName(const std::string& name) {
        return name.find_first_of(PermAbility::abilityNameInvalidChars.data()) == std::string::npos && name.find_first_of(':') != std::string::npos;
    }
};

struct PermAbilityInfo {
    std::string name;
    std::string desc;
};

using PermAbilities = PermContainer<PermAbility>;
using PermAbilitiesInfo = PermContainer<PermAbilityInfo>;