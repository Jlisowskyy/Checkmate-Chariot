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

#include "compilationConstants.h"

struct Option {
    enum class OptionType { check, spin, combo, button, string };
    static constexpr const char* OptionNames[] = { "check", "spin", "combo", "button", "string" };

    explicit Option(std::string&& name, const OptionType opT) : optionT(opT), optionName{std::move( name )}{}

    friend std::ostream& operator<<(std::ostream& out, const Option& opt) {
        out << "option name " << opt.optionName << " type " << OptionNames[static_cast<int>(opt.optionT)] << ' ';
        return opt._printDetails(out);;
    }

private:

    virtual std::ostream& _printDetails(std::ostream& out) const {
        return out << '\n';
    };

public:
    virtual ~Option() = default;

    const OptionType optionT;
    const std::string optionName;
};

template<Option::OptionType opT> struct OptionT;

template<>
struct OptionT<Option::OptionType::check>final : Option{
    OptionT(std::string&& name, const bool defVal) :
        Option(std::move(name), OptionType::check), defaultValue(defVal), actValue(defVal) {}

private:

    std::ostream& _printDetails(std::ostream& out) const final {
        return out << "default " << defaultValue << '\n';
    }

public:

    const bool defaultValue;
    bool actValue;
};

template<>
struct OptionT<Option::OptionType::spin>final : Option{
    OptionT(std::string&& name, const lli minV, const lli maxV, const lli defV) :
        Option(std::move(name),OptionType::spin), minVal(minV), maxVal(maxV), defaultVal(defV), actVal(defV) {}

private:

    std::ostream& _printDetails(std::ostream& out) const final {
        return out << "default " << defaultVal << " min " << minVal << " max " << maxVal << '\n';
    }


public:

    const lli minVal;
    const lli maxVal;
    const lli defaultVal;
    lli actVal;
};

template<>
struct OptionT<Option::OptionType::combo>final : Option{
    OptionT(std::string&& name, std::string&& defVal, std::vector<std::string>&& predefVal) :
        Option(std::move(name), OptionType::combo), defaultVal(defVal), predefinedVals(predefVal), actVal(defVal) {}

private:

    std::ostream& _printDetails(std::ostream& out) const final {
        out << "default " << defaultVal;
        for (const auto& var : predefinedVals) {
            out << " var " << var;
        }
        return out << '\n';
    }

public:

    const std::string defaultVal;
    const std::vector<std::string> predefinedVals;
    std::string actVal;
};

template<>
struct OptionT<Option::OptionType::button>final : Option{
    OptionT(std::string&& name, void (*action)()) :
        Option(std::move(name), OptionType::button), pressAction(action) {}

    void (* const pressAction) ();
};

template<>
struct OptionT<Option::OptionType::string>final : Option{
    OptionT(std::string&& name, std::string&& defV) :
        Option(std::move(name), OptionType::string), defaultVal(defV), actVal(defV) {}

private:

    std::ostream& _printDetails(std::ostream& out) const final {
        return out << "default " << defaultVal << '\n';
    }

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