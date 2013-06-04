#include <string>
#include <tuple>
#include <vector>
#include "gtest/gtest.h"
#include "lightconf/lightconf.hpp"

enum class color {
    red, green, blue
};

struct point {
    double x;
    double y;
};

namespace lightconf {

LIGHTCONF_BEGIN_ENUM(color)
    LIGHTCONF_ENUM_VALUE(color::red,    "RED")
    LIGHTCONF_ENUM_VALUE(color::green,  "GREEN")
    LIGHTCONF_ENUM_VALUE(color::blue,   "BLUE")
LIGHTCONF_END_ENUM()

LIGHTCONF_BEGIN_TYPE(point)
    LIGHTCONF_TYPE_MEMBER_REQ(double,   x,  "x")
    LIGHTCONF_TYPE_MEMBER_REQ(double,   y,  "y")
LIGHTCONF_END_TYPE()

}

class GroupTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        grp.set<int>("group1.group2.intval", 5);
        grp.set<std::string>("group1.group2.strval", "hello");
        grp.set<double>("group1.group2.dblval", 1.23);
        grp.set<float>("group1.group2.floatval", 4.56f);
        grp.set<std::vector<int>>("group2.vec", { 1, 2, 3 });
    }

    lightconf::group grp;
};

TEST_F(GroupTest, ReadCorrect) {
    std::vector<int> res = { 1, 2, 3 };
    EXPECT_EQ(5, grp.get<int>("group1.group2.intval"));
    EXPECT_EQ(res, grp.get<std::vector<int>>("group2.vec"));
}

TEST_F(GroupTest, CustomTypeStoredCorrectly) {
    point p = { 10, 20 };
    grp.set<point>("pointval", p);
    EXPECT_EQ(10, grp.get<point>("pointval").x);
    EXPECT_EQ(20, grp.get<point>("pointval").y);
    EXPECT_EQ(10, grp.get<double>("pointval.x"));
    EXPECT_EQ(20, grp.get<double>("pointval.y"));
}

TEST_F(GroupTest, CustomEnumStoredCorrectly) {
    grp.set<color>("col", color::blue);
    EXPECT_EQ("BLUE", grp.get<std::string>("col"));
    EXPECT_EQ(color::blue, grp.get<color>("col"));
}

TEST_F(GroupTest, CustomTypeMalformed) {
    point p = { 5, 6 };
    grp.set<double>("a.x", 10);
    grp.set<double>("a.z", 15);
    EXPECT_TRUE(grp.has<lightconf::group>("a"));
    EXPECT_EQ(5, grp.get<point>("a", p).x);
    EXPECT_EQ(6, grp.get<point>("a", p).y);
}

TEST_F(GroupTest, DefaultValueCorrect) {
    EXPECT_EQ(10, grp.get<int>("nonexistent.key", 10));
}

TEST_F(GroupTest, NonExistantValueThrowsException) {
    EXPECT_THROW(grp.get<int>("nonexistent.key"), lightconf::path_error);
}

TEST_F(GroupTest, IncorrectTypeThrowsException) {
    EXPECT_THROW(grp.get<std::string>("group1.group2.intval"), lightconf::value_error);
}

TEST_F(GroupTest, ReplaceGroupWithValue) {
    grp.set<int>("group1.group2", 10);
    EXPECT_EQ(10, grp.get<int>("group1.group2"));
    EXPECT_THROW(grp.get<int>("group1.group2.intval"), lightconf::path_error);
}

TEST_F(GroupTest, ReplaceValueWithGroup) {
    grp.set<int>("group1.group2.intval.sub1.sub2", 20);
    EXPECT_EQ(20, grp.get<int>("group1.group2.intval.sub1.sub2"));
    EXPECT_THROW(grp.get<int>("group1.group2.intval"), lightconf::value_error);
}

TEST_F(GroupTest, EmptyPathThrowsException) {
    EXPECT_THROW(grp.get<int>(""), lightconf::path_error);
}

TEST_F(GroupTest, UnsetRemovesItem) {
    grp.unset("group1.group2.intval");
    EXPECT_THROW(grp.get<int>("group1.group2.intval"), lightconf::path_error);
    EXPECT_EQ("hello", grp.get<std::string>("group1.group2.strval"));
}

TEST_F(GroupTest, VectorOfGroups) {
    lightconf::group g1;
    lightconf::group g2;
    lightconf::group g3;
    g1.set<int>("intval", 5);
    g2.set<std::string>("strval", "hello");
    g3.set<double>("dblval", 1.23);
    std::vector<lightconf::group> groups = { g1, g2, g3 };
    grp.set("group1.group2.vecval", groups);
    EXPECT_EQ(groups, grp.get<std::vector<lightconf::group>>("group1.group2.vecval"));
}

TEST_F(GroupTest, VectorOfVectors) {
    std::vector<int> v = { 1, 2, 3 };
    std::vector<std::vector<int>> vs = { v, v, v };
    grp.set("group1.group2.vecval", vs);
    EXPECT_EQ(vs, grp.get<std::vector<std::vector<int>>>("group1.group2.vecval"));
}

TEST_F(GroupTest, StoresTuplesProperly) {
    auto tup1 = std::make_tuple(123, "hello", 3.14);
    grp.set("group1.group2.tupval", tup1);
    auto tup2 = grp.get<std::tuple<int, std::string, double>>("group1.group2.tupval");
    EXPECT_EQ(tup1, tup2);
}

TEST_F(GroupTest, InvalidKeys) {
    EXPECT_THROW(grp.set<int>("_abc", 1), lightconf::path_error);
    EXPECT_THROW(grp.set<int>("abc@", 1), lightconf::path_error);
}

TEST_F(GroupTest, IteratorTest) {
    lightconf::group g;
    g.set<int>("abc", 5);
    g.set<int>("def", 10);
    g.set<int>("ghi", 15);
    std::string s;
    int d = 0;

    for (auto key : g) { 
        s += key;
        d += g.get<int>(key);
    }

    EXPECT_EQ("abcdefghi", s);
    EXPECT_EQ(5+10+15, d);
}
