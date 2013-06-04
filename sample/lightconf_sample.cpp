#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "lightconf/lightconf.hpp"
#include "lightconf/config_format.hpp"

/* 
   This sample program gives an overview of using the lightconf library. It
   will generate a configuration file that looks something like this:

        users = [
            {
                uid = 123
                first_name = "John"
                last_name = "Doe"
                permissions = [ "READ", "CREATE", "UPDATE" ]
                join_date = [ 5, 5, 2013 ]
            }
            {
                uid = 331
                first_name = "Test"
                last_name = "Person"
                permissions = [ "UPDATE", "CREATE" ]
                join_date = [ 2, 4, 2012 ]
            }
        ]
        global = { maintainer = "user" }

*/

//
//
enum class permission {
    READ, CREATE, DELETE, UPDATE
};

//
//
struct date {
    int month;
    int day;
    int year;
};

//
//
struct person {
    int userid;
    std::string firstname;
    std::string lastname;
    std::vector<permission> perms;
    date joindate;
};

namespace lightconf {

// Custom types can be serialized by creating a specialization of the
// lightconf::value_type_info struct. This specialization serializes a date struct as 
// a vector of three integers.
template <> struct value_type_info<date> {
    static bool can_convert_from(const value& val) {
        if (!val.is<std::vector<int>>()) return false;
        auto vec = val.get<std::vector<int>>();
        return vec.size() == 3;
    }
    static date extract_value(const value& val) {
        auto vec = val.get<std::vector<int>>();
        return { vec[0], vec[1], vec[2] };
    }
    static value create_value(const date& x) {
        return value_type_info<std::vector<int>>::create_value({ x.month, x.day, x.year });
    }
};

// There is a convenience macro for automatically serializing enumeration types by mapping
// them to string values. This must be placed inside the lightconf namespace.
LIGHTCONF_BEGIN_ENUM(permission)
    LIGHTCONF_ENUM_VALUE(permission::READ,   "READ")
    LIGHTCONF_ENUM_VALUE(permission::CREATE, "CREATE")
    LIGHTCONF_ENUM_VALUE(permission::DELETE, "DELETE")
    LIGHTCONF_ENUM_VALUE(permission::UPDATE, "UPDATE")
LIGHTCONF_END_ENUM()

// There is also a convenience macro for automatically serializing simple structs, as long
// as all the members can be serialized.

// LIGHTCONF_TYPE_MEMBER_REQ specifies a required field.
// LIGHTCONF_TYPE_MEMBER_OPT specifies an optional field with a default value.
LIGHTCONF_BEGIN_TYPE(person)
    LIGHTCONF_TYPE_MEMBER_REQ(int,                      userid,     "uid")
    LIGHTCONF_TYPE_MEMBER_OPT(std::string,              firstname,  "first_name",   "")
    LIGHTCONF_TYPE_MEMBER_OPT(std::string,              lastname,   "last_name",    "")
    LIGHTCONF_TYPE_MEMBER_REQ(std::vector<permission>,  perms,      "permissions")
    LIGHTCONF_TYPE_MEMBER_REQ(date,                     joindate,   "join_date")
LIGHTCONF_END_TYPE()

}

//
//
void usage() {
    std::cout <<
        "lightconf sample program usage:\n"
        "  lightconf_sample [command] [args]\n"
        "\n"
        "Commands\n"
        "--------\n"
        "setmaintainer <name>\n"
        "   Set the name of the maintainer of the user config file\n"
        "adduser <uid> <firstname> <lastname> <permissions> <mm> <dd> <yyyy>\n"
        "   Add a new user to the config file\n"
        "     uid         The user's unique identification number\n"
        "     firstname   The first name of the user\n"
        "     lastname    The last name of the user\n"
        "     permissions A string optionally containing the letters 'R', 'C', 'U', and 'D' for\n"
        "                 Read, Create, Update, and Delete permissions respectively\n"
        "     mm dd yyyy  The date on which the user joined the system\n"
        "deluser <uid>\n"
        "   Delete the specified user from the system\n"
        "print\n"
        "   Print the current state of the system\n";
}

//
//
void argfail() {
    std::cout << "Incorrect arguments\n";
    exit(1);
}

