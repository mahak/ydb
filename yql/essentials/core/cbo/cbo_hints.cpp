#include "cbo_optimizer_new.h"

#include <util/string/join.h>
#include <util/string/printf.h>
#include <library/cpp/iterator/zip.h>

using namespace NYql;

TString ToLower(TString s) {
    for (char& c: s) {
        c = std::tolower(c);
    }
    return s;
}

class TOptimizerHintsParser {
public:
    TOptimizerHintsParser(const TString& text)
        : Pos_(-1)
        , Size_(static_cast<i32>(text.size()) - 1)
        , Text_(text)
    {}

    TOptimizerHints Parse() {
        Start();
        return Hints_;
    }

private:
    void Start() {
        while (Pos_ < Size_) {
            auto hintType = Keyword({"JoinOrder", "Leading", "JoinType", "Rows", "Bytes"});
            if (hintType == "JoinOrder" || hintType == "Leading") {
                JoinOrder(hintType == "Leading");
            } else if (hintType == "JoinType") {
                JoinType();
            } else if (hintType == "Rows") {
                CardinalityOrBytes(true);
            } else if (hintType == "Bytes") {
                CardinalityOrBytes(false);
            } else {
                ParseError(Sprintf("Undefined hints type: %s", hintType.c_str()), Pos_ - hintType.size());
            }

            SkipWhiteSpaces();
        }
    }

    TVector<TString> CollectLabels() {
        TVector<TString> labels;
        while (auto maybeTerm = MaybeLabel()) {
            labels.push_back(maybeTerm.value());
        }
        return labels;
    }

    void JoinType() {
        i32 beginPos = Pos_ + 1;

        Keyword({"("});

        i32 labelsBeginPos = Pos_ + 1;
        TVector<TString> labels = CollectLabels();
        if (labels.size() <= 1) {
            ParseError(Sprintf("Bad labels for JoinType hint: %s, example of the format: JoinType(t1 t2 Shuffle)", JoinSeq(", ", labels).c_str()), labelsBeginPos);
        }
        TString reqJoinAlgoStr = std::move(labels.back());
        labels.pop_back();

        Keyword({")"});

        TVector<EJoinAlgoType> joinAlgos = {EJoinAlgoType::GraceJoin, EJoinAlgoType::LookupJoin, EJoinAlgoType::MapJoin};
        TVector<TString> joinAlgosStr = {"shuffle", "lookup", "broadcast"};

        for (const auto& [JoinType, joinAlgoStr]: Zip(joinAlgos, joinAlgosStr)) {
            if (ToLower(reqJoinAlgoStr) == joinAlgoStr) {
                Hints_.JoinAlgoHints->PushBack(std::move(labels), JoinType, "JoinType" + Text_.substr(beginPos, Pos_ - beginPos + 1));
                return;
            }
        }

        ParseError(Sprintf("Unknown JoinType: '%s', supported algos: [%s]", reqJoinAlgoStr.c_str(), JoinSeq(", ", joinAlgosStr).c_str()), Pos_ - reqJoinAlgoStr.size());
        Y_UNREACHABLE();
    }

    void JoinOrder(bool leading /* is keyword "Leading" or "JoinOrder" */) {
        i32 beginPos = Pos_ + 1;

        Keyword({"("});
        auto joinOrderHintTree = JoinOrderLabels();
        Keyword({")"});

        Hints_.JoinOrderHints->PushBack(
            std::move(joinOrderHintTree),
            leading? "Leading" : "JoinOrder" + Text_.substr(beginPos, Pos_ - beginPos + 1)
        );
    }

    std::shared_ptr<TJoinOrderHints::ITreeNode> JoinOrderLabels() {
        auto lhs = JoinOrderLabel();
        auto rhs = JoinOrderLabel();
        return std::make_shared<TJoinOrderHints::TJoinNode>(std::move(lhs), std::move(rhs));
    }

    std::shared_ptr<TJoinOrderHints::ITreeNode> JoinOrderLabel() {
        if (auto maybeLabel = MaybeLabel()) {
            return std::make_shared<TJoinOrderHints::TRelationNode>(std::move(maybeLabel.value()));
        } else if (auto maybeBracket = MaybeKeyword({"("})) {
            auto join = JoinOrderLabels();
            Keyword({")"});
            return join;
        }

        ParseError(Sprintf("JoinOrder args must be either a relation, either a join, example of the format: JoinOrder(t1 (t2 t3))"), Pos_);
        Y_UNREACHABLE();
    }

    void CardinalityOrBytes(bool isRows) {
        i32 beginPos = Pos_ + 1;

        Keyword({"("});

        TVector<TString> labels = CollectLabels();
        auto signStr = Keyword({"+", "-", "/", "*", "#"});
        char sign = signStr[0];
        auto value = Number();
        Keyword({")"});

        TCardinalityHints::ECardOperation op;
        switch (sign) {
            case '+': { op = TCardinalityHints::ECardOperation::Add; break; }
            case '-': { op = TCardinalityHints::ECardOperation::Subtract; break; }
            case '/': { op = TCardinalityHints::ECardOperation::Divide; break; }
            case '*': { op = TCardinalityHints::ECardOperation::Multiply; break; }
            case '#': { op = TCardinalityHints::ECardOperation::Replace; break; }
            default: {ParseError(Sprintf("Unknown operation: '%c'", sign), Pos_ - 1); Y_UNREACHABLE();}
        }

        if (isRows) {
            Hints_.CardinalityHints->PushBack(std::move(labels), op, value, "Rows" + Text_.substr(beginPos, Pos_ - beginPos + 1));
        } else {
            Hints_.BytesHints->PushBack(std::move(labels), op, value, "Bytes" + Text_.substr(beginPos, Pos_ - beginPos + 1));
        }
    }

private:
    // Expressions
    void ParseError(const TString& err, i32 pos) {
        auto [line, linePos] = GetLineAndLinePosFromTextPos(pos);
        Y_ENSURE(false, Sprintf("Optimizer hints parser error at [line:%d, pos:%d], msg: %s", line, linePos, err.c_str()));
    }

