#define NOMINMAX
#include <EventAPI.h>
#include "../PermissionAPI/PermissionAPI.h"
#include "pch.h"

using namespace script;

#if defined(SCRIPT_JS)
::Logger slogger("PermAPI-JS");
#elif defined(SCRIPT_LUA)
::Logger slogger("PermAPI-LUA");
#endif

PermissionAPI api(true);

bool CheckIsFloat(const Local<Value>& num) {
    try {
        return fabs(num.asNumber().toDouble() - num.asNumber().toInt64()) >= 1e-15;
    } catch (...) {
        return false;
    }
}

std::string ValueKindToString(const ValueKind& kind) {
    switch (kind) {
    case ValueKind::kString:
        return "string";
    case ValueKind::kNumber:
        return "number";
    case ValueKind::kBoolean:
        return "boolean";
    case ValueKind::kNull:
        return "null";
    case ValueKind::kObject:
        return "object";
    case ValueKind::kArray:
        return "array";
    case ValueKind::kFunction:
        return "function";
    case ValueKind::kByteBuffer:
        return "bytebuffer";
    default:
        return "unknown";
    }
}

Local<Value> JsonToValue(nlohmann::json j);

Local<Value> BigInteger_Helper(nlohmann::json& i)
{
    if (i.is_number_integer())
    {
        if (i.is_number_unsigned())
        {
            auto ui = i.get<uint64_t>();
            if (ui <= LLONG_MAX) return Number::newNumber((int64_t)ui);
            return Number::newNumber((double)ui);
        }
        return Number::newNumber(i.get<int64_t>());
    }
    return JsonToValue(i);
}

void JsonToValue_Helper(Local<Array> &res, nlohmann::json &j);

void JsonToValue_Helper(Local<Object> &res, const string &key, nlohmann::json &j)
{
    switch (j.type())
    {
    case nlohmann::json::value_t::string:
        res.set(key, String::newString(j.get<string>()));
        break;
    case nlohmann::json::value_t::number_integer:
    case nlohmann::json::value_t::number_unsigned:
        res.set(key, BigInteger_Helper(j));
        break;
    case nlohmann::json::value_t::number_float:
        res.set(key, Number::newNumber(j.get<double>()));
        break;
    case nlohmann::json::value_t::boolean:
        res.set(key, Boolean::newBoolean(j.get<bool>()));
        break;
    case nlohmann::json::value_t::null:
        res.set(key, Local<Value>());
        break;
    case nlohmann::json::value_t::array:
    {
        Local<Array> arrToAdd = Array::newArray();
        for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
            JsonToValue_Helper(arrToAdd, *it);
        res.set(key, arrToAdd);
        break;
    }
    case nlohmann::json::value_t::object:
    {
        Local<Object> objToAdd = Object::newObject();
        for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
            JsonToValue_Helper(objToAdd, it.key(), it.value());
        res.set(key, objToAdd);
        break;
    }
    default:
        res.set(key, Local<Value>());
        break;
    }
}

void JsonToValue_Helper(Local<Array> &res, nlohmann::json &j)
{
    switch (j.type())
    {
    case nlohmann::json::value_t::string:
        res.add(String::newString(j.get<string>()));
        break;
    case nlohmann::json::value_t::number_integer:
    case nlohmann::json::value_t::number_unsigned:
        res.add(BigInteger_Helper(j));
        break;
    case nlohmann::json::value_t::number_float:
        res.add(Number::newNumber(j.get<double>()));
        break;
    case nlohmann::json::value_t::boolean:
        res.add(Boolean::newBoolean(j.get<bool>()));
        break;
    case nlohmann::json::value_t::null:
        res.add(Local<Value>());
        break;
    case nlohmann::json::value_t::array:
    {
        Local<Array> arrToAdd = Array::newArray();
        for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
            JsonToValue_Helper(arrToAdd, *it);
        res.add(arrToAdd);
        break;
    }
    case nlohmann::json::value_t::object:
    {
        Local<Object> objToAdd = Object::newObject();
        for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
            JsonToValue_Helper(objToAdd, it.key(), it.value());
        res.add(objToAdd);
        break;
    }
    default:
        res.add(Local<Value>());
        break;
    }
}

Local<Value> JsonToValue(nlohmann::json j)
{
    Local<Value> res;

    switch (j.type())
    {
    case nlohmann::json::value_t::string:
        res = String::newString(j.get<string>());
        break;
    case nlohmann::json::value_t::number_integer:
    case nlohmann::json::value_t::number_unsigned:
        res = BigInteger_Helper(j);
        break;
    case nlohmann::json::value_t::number_float:
        res = Number::newNumber(j.get<double>());
        break;
    case nlohmann::json::value_t::boolean:
        res = Boolean::newBoolean(j.get<bool>());
        break;
    case nlohmann::json::value_t::null:
        res = Local<Value>();
        break;
    case nlohmann::json::value_t::array:
    {
        Local<Array> resArr = Array::newArray();
        for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
            JsonToValue_Helper(resArr, *it);
        res = resArr;
        break;
    }
    case nlohmann::json::value_t::object:
    {
        Local<Object> resObj = Object::newObject();
        for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
            JsonToValue_Helper(resObj, it.key(), it.value());
        res = resObj;
        break;
    }
    default:
        res = Local<Value>();
        break;
    }
    
    return res;
}

void ValueToJson_Obj_Helper(nlohmann::json& res, const Local<Object>& v);

void ValueToJson_Arr_Helper(nlohmann::json& res, const Local<Array>& v) {
    for (int i = 0; i < v.size(); ++i) {
        switch (v.get(i).getKind()) {
            case ValueKind::kString:
                res.push_back(v.get(i).asString().toString());
                break;
            case ValueKind::kNumber:
                if (CheckIsFloat(v.get(i)))
                    res.push_back(v.get(i).asNumber().toDouble());
                else
                    res.push_back(v.get(i).asNumber().toInt64());
                break;
            case ValueKind::kBoolean:
                res.push_back(v.get(i).asBoolean().value());
                break;
            case ValueKind::kNull:
                res.push_back(nullptr);
                break;
            case ValueKind::kArray: {
                Local<Array> arrToAdd = v.get(i).asArray();
                if (arrToAdd.size() == 0)
                    res.push_back(nlohmann::json::array());
                else {
                    nlohmann::json arrJson = nlohmann::json::array();
                    ValueToJson_Arr_Helper(arrJson, arrToAdd);
                    res.push_back(arrJson);
                }
                break;
            }
            case ValueKind::kObject: {
                Local<Object> objToAdd = v.get(i).asObject();
                if (objToAdd.getKeyNames().empty())
                    res.push_back(nlohmann::json::object());
                else {
                    nlohmann::json objJson = nlohmann::json::object();
                    ValueToJson_Obj_Helper(objJson, objToAdd);
                    res.push_back(objJson);
                }
                break;
            }
            default:
                res.push_back(nullptr);
                break;
        }
    }
}

void ValueToJson_Obj_Helper(nlohmann::json& res, const Local<Object>& v) {
    auto keys = v.getKeyNames();
    for (auto& key : keys) {
        switch (v.get(key).getKind()) {
            case ValueKind::kString:
                res.push_back({key, v.get(key).asString().toString()});
                break;
            case ValueKind::kNumber:
                if (CheckIsFloat(v.get(key)))
                    res.push_back({key, v.get(key).asNumber().toDouble()});
                else
                    res.push_back({key, v.get(key).asNumber().toInt64()});
                break;
            case ValueKind::kBoolean:
                res.push_back({key, v.get(key).asBoolean().value()});
                break;
            case ValueKind::kNull:
                res.push_back({key, nullptr});
                break;
            case ValueKind::kArray: {
                Local<Array> arrToAdd = v.get(key).asArray();
                if (arrToAdd.size() == 0)
                    res.push_back({key, nlohmann::json::array()});
                else {
                    nlohmann::json arrJson = nlohmann::json::array();
                    ValueToJson_Arr_Helper(arrJson, arrToAdd);
                    res.push_back({key, arrJson});
                }
                break;
            }
            case ValueKind::kObject: {
                Local<Object> objToAdd = v.get(key).asObject();
                if (objToAdd.getKeyNames().empty())
                    res.push_back({key, nlohmann::json::object()});
                else {
                    nlohmann::json objJson = nlohmann::json::object();
                    ValueToJson_Obj_Helper(objJson, objToAdd);
                    res.push_back({key, objJson});
                }
                break;
            }
            default:
                res.push_back({key, nullptr});
                break;
        }
    }
}

