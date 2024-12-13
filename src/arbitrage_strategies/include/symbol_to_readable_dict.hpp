//
// Created by cyberjunk2023 on 2024-11-18.
//

#ifndef SYMBOL_TO_READABLE_DICT_HPP
#define SYMBOL_TO_READABLE_DICT_HPP

#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

// Function to save dictionary to a file
void saveDictionary(const std::unordered_map<std::string, std::string>& dictionary, const std::string& filename="/Users/toddtan/Desktop/DV_Trading/ucpl-f24/conf/symbol_to_readable.txt") {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& [machineCode, humanReadable] : dictionary) {
            file << machineCode << " " << humanReadable << "\n";
        }
        file.close();
    } else {
        std::cerr << "Unable to open file for writing." << std::endl;
    }
}

void saveDictionaryToCSV(const std::unordered_map<std::string, unordered_map<std::string, std::string>>& dictionary, const std::string& filename="/Users/toddtan/Desktop/DV_Trading/ucpl-f24/conf/symbol_to_readable.txt") {
    std::ofstream file(filename);
    if (file.is_open()) {
        //{"name":"VXV4", "id":"000XIZ", "legs":{}, "tick":0.05, "size":1000, "product":"VX/V4", "root":"VX", "expiry":"2024-10-16", "description":"VX Monthly Oct 2024", "test":false}
        file << "name,id,legs,tick,size,product,root,expiry,test\n";
        for (const auto& outerPair : dictionary) {
            const std::string& outerKey = outerPair.first;
            const std::unordered_map<std::string, std::string>& data = outerPair.second;
            file << data.at("name") << "," << outerKey << "," << data.at("legs") << "," << data.at("tick") << "," << data.at("size") << "," << data.at("product") << "," << data.at("root") << "," << data.at("expiry") << "," << data.at("test") << "\n";
        }
        file.close();
    } else {
        std::cerr << "Unable to open file for writing." << std::endl;
    }
}

// Function to load dictionary from a file
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> loadDictionaryFromCSV(const std::string& filename) {
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> dictionary;
    std::ifstream file(filename);

    if (file.is_open()) {
        std::string line;

        // Skip the header line
        std::getline(file, line);

        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string token;

            // Container for the parsed fields
            std::unordered_map<std::string, std::string> data;

            // Read each field from the CSV line
            std::string name, id, legs, tick, size, product, root, expiry, test;

            std::getline(ss, name, ',');
            std::getline(ss, id, ',');
            std::getline(ss, legs, ',');
            std::getline(ss, tick, ',');
            std::getline(ss, size, ',');
            std::getline(ss, product, ',');
            std::getline(ss, root, ',');
            std::getline(ss, expiry, ',');
            std::getline(ss, test, ',');

            // Populate the inner map
            data["name"] = name;
            data["legs"] = legs;
            data["tick"] = tick;
            data["size"] = size;
            data["product"] = product;
            data["root"] = root;
            data["expiry"] = expiry;
            data["test"] = test;

            // Insert into the dictionary with id as the outer key
            dictionary[id] = data;
        }

        file.close();
    } else {
        std::cerr << "Unable to open file for reading." << std::endl;
    }

    return dictionary;
}

void printDictionary(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& dictionary) {
    for (const auto& outerPair : dictionary) {
        const std::string& outerKey = outerPair.first;
        const auto& data = outerPair.second;

        std::cout << "ID: " << outerKey << "\n";
        std::cout << "  Name: " << data.at("name") << "\n";
        std::cout << "  Legs: " << data.at("legs") << "\n";
        std::cout << "  Tick: " << data.at("tick") << "\n";
        std::cout << "  Size: " << data.at("size") << "\n";
        std::cout << "  Product: " << data.at("product") << "\n";
        std::cout << "  Root: " << data.at("root") << "\n";
        std::cout << "  Expiry: " << data.at("expiry") << "\n";
        std::cout << "  Test: " << data.at("test") << "\n";
        std::cout << "---------------------------\n";
    }
}

#endif //SYMBOL_TO_READABLE_DICT_HPP