    TString Label() {
        return Term(LabelAllowedSymbols());
    }

    std::optional<TString> MaybeLabel() {
        try {
            return Label();
        } catch (...) {
            return std::nullopt;
        }
    }

    TString Term(const std::bitset<256>& allowedSym = {}) {
        SkipWhiteSpaces();
        Y_ENSURE(Pos_ < Size_, "Expected <string>, but got end of the string.");

        TString term;
        while (Pos_ < Size_) {
            try {
                term.push_back(Char(allowedSym));
            } catch (...) {
                break;
            }
        }

        if (term.empty()) {
            ParseError("Expected a term!", Pos_);
        }
        return term;
    }

    char Char(unsigned char c) {
        std::bitset<256> allowed;
        allowed[c] = 1;
        return Char(allowed);
    }

    char Char(unsigned char intervalBegin, unsigned char intervalEnd) {
        std::bitset<256> allowed;
        for (size_t i = intervalBegin; i <= intervalEnd; ++i) {
            allowed[i] = 1;
        }
        return Char(allowed);
    }

    char Char(const std::bitset<256>& allowedSymbols = {}) {
        Y_ENSURE(Pos_ < Size_, Sprintf("Expected [%s], but got end of the string.", ""));

        char nextSym = Text_[Pos_ + 1];
        if (allowedSymbols.count() == 0) {
            ++Pos_;
            return nextSym;
        }

        for (size_t i = 0; i < allowedSymbols.size(); ++i) {
            if (allowedSymbols[i] && tolower(i) == tolower(nextSym)) {
                ++Pos_;
                return nextSym;
            }
        }

        ParseError(Sprintf("Expected [%s], but got [%c]", "", nextSym), Pos_);
        Y_UNREACHABLE();
    }

    std::optional<TString> MaybeKeyword(const TVector<TString>& keywords) {
        try {
            return Keyword(keywords);
        } catch(...) {
            return std::nullopt;
        }
    }

    TString Keyword(const TVector<TString>& keywords) {
        SkipWhiteSpaces();
        Y_ENSURE(Pos_ < Size_, Sprintf("Expected [%s], but got end of the string.", JoinSeq(", ", keywords).c_str()));

        for (const auto& keyword: keywords) {
            size_t lowInclude = Pos_ + 1;
            size_t highExclude = lowInclude + keyword.size();

            if (Text_.substr(lowInclude, highExclude - lowInclude).equal(keyword)) {
                Pos_ += keyword.size();
                return keyword;
            }
        }

        ParseError(Sprintf("Expected [%s], but got [%c]", JoinSeq(", ", keywords).c_str(), Text_[Pos_ + 1]), Pos_);
        Y_UNREACHABLE();
    }

    double Number() {
        SkipWhiteSpaces();
        Y_ENSURE(Pos_ < Size_, Sprintf("Expected number, but got end of the string."));

        TString number;
        if (auto maybeSign = MaybeKeyword({"+", "-"})) {
            number.push_back(maybeSign.value()[0]);
        }

        auto term = Term(Digits() | Chars(".-e")); // for double like 1.0 / 1e9
        try {
            return std::stod(term);
        } catch (...) {
            ParseError(Sprintf("Expected a number, got [%s]", term.c_str()), Pos_ - term.size());
        }
        Y_UNREACHABLE();
    }

private:
    // Helpers
    constexpr std::bitset<256> Chars(const TString& s) {
        std::bitset<256> res;

        for (char c: s) {
            res[c] = 1;
        }

        return res;
    }

    constexpr std::bitset<256> Letters() {
        std::bitset<256> res;

        for (unsigned char i = 'a'; i <= 'z'; ++i) {
            res[i] = 1;
        }
        for (unsigned char i = 'A'; i <= 'Z'; ++i) {
            res[i] = 1;
        }

        return res;
    }

    constexpr std::bitset<256> Digits() {
        std::bitset<256> res;

        for (unsigned char i = '0'; i <= '9'; ++i) {
            res[i] = 1;
        }

        return res;
    }

    constexpr std::bitset<256> LabelAllowedSymbols() {
        auto labelSymbols = Digits() | Letters();
        labelSymbols['_'] = 1;
        return labelSymbols;
    }

    void SkipWhiteSpaces() {
        for (; Pos_ < Size_ && isspace(Text_[Pos_ + 1]); ++Pos_) {
        }
    }

    std::pair<i32, i32> GetLineAndLinePosFromTextPos(i32 pos) {
        i32 Line = 0;
        i32 LinePos = 0;

        for (i32 i = 0; i <= pos && i < static_cast<i32>(Text_.size()); ++i) {
            if (Text_[i] == '\n') {
                LinePos = 0;
                ++Line;
            } else {
                ++LinePos;
            }
        }

        return {Line, LinePos};
    }

private:
    i32 Pos_;
    const i32 Size_;
    const TString& Text_;

private:
    TOptimizerHints Hints_;
};

TOptimizerHints TOptimizerHints::Parse(const TString& text) {
    return TOptimizerHintsParser(text).Parse();
}
