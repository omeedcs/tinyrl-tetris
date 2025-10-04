#include "input.h"
#include <iostream>
#include <string>

namespace Input {
    int getAction() {
        // get user input and map
        std::string input;
        std::cin >> input;

        if (input == "j") {
            return static_cast<int>(Action::LEFT);
        } else if (input == "l") {
            return static_cast<int>(Action::RIGHT);
        } else if (input == "k") {
            return static_cast<int>(Action::DOWN);
        } else if (input == "o") {
            return static_cast<int>(Action::CW);
        } else if (input == "u") {
            return static_cast<int>(Action::CCW);
        } else if (input == "K") {
            return static_cast<int>(Action::DROP);
        } else if (input == ",") {
            return static_cast<int>(Action::SWAP);
        } else {
            return static_cast<int>(Action::NOOP);
        }
    }
}