//
//
int main(int argc, char *argv[]) {
    if (argc < 2) {
        usage();
        return 1;
    }

    std::string filename = "users.config";
    lightconf::group config_group;
    std::string source = "";
    try {
        std::ifstream stream(filename, std::ios::in);
        if (stream) {
            // If the file exists, load the configuration group from it
            source = std::string(std::istreambuf_iterator<char>(stream),
                std::istreambuf_iterator<char>());
            config_group = lightconf::config_format::read(source);
        } else {
            // If the file does not exist, just create an empty group
            config_group = lightconf::group();
        }
    } catch (const lightconf::parse_error& e) {
        std::cout << "Parse error (" << e.line() << ":" << e.col() << "): " << e.what();
        return 1;
    } catch (const std::exception& e) {
        std::cout << "Failed to read configuration file: " << e.what();
        return 1;
    }

    std::string command(argv[1]);
    std::vector<person> users = config_group.get<std::vector<person>>("users", {});

    if (command == "setmaintainer") {
        if (argc != 3) argfail();

        // Keys can be specified using full paths, and nonexistant parent groups will
        // be automatically created.
        config_group.set<std::string>("global.maintainer", argv[2]);
        std::cout << "Maintainer set to " << argv[2] << ".\n";
    } else if (command == "adduser") {
        if (argc != 9) argfail();
        person p;
        p.userid = atoi(argv[2]);
        p.firstname = argv[3];
        p.lastname = argv[4];
        for (unsigned int i = 0; i < strlen(argv[5]); i++) {
            if (argv[5][i] == 'R') p.perms.push_back(permission::READ);
            else if (argv[5][i] == 'C') p.perms.push_back(permission::CREATE);
            else if (argv[5][i] == 'U') p.perms.push_back(permission::UPDATE);
            else if (argv[5][i] == 'D') p.perms.push_back(permission::DELETE);
            else std::cout << "Unknown permission letter: " << std::string(1, argv[5][i]) << "\n";
        }
        p.joindate.day = atoi(argv[6]);
        p.joindate.month = atoi(argv[7]);
        p.joindate.year = atoi(argv[8]);

        if (p.joindate.day < 1 || p.joindate.day > 31) {
            std::cout << "Day out of range [1-31], adjusted to 1.\n";
            p.joindate.day = 1;
        }
        if (p.joindate.month < 1 || p.joindate.month > 12) {
            std::cout << "Month out of range [1-12], adjusted to 1.\n";
            p.joindate.month = 1;
        }
        if (p.joindate.year < 1970 || p.joindate.year > 2099) {
            std::cout << "Year out of range [1970-2099], adjusted to 1970.\n";
            p.joindate.year = 1970;
        }

        for (const auto& x : users) {
            if (x.userid == p.userid) {
                std::cout << "User id already exists.\n";
                goto save;
            }
        }
        users.push_back(p);
        std::cout << "User added.\n";
    } else if (command == "deluser") {
        if (argc != 3) argfail();
        int uid = atoi(argv[2]);
        for (unsigned int i = 0; i < users.size(); i++) {
            if (users[i].userid == uid) {
                users.erase(std::begin(users) + i);
                std::cout << "User deleted.\n";
                goto save;
            }
        }
        std::cout << "User not found.\n";
    } else if (command == "print") {
        std::cout << "Maintainer: "
            << config_group.get<std::string>("global.maintainer", "<not set>") << "\n";
        std::cout << "Users:\n";
        for (const auto& x : users) {
            std::cout << "    name:        " << x.firstname << " " << x.lastname << "\n";
            std::cout << "    uid:         " << x.userid << "\n";
            std::cout << "    permissions: ";
            for (const auto& p : x.perms) {
                if (p == permission::READ) std::cout << "read ";
                else if (p == permission::CREATE) std::cout << "create ";
                else if (p == permission::UPDATE) std::cout << "update ";
                else if (p == permission::DELETE) std::cout << "delete ";
            }
            std::cout << "\n";
            std::cout << "    join date:   " << x.joindate.month << "/"
                << x.joindate.day << "/" << x.joindate.year << "\n";
            std::cout << "\n";
        }
    }

save:
    config_group.set<std::vector<person>>("users", users);
    std::ofstream stream(filename, std::ios::out);
    if (stream) {
        stream << lightconf::config_format::write(config_group, source, 80);
        std::cout << "Configuration saved.";
    }
}
