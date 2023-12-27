//
// Created by Jlisowskyy on 12/26/23.
//

#ifndef UCIOPTIONS_H
#define UCIOPTIONS_H

#include <utility>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cerrno>

#include "compilationConstants.h"

class Engine;

// ------------------------------
// Parent class
// ------------------------------

struct Option {
    enum class OptionType { check, spin, combo, button, string };

    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------

    explicit Option(std::string&& name, const OptionType opT) :
        optionT(opT), optionName{std::move( name )}{}

    virtual ~Option() = default;

    // ------------------------------
    // Type interaction
    // ------------------------------

    friend std::ostream& operator<<(std::ostream& out, const Option& opt) {
        out << "option name " << opt.optionName << " type " << OptionNames[static_cast<int>(opt.optionT)] << ' ';
        return opt._printDetails(out);
    }

    bool virtual TryChangeValue(std::string& nArg, Engine& eng) {
        return false;
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:

    virtual std::ostream& _printDetails(std::ostream& out) const {
        return out << '\n';
    };

    // ------------------------------
    // Public fields
    // ------------------------------
public:

    static constexpr const char* OptionNames[] = { "check", "spin", "combo", "button", "string" };

    const OptionType optionT;
    const std::string optionName;
};

// ------------------------------
// Derived classes
// ------------------------------

template<Option::OptionType opT> struct OptionT;

template<>
struct OptionT<Option::OptionType::check>final : Option{
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------

    OptionT(std::string&& name, void (* const react)(Engine&, bool), const bool defVal) :
        Option(std::move(name), OptionType::check), changeReaction(react), defaultValue(defVal) {}

    // -------------------------------
    // Virtual methods overrides
    // -------------------------------

private:

    std::ostream& _printDetails(std::ostream& out) const override {
        return out << "default " << defaultValue << '\n';
    }

public:

    bool TryChangeValue(std::string& nArg, Engine& eng) override {
        if (nArg == "true") {
            changeReaction(eng, true);
            return true;
        }

        if (nArg == "false") {
            changeReaction(eng, false);
            return true;
        }

        return false;
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:
    void (* const changeReaction)(Engine&, bool);
public:
    const bool defaultValue;
};

template<>
struct OptionT<Option::OptionType::spin>final : Option{
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------

    OptionT(std::string&& name, void (* const react)(Engine&, lli), const lli minV, const lli maxV, const lli defV) :
        Option(std::move(name),OptionType::spin), changeReaction(react), minVal(minV), maxVal(maxV), defaultVal(defV) {}

    // -------------------------------
    // Virtual methods overrides
    // -------------------------------

private:

    std::ostream& _printDetails(std::ostream& out) const override {
        return out << "default " << defaultVal << " min " << minVal << " max " << maxVal << '\n';
    }

public:

    bool TryChangeValue(std::string& nArg, Engine& eng) override {
        errno = 0;

        const lli result = strtoll(nArg.c_str(), nullptr, 10);
        if (errno != 0 || result > maxVal || result < minVal)
            return false;

        changeReaction(eng, result);
        return true;
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:
    void (* const changeReaction)(Engine& eng, lli);
public:
    const lli minVal;
    const lli maxVal;
    const lli defaultVal;
};

template<>
struct OptionT<Option::OptionType::combo>final : Option{
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------

    OptionT(std::string&& name, void (* const react)(Engine&, std::string&), std::string&& defVal, std::vector<std::string>&& predefVal) :
        Option(std::move(name), OptionType::combo), changeReaction(react), defaultVal(defVal), predefinedVals(predefVal) {}

    // -------------------------------
    // Virtual methods overrides
    // -------------------------------

private:

    std::ostream& _printDetails(std::ostream& out) const final {
        out << "default " << defaultVal;
        for (const auto& var : predefinedVals) {
            out << " var " << var;
        }
        return out << '\n';
    }

public:

    bool TryChangeValue(std::string& nArg, Engine& eng) override {
        if (!_isValidOption(nArg))
            return false;

        changeReaction(eng, nArg);
        return true;
    }

    // ------------------------------
    // private methods
    // ------------------------------
private:
     [[nodiscard]] bool _isValidOption(const std::string& str) const {
         for (auto& value: predefinedVals) {
             if (str == value) return true;
         }

         return false;
     }

    // ------------------------------
    // Class fields
    // ------------------------------

    void (* const changeReaction)(Engine&, std::string&);
public:
    const std::string defaultVal;
    const std::vector<std::string> predefinedVals;
};

template<>
struct OptionT<Option::OptionType::button>final : Option{
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------

    OptionT(std::string&& name, void (* const react)(Engine&)) :
        Option(std::move(name), OptionType::button), changeReaction(react) {}

private:
    void (* const changeReaction)(Engine&);

    // -------------------------------
    // Virtual methods overrides
    // -------------------------------
public:

    bool TryChangeValue(std::string& nArg, Engine& eng) override {
        changeReaction(eng);
        return true;
    }
};

template<>
struct OptionT<Option::OptionType::string>final : Option{
    // --------------------------------------
    // Type creation and initialization
    // --------------------------------------

    OptionT(std::string&& name, void (* const react)(Engine& eng, std::string&), std::string&& defV) :
        Option(std::move(name), OptionType::string), changeReaction(react), defaultVal(defV) {}

    // -------------------------------
    // Virtual methods overrides
    // -------------------------------

private:

    std::ostream& _printDetails(std::ostream& out) const final {
        return out << "default " << defaultVal << '\n';
    }

public:

    bool TryChangeValue(std::string& nArg, Engine& eng) override {
        changeReaction(eng, nArg);
        return true;
    }

    // ------------------------------
    // Class fields
    // ------------------------------
private:
    void (* const changeReaction)(Engine& eng, std::string&);
public:
    const std::string defaultVal;
    std::string actVal;
};

struct EngineInfo {
    std::string author;
    std::string name;

    std::map<std::string, Option> options;
};

#endif //UCIOPTIONS_H