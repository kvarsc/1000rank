// 1000rank.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "json.hpp"
#include <cpr/cpr.h>

int main()
{
    nlohmann::json jsonObj;
    jsonObj["name"] = "John";
    jsonObj["age"] = 30;

    // Accessing data
    std::string name = jsonObj["name"];
    int age = jsonObj["age"];

    // Converting JSON object to a string
    std::string jsonString = jsonObj.dump();

    std::cout << jsonString << std::endl;

    cpr::Response response = cpr::Get(cpr::Url{ "https://api.example.com/data" });
    if (response.status_code == 200) {
        std::string responseData = response.text;
        std::cout << response.text << std::endl;
    }
    else {
        std::cout << "ERROR" << std::endl;
    }

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
