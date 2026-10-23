// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives
TEST(reg_null_rejected) {
    ToolRegistry tr;
    CHECK(!tr.add(nullptr));
    CHECK(tr.size() == 0);
}

TEST(reg_find_absent_name) {
    ToolRegistry tr;
    CHECK(tr.find("nothing") == nullptr);
    REQUIRE(tr.add(std::make_unique<NamedTool>("alpha")));
    CHECK(tr.find("beta") == nullptr);
    CHECK(tr.size() == 1);
    CHECK(tr.bucket_count() >= 1 && tr.bucket_count() <= 64);
}

TEST(reg_for_each_visits_each_once) {
    ToolRegistry tr;
    for (const char* n : {"a", "b", "c", "d", "e"}) {
        REQUIRE(tr.add(std::make_unique<NamedTool>(n)));
    }
    std::vector<std::string> seen;
    tr.for_each([&](const Tool& t) { seen.push_back(t.name()); });
    CHECK(seen.size() == 5);
    std::sort(seen.begin(), seen.end());
    CHECK(seen == (std::vector<std::string>{"a", "b", "c", "d", "e"}));
}
TEST(reg_10000_tools) {
    ToolRegistry tr;
    Tool* first = nullptr;
    for (int i = 0; i < 10000; ++i) {
        auto t = std::make_unique<NamedTool>("tool_" + std::to_string(i));
        Tool* raw = t.get();
        REQUIRE(tr.add(std::move(t)));
        if (i == 0) first = raw;
        CHECK(tr.load_factor() <= 1.0);
    }
    CHECK(tr.size() == 10000);
    CHECK(tr.find("tool_0") == first);                  // pointer survived every resize
    for (int i = 0; i < 10000; ++i) {
        CHECK(tr.find("tool_" + std::to_string(i)) != nullptr);
    }
    CHECK(tr.max_probe_length() <= 64);
    std::printf("[10000 tools] bucket_count=%zu load_factor=%.3f max_probe_length=%zu\n",
                tr.bucket_count(), tr.load_factor(), tr.max_probe_length());
}
TEST(reg_alternative_hash) {
    ToolRegistry r;
    for (int i = 0; i < 10000; ++i) {
        REQUIRE(r.add(std::make_unique<NamedTool>("tool_" + std::to_string(i))));
    }
    const std::size_t buckets = r.bucket_count();
    std::vector<int> weak_counts(buckets, 0);
    int weak_longest = 0;
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        std::size_t weak = static_cast<unsigned char>(name[0]) + name.size();   // s[0] + s.size()
        int len = ++weak_counts[weak % buckets];
        if (len > weak_longest) weak_longest = len;
    }
    std::printf("[hash comparison] real longest chain=%zu, weak hash longest chain=%d\n",
                r.max_probe_length(), weak_longest);
    CHECK(weak_longest > 64);                          // the weak hash really is bad
    CHECK(static_cast<std::size_t>(weak_longest) > r.max_probe_length());
}

// ---- workspace (every row of spec 4.4) ----

TEST(ws_accepts_valid_paths) {
    TempDir d;
    Workspace ws(d.path());
    struct Row { std::string in; std::string rel; };    // rel "" means the root itself
    const std::vector<Row> rows = {
        {"notes/todo.txt",     "notes/todo.txt"},
        {"./notes//todo.txt",  "notes/todo.txt"},
        {".",                  ""},
        {"a/../b",             "b"},
        {"a/..",               ""},
        {"new/dir/file.txt",   "new/dir/file.txt"},
    };
    for (const Row& row : rows) {
        fs::path out;
        std::string reason;
        CHECK(ws.resolve(row.in, &out, &reason));
        CHECK(out == (row.rel.empty() ? ws.root() : ws.root() / row.rel));
    }
}

TEST(ws_allows_dots_inside_names) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(ws.resolve("my..notes.txt", &out, &reason));  // not a ".." component
    CHECK(out == ws.root() / "my..notes.txt");
}

TEST(ws_rejects_empty_and_nul) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("", &out, &reason));
    CHECK(!reason.empty());
    reason.clear();
    CHECK(!ws.resolve(std::string("notes\0/x", 8), &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_absolute) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("/etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_escapes) {
    TempDir d;
    Workspace ws(d.path());
    const std::string root_name = ws.root().filename().string();
    const std::vector<std::string> bad = {
        "..",
        "../etc/passwd",
        "a/../../b",
        "notes/../../" + root_name + "/notes/todo.txt",   // climbs out, then back in by name
    };
    for (const std::string& p : bad) {
        fs::path out;
        std::string reason;
        CHECK(!ws.resolve(p, &out, &reason));
        CHECK(!reason.empty());
    }
}

// ---- calculator (every row of spec 4.6, plus two of my own) ----

TEST(calc_left_to_right) {
    double v = 0;
    std::string err;
    REQUIRE(eval("10 - 4 - 3", &v, &err));
    CHECK(v == 3);
    REQUIRE(eval("100 / 10 / 5", &v, &err));
    CHECK(v == 2);
}

TEST(calc_parentheses) {
    double v = 0;
    std::string err;
    REQUIRE(eval("(2 + 3) * 4", &v, &err));
    CHECK(v == 20);
    REQUIRE(eval("2 * (3 + 4) - 5", &v, &err));
    CHECK(v == 9);
}

TEST(calc_negatives_and_decimals) {
    double v = 0;
    std::string err;
    REQUIRE(eval("-3 * -3", &v, &err));
    CHECK(v == 9);
    REQUIRE(eval("7 / 2", &v, &err));
    CHECK(v == 3.5);
}

TEST(calc_division_by_zero_in_parens) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / (2 - 2)", &v, &err));
    CHECK(err == "division by zero");
}

TEST(calc_my_own_expressions) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 * (3 + 4) * (5 - 1)", &v, &err));
    CHECK(v == 56);
    REQUIRE(eval("10 - (4 - 3)", &v, &err));
    CHECK(v == 9);
}

TEST(calc_very_long_and_deeply_nested) {
    double v = 0;
    std::string err;
    std::string longexpr = "1";
    for (int i = 0; i < 20000; ++i) longexpr += "+1";
    REQUIRE(eval(longexpr, &v, &err));
    CHECK(v == 20001);

    std::string deep = std::string(5000, '(') + "1" + std::string(5000, ')');
    REQUIRE(eval(deep, &v, &err));
    CHECK(v == 1);
}


int main(int argc, char** argv) { return mt::run_all(argc, argv); }
