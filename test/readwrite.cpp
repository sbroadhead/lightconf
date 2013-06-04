#include "gtest/gtest.h"
#include "lightconf/lightconf.hpp"
#include "lightconf/config_format.hpp"
#include "lightconf/json_format.hpp"

struct point {
    double x;
    double y;
};

namespace lightconf {

template <>
struct value_type_info<point> {
    static bool can_convert_from(const value& val) {
        group grp = val.get<group>(group());
        bool can = true;

        can &= grp.has<double>("x");
        can &= grp.has<double>("y");

        return can;
    }
    static point extract_value(const value& val) {
        const group& grp = val.get<group>();
        point p;
        p.x = grp.get<double>("x");
        p.y = grp.get<double>("y");
        return p;
    }
    static value create_value(const point& p) {
        group grp;
        grp.set<double>("x", p.x);
        grp.set<double>("y", p.y);
        return value(grp);
    }
};

}

class ConfigFormatTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        sampleConfig =
            "key1=\"string\"\n\n"
            "key2=1.23 // this is a comment\n"
            u8"key3=[\"list\" \"of\" \"strings\" \"\\u2603\" \"☃\"]\n"
            "key4={ \t subkey1=5 subkey2=true subkey3=[\n 2, 3,\n 4, ]\n"
            "  subkey4={//comment\nsubsubkey1=false} }";
        sampleJson =
            "{ \"key1\": \"string\",\n"
            "  \"key2\": 1.23,\n"
            u8"  \"key3\": [ \"list\", \"of\", \"strings\", \"\\u2603\", \"☃\" ],\n"
            "  \"key4\": { \"subkey1\": 5, \"subkey2\": true, \"subkey3\": [2,3,4], \n"
            "     \"subkey4\": { \"subsubkey1\": false } } }";

    }

    std::string sampleConfig;
    std::string sampleJson;
};

TEST_F(ConfigFormatTest, ParseEmptyConfig) {
    EXPECT_EQ(lightconf::group(), lightconf::config_format::read(""));
}

TEST_F(ConfigFormatTest, ParseEmptyJson) {
    EXPECT_EQ(lightconf::group(), lightconf::json_format::read("{}"));
}

TEST_F(ConfigFormatTest, ParseConfig) {
    lightconf::group grp = lightconf::config_format::read(sampleConfig);
    EXPECT_EQ("string", grp.get<std::string>("key1"));
    EXPECT_EQ(1.23, grp.get<double>("key2"));
    std::vector<std::string> v1 = {"list", "of", "strings", u8"☃", u8"☃"};
    EXPECT_EQ(v1, grp.get<std::vector<std::string>>("key3"));
    EXPECT_EQ(5, grp.get<int>("key4.subkey1"));
    EXPECT_TRUE(grp.get<bool>("key4.subkey2"));
    std::vector<int> v2 = { 2, 3, 4 };
    EXPECT_EQ(v2, grp.get<std::vector<int>>("key4.subkey3"));
    EXPECT_FALSE(grp.get<bool>("key4.subkey4.subsubkey1"));
}

TEST_F(ConfigFormatTest, ParseJson) {
    lightconf::group grp1 = lightconf::config_format::read(sampleConfig);
    lightconf::group grp2 = lightconf::json_format::read(sampleJson);
    EXPECT_EQ(grp1, grp2);
}

TEST_F(ConfigFormatTest, WriteConfig) {
    lightconf::group grp = lightconf::config_format::read(sampleConfig);
    grp.set<double>("key2", 7.55);
    grp.set<std::vector<int>>("key4.subkey3", { 5, 6, 7, 9 });
    grp.set<std::vector<int>>("key4.subkey5", { });
    grp.unset("key4.subkey2");
    point p = { 6.12, 9.1234 };
    grp.set<point>("key4.subkey4.new", p);
    std::string new_config = lightconf::config_format::write(grp, sampleConfig, 50);
    
    EXPECT_EQ(
        "key1 = \"string\"\n"
        "\n"
        "key2 = 7.55\n"
        "// this is a comment\n"
        u8"key3 = [ \"list\", \"of\", \"strings\", \"☃\", \"☃\" ]\n"
        "key4 = { \n"
        "    subkey1 = 5\n"
        "    subkey3 = [ 5, 6, 7, 9 ]\n"
        "    subkey4 = { \n"
        "        //comment\n"
        "        subsubkey1 = false\n"
        "        new = { x = 6.12, y = 9.1234 }\n"
        "    }\n"
        "    subkey5 = [ ]\n"
        "}\n",
        new_config);

    lightconf::group new_grp = lightconf::config_format::read(new_config);
    EXPECT_EQ(grp, new_grp);
}

TEST_F(ConfigFormatTest, WriteConfigWithParseFailure) {
    lightconf::group grp = lightconf::config_format::read(sampleConfig);
    grp.set<double>("key2", 7.55);
    grp.set<std::vector<int>>("key4.subkey3", { 5, 6, 7, 9 });
    grp.set<std::vector<int>>("key4.subkey5", { });
    grp.unset("key4.subkey2");
    point p = { 6.12, 9.1234 };
    grp.set<point>("key4.subkey4.new", p);
    std::string new_config = lightconf::config_format::write(grp, "asdf = {", 50);
    
    EXPECT_EQ(
        "key1 = \"string\"\n"
        "key2 = 7.55\n"
        u8"key3 = [ \"list\", \"of\", \"strings\", \"☃\", \"☃\" ]\n"
        "key4 = { \n"
        "    subkey1 = 5\n"
        "    subkey3 = [ 5, 6, 7, 9 ]\n"
        "    subkey4 = { \n"
        "        subsubkey1 = false\n"
        "        new = { x = 6.12, y = 9.1234 }\n"
        "    }\n"
        "    subkey5 = [ ]\n"
        "}",
        new_config);

    lightconf::group new_grp = lightconf::config_format::read(new_config);
    EXPECT_EQ(grp, new_grp);
}

TEST_F(ConfigFormatTest, WriteJson) {
    lightconf::group grp = lightconf::json_format::read(sampleJson);
    grp.set<double>("key2", 7.55);
    grp.set<std::vector<int>>("key4.subkey3", { 5, 6, 7, 9 });
    grp.set<std::vector<int>>("key4.subkey5", { });
    grp.unset("key4.subkey2");
    point p = { 6.12, 9.1234 };
    grp.set<point>("key4.subkey4.new", p);
    std::string new_json = lightconf::json_format::write(grp);
    EXPECT_EQ(
        "{\n"
        "    \"key1\": \"string\",\n"
        "    \"key2\": 7.55,\n"
        "    \"key3\": [\n"
        "        \"list\",\n"
        "        \"of\",\n"
        "        \"strings\",\n"
        u8"        \"☃\",\n"
        u8"        \"☃\"\n"
        "    ],\n"
        "    \"key4\": {\n"
        "        \"subkey1\": 5,\n"
        "        \"subkey3\": [\n"
        "            5,\n"
        "            6,\n"
        "            7,\n"
        "            9\n"
        "        ],\n"
        "        \"subkey4\": {\n"
        "            \"subsubkey1\": false,\n"
        "            \"new\": {\n"
        "                \"x\": 6.12,\n"
        "                \"y\": 9.1234\n"
        "            }\n"
        "        },\n"
        "        \"subkey5\": [\n"
        "            \n"
        "        ]\n"
        "    }\n"
        "}",
        new_json);

    lightconf::group new_grp = lightconf::json_format::read(new_json);
    EXPECT_EQ(grp, new_grp);
}