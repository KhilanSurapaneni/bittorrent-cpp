#include <iostream>
#include <string>
#include <vector>
#include <cctype>  // For std::isdigit
#include <cstdlib> // For std::atoll (convert string to long long int)

#include "lib/nlohmann/json.hpp"  // Include for using JSON data structures

using json = nlohmann::json;  // Alias for nlohmann::json library usage

// Function to decode a bencoded string
json decode_bencoded_string(const std::string& encoded_string, size_t& position) {
    // Find the colon that separates the string length and the actual string
    size_t length_prefix = encoded_string.find(':', position);
    if (length_prefix != std::string::npos) {
        // Extract the substring before the colon (this indicates the string length)
        std::string string_size_str = encoded_string.substr(position, length_prefix - position);
        
        // Convert the extracted length string to an integer
        int64_t string_size_int = std::atoll(string_size_str.c_str());
        
        // Update the position to point past the colon and the length of the string
        position = length_prefix + 1 + string_size_int;
        
        // Extract the actual string content based on the previously extracted length
        std::string str = encoded_string.substr(length_prefix + 1, string_size_int);
        
        // Return the string as a JSON object
        return json(str);
    } else {
        // If no colon is found, the encoded string is invalid
        throw std::runtime_error("Invalid encoded value: " + encoded_string);
    }
}

// Function to decode a bencoded integer (in the form i<number>e)
json decode_bencoded_integer(const std::string& encoded_value, size_t& position) {
    position++;  // Skip the 'i' character
    size_t end = encoded_value.find('e', position);  // Find the terminating 'e'
    if (end == std::string::npos) {
        throw std::invalid_argument("Invalid bencoded integer");  // Throw an error if no 'e' is found
    }
    
    // Extract the integer string (the part between 'i' and 'e')
    std::string integer_str = encoded_value.substr(position, end - position);
    
    // Update the position to after the 'e'
    position = end + 1;
    
    // Convert the string to a long long integer and return it as a JSON object
    return std::stoll(integer_str);
}

// Forward declaration of the decode_bencoded_value function to handle different types
json decode_bencoded_value(const std::string& encoded_value, size_t& position);

// Function to decode a bencoded list (in the form l<values>e)
json decode_bencoded_list(const std::string& encoded_value, size_t& position) {
    position++;  // Skip the 'l' character at the beginning of the list
    
    // Initialize a JSON array to hold the decoded list elements
    json list = json::array();
    
    // Loop through the list elements until the terminating 'e' is found
    while (encoded_value[position] != 'e') {
        // Decode each value and add it to the list
        list.push_back(decode_bencoded_value(encoded_value, position));
    }
    
    position++;  // Skip the 'e' character at the end of the list
    return list;  // Return the JSON array
}

json decode_bencoded_dictionary(const std::string& encoded_value, size_t& position) {
    position++;  // Skip the 'd' character at the beginning of the dictionary
    json dictionary = json::object();  // Initialize a JSON object to hold the decoded dictionary

    // Loop through the dictionary elements until the terminating 'e' is found
    while (encoded_value[position] != 'e') {
        // Decode each key and add it to the dictionary
        std::string key = decode_bencoded_string(encoded_value, position).get<std::string>();

        // Decode each value and add it to the dictionary
        dictionary[key] = decode_bencoded_value(encoded_value, position);
    }
    position++;  // Skip the 'e' character at the end of the dictionary

    // Return the JSON object
    return dictionary;
}

// Function to decode different bencoded values (string, integer, list)
json decode_bencoded_value(const std::string& encoded_value, size_t& position) {
    if (std::isdigit(encoded_value[position])) {
        // If the current character is a digit, it's a string
        return decode_bencoded_string(encoded_value, position);
    } else if (encoded_value[position] == 'i') {
        // If the current character is 'i', it's an integer
        return decode_bencoded_integer(encoded_value, position);
    } else if (encoded_value[position] == 'l') {
        // If the current character is 'l', it's a list
        return decode_bencoded_list(encoded_value, position);
    } else if (encoded_value[position] == 'd') {
        // If the current character is 'd', it's a dictionary
        return decode_bencoded_dictionary(encoded_value, position);
    } else {
        // Throw an error if the value type is not handled
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}

// Helper function to initiate the decoding process (starts with position 0)
json decode_bencoded_value(const std::string& encoded_value) {
    size_t position = 0;  // Initial position in the string
    return decode_bencoded_value(encoded_value, position);  // Call the main decoding function
}

int main(int argc, char* argv[]) {
    // Enable automatic flushing of std::cout and std::cerr buffers after each output operation
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // Check if there are enough command-line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
        return 1;
    }

    std::string command = argv[1];  // Store the first command-line argument as the command

    if (command == "decode") {
        // If the command is "decode", check if an encoded value is provided
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
            return 1;
        }

        // Store the second command-line argument as the encoded value
        std::string encoded_value = argv[2];
        
        // Decode the bencoded value and print the resulting JSON
        json decoded_value = decode_bencoded_value(encoded_value);
        std::cout << decoded_value.dump() << std::endl;
    } else {
        // If the command is not "decode", print an error message
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }

    return 0;  // Exit the program
}