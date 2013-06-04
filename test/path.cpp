#include <vector>
#include "gtest/gtest.h"
#include "lightconf/lightconf.hpp"

TEST(PathTest, ConstructsSinglePartPath) {
    lightconf::path p("singlepart");
    EXPECT_EQ(1u, p.size());
    EXPECT_EQ("singlepart", p[0]);
}

TEST(PathTest, ConstructsMultiPartPath) {
    lightconf::path p("part1.part2.part3");
    ASSERT_EQ(3u, p.size());
    EXPECT_EQ("part1", p[0]);
    EXPECT_EQ("part2", p[1]);
    EXPECT_EQ("part3", p[2]);
}

TEST(PathTest, FullPathCorrect) {
    EXPECT_EQ("part1.part2.part3", lightconf::path("part1.part2.part3").fullpath());
    EXPECT_EQ("singlepart", lightconf::path("singlepart").fullpath());
}

TEST(PathTest, EmptyPathIsEmpty) {
    lightconf::path p("");
    EXPECT_TRUE(p.empty());
    EXPECT_EQ("", p.fullpath());
}

TEST(PathTest, Concatenate) {
    lightconf::path p("part1");
    p += "part2";
    p += std::string("part3");
    p += lightconf::path("part4.part5");
    EXPECT_EQ("part1.part2.part3.part4.part5", p.fullpath());
}

TEST(PathTest, IteratorTest) {
    int prod = 1;
    lightconf::path p("part2.part3.part5.part7.part9");
    for (lightconf::path::iterator it = p.begin(); it != p.end(); it++) {
        const std::string& part = *it;
        prod *= (part[4] - '0');
    }
    ASSERT_EQ(2*3*5*7*9, prod);
}