Local<Value> PermAbilityToObject(const PermAbility& ab) {
    Local<Object> obj = Object::newObject();
    obj.set("name", String::newString(ab.name));
    obj.set("enabled", Boolean::newBoolean(ab.enabled));
    if (!ab.extra.is_object()) {
        obj.set("extra", Local<Value>());
    } else {
        nlohmann::json extra = ab.extra;
        obj.set("extra", JsonToValue(extra));
    }
    return obj;
}

#define toStr() asString().toString()

#define CHECK_ARGS_COUNT(count) \
    if (args.size() != count) { \
        throw Exception(fmt::format("Invalid arguments count: {} instead of {}", args.size(), count)); \
    }

#define CHECK_ARG_TYPE(index, type) \
    if (args[index].getKind() != ValueKind::type) { \
        throw Exception(fmt::format("Wrong type of arguments[{}]: expect {}", index, ValueKindToString(ValueKind::type))); \
    }

#define CHECK_VAL_TYPE(val, type) \
    if (val.getKind() != ValueKind::type) { \
        throw Exception(fmt::format("Wrong type of value: expect {}", ValueKindToString(ValueKind::type))); \
    }

#define CATCH_AND_THROW \
    catch (const Exception& e) { \
        throw e; \
    } \
    catch (const std::exception& e) { \
        throw Exception(e.what()); \
    } \
    catch (...) { \
        throw Exception("Unknown exception in " __FUNCTION__); \
    }

class PermGroupClass : public ScriptClass {

    std::weak_ptr<PermGroup> group;

    std::shared_ptr<PermGroup> lock() {
        if (group.expired()) throw Exception("Group pointer expired!");
        return group.lock();
    }

public:

    PermGroupClass(const Local<Object>& scriptObj, std::weak_ptr<PermGroup> group)
        : ScriptClass(scriptObj)
        , group(group) {
    }

    PermGroupClass(std::weak_ptr<PermGroup> group)
        : ScriptClass(ScriptClass::ConstructFromCpp<PermGroupClass>())
        , group(group) {
    }

    static PermGroupClass* constructor(const Arguments& args) {
        std::string name;
        std::string displayName;
        if (args.size() == 1) {
            CHECK_ARG_TYPE(0, kString);
            displayName = name = args[0].toStr();
        } else if (args.size() == 2) {
            CHECK_ARG_TYPE(0, kString);
            CHECK_ARG_TYPE(1, kString);
            name = args[0].toStr();
            displayName = args[1].toStr();
        } else {
            CHECK_ARGS_COUNT(2);
        }

        try {
            return new PermGroupClass(args.thiz(), api.createGroup(name, displayName));
        }
        CATCH_AND_THROW;
        return nullptr;
    }

    Local<Value> getName() {
        return String::newString(lock()->name);
    }

    Local<Value> getDisplayName() {
        return String::newString(lock()->displayName);
    }

    Local<Value> getPriority() {
        return Number::newNumber(lock()->priority);
    }

    Local<Value> getMembers() {
        auto members = Array::newArray();
        if (lock()->getType() == PermGroup::Type::Everyone) return members;
        for (auto& member : lock()->members) {
            members.add(String::newString(member));
        }
        return members;
    }

    Local<Value> getAbilities() {
        auto abilities = Array::newArray();
        for (auto& ability : lock()->abilities) {
            abilities.add(PermAbilityToObject(ability));
        }
        return abilities;
    }

    void setName(const Local<Value>& val) {
        CHECK_VAL_TYPE(val, kString);

        try {
            lock()->name = val.toStr();
        }
        CATCH_AND_THROW;
    }

    void setDisplayName(const Local<Value>& val) {
        CHECK_VAL_TYPE(val, kString);

        try {
            lock()->displayName = val.toStr();
        }
        CATCH_AND_THROW;
    }

    void setPriority(const Local<Value>& val) {
        CHECK_VAL_TYPE(val, kNumber);

        try {
            lock()->priority = val.asNumber().toInt32();
        }
        CATCH_AND_THROW;
    }

    void setMembers(const Local<Value>& val) {
        CHECK_VAL_TYPE(val, kArray);

        try {
            lock()->members.clear();
            auto arr = val.asArray();
            for (size_t i = 0; i < arr.size(); i++) {
                CHECK_VAL_TYPE(arr.get(i), kString);
                lock()->members.push_back(arr.get(i).toStr());
            }
        }
        CATCH_AND_THROW;
    }

    void setAbilities(const Local<Value>& val) {
        CHECK_VAL_TYPE(val, kArray);

        try {
            lock()->abilities.clear();
            auto arr = val.asArray();
            for (size_t i = 0; i < arr.size(); i++) {
                CHECK_VAL_TYPE(arr.get(i), kObject);
                auto obj = arr.get(i).asObject();
                if (!obj.has("name") || !obj.has("enabled"))
                    throw Exception("Invalid PermGroup instance in array");
                PermAbility ab;
                ab.name = obj.get("name").toStr();
                ab.enabled = obj.get("enabled").asBoolean().value();
                ValueToJson_Obj_Helper(ab.extra, obj);
                ab.extra.erase("name");
                ab.extra.erase("enabled");
                lock()->abilities.push_back(ab);
            }
        }
        CATCH_AND_THROW;
    }

    Local<Value> hasMember(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            return Boolean::newBoolean(lock()->hasMember(args[0].toStr()));
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    Local<Value> addMember(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            lock()->addMember(args[0].toStr());
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    Local<Value> removeMember(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            lock()->removeMember(args[0].toStr());
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    Local<Value> hasAbility(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            return Boolean::newBoolean(lock()->hasAbility(args[0].toStr()));
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    Local<Value> setAbility(const Arguments& args) {
        if (args.size() == 2) {
            CHECK_ARG_TYPE(0, kString);
            CHECK_ARG_TYPE(1, kBoolean);
        } else if (args.size() == 3) {
            CHECK_ARG_TYPE(0, kString);
            CHECK_ARG_TYPE(1, kBoolean);
            CHECK_ARG_TYPE(2, kObject);
        } else {
            CHECK_ARGS_COUNT(2);
        }

        try {
            nlohmann::json extra{};
            auto name = args[0].toStr();
            auto enabled = args[1].asBoolean().value();
            if (args.size() == 3) {
                ValueToJson_Obj_Helper(extra, args[2].asObject());
            }
            lock()->setAbility(name, enabled, extra);
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    Local<Value> removeAbility(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            lock()->removeAbility(args[0].toStr());
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    Local<Value> abilityDefined(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            return Boolean::newBoolean(lock()->abilityDefined(args[0].toStr()));
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }
};
ClassDefine<PermGroupClass> PermGroupClassBuilder =
    defineClass<PermGroupClass>("PermGroup")
        .constructor(&PermGroupClass::constructor)
        .instanceFunction("hasMember", &PermGroupClass::hasMember)
        .instanceFunction("addMember", &PermGroupClass::addMember)
        .instanceFunction("removeMember", &PermGroupClass::removeMember)
        .instanceFunction("hasAbility", &PermGroupClass::hasAbility)
        .instanceFunction("setAbility", &PermGroupClass::setAbility)
        .instanceFunction("removeAbility", &PermGroupClass::removeAbility)
        .instanceFunction("abilityDefined", &PermGroupClass::abilityDefined)
        .instanceProperty("name", &PermGroupClass::getName, &PermGroupClass::setName)
        .instanceProperty("displayName", &PermGroupClass::getDisplayName, &PermGroupClass::setDisplayName)
        .instanceProperty("priority", &PermGroupClass::getPriority, &PermGroupClass::setPriority)
        .instanceProperty("members", &PermGroupClass::getMembers, &PermGroupClass::setMembers)
        .instanceProperty("abilities", &PermGroupClass::getAbilities, &PermGroupClass::setAbilities)
        .build();

class PermissionClass : public ScriptClass {

public:

    static Local<Value> createGroup(const Arguments& args) {
        try {
            auto res = PermGroupClass::constructor(args);
            if (res) return res->getScriptObject();
            return Local<Value>();
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> groupExists(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            return Boolean::newBoolean(api.groupExists(args[0].toStr()));
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> getGroup(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            return PermGroupClass(args.thiz(), api.getGroup(args[0].toStr())).getScriptObject();
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> getOrCreateGroup(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            return PermGroupClass(args.thiz(), api.getOrCreateGroup(args[0].toStr())).getScriptObject();
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> registerAbility(const Arguments& args) {
        CHECK_ARGS_COUNT(2);
        CHECK_ARG_TYPE(0, kString);
        CHECK_ARG_TYPE(1, kString);

        try {
            api.registerAbility(args[0].toStr(), args[1].toStr());
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> deleteAbility(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            api.deleteAbility(args[0].toStr());
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> abilityExists(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            return Boolean::newBoolean(api.abilityExists(args[0].toStr()));
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> hasAbility(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);
        CHECK_ARG_TYPE(1, kString);

        try {
            return Boolean::newBoolean(api.hasAbility(args[0].toStr(), args[1].toStr()));
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> isMemberOf(const Arguments& args) {
        CHECK_ARGS_COUNT(2);
        CHECK_ARG_TYPE(0, kString);
        CHECK_ARG_TYPE(1, kString);

        try {
            return Boolean::newBoolean(api.isMemberOf(args[0].toStr(), args[1].toStr()));
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> getPlayerGroups(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            auto groups = api.getPlayerGroups(args[0].toStr());
            auto res = Array::newArray();
            for (auto& group : groups) {
                res.add(PermGroupClass(group).getScriptObject());
            }
            return res;
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> getPlayerAbilities(const Arguments& args) {
        CHECK_ARGS_COUNT(1);
        CHECK_ARG_TYPE(0, kString);

        try {
            auto abilities = api.getPlayerAbilities(args[0].toStr());
            auto res = Array::newArray();
            for (auto& ability : abilities) {
                res.add(PermAbilityToObject(ability));
            }
            return res;
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

    static Local<Value> saveData(const Arguments& args) {
        CHECK_ARGS_COUNT(0);

        try {
            api.saveData();
        }
        CATCH_AND_THROW;
        return Local<Value>();
    }

};
ClassDefine<void> PermScriptAPIBuilder =
    defineClass("perm")
        .function("createGroup", &PermissionClass::createGroup)
        .function("groupExists", &PermissionClass::groupExists)
        .function("getGroup", &PermissionClass::getGroup)
        .function("getOrCreateGroup", &PermissionClass::getOrCreateGroup)
        .function("registerAbility", &PermissionClass::registerAbility)
        .function("deleteAbility", &PermissionClass::deleteAbility)
        .function("abilityExists", &PermissionClass::abilityExists)
        .function("hasAbility", &PermissionClass::hasAbility)
        .function("isMemberOf", &PermissionClass::isMemberOf)
        .function("getPlayerGroups", &PermissionClass::getPlayerGroups)
        .function("getPlayerAbilities", &PermissionClass::getPlayerAbilities)
        .function("saveData", &PermissionClass::saveData)
        .build();

void entry() {
#if defined(SCRIPT_JS)
    slogger.info("PermissionAPI v{} for JavaScript loaded! Author: Jasonzyt", PERM_VER.toString(true));
#elif defined(SCRIPT_LUA)
    slogger.info("PermissionAPI v{} for Lua loaded! Author: Jasonzyt", PERM_VER.toString(true));
#endif
    try {
        api.init();
    }
    catch (const std::exception& e) {
        slogger.error("Error when initializing the APIs: {}", e.what());
    }
    Event::RegScriptClassEvent::subscribe([&](const Event::RegScriptClassEvent& ev) {
        ev.mEngine->registerNativeClass(PermGroupClassBuilder);
        ev.mEngine->registerNativeClass(PermScriptAPIBuilder);
        return true;
    });
}