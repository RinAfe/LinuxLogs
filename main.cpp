#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>
#include <iomanip>

std::time_t parseTimestamp(const std::string& timestamp) {
    std::tm tm = {};

    std::string clean_timestamp = timestamp;
    size_t dot_pos = clean_timestamp.find('.');
    if (dot_pos != std::string::npos) {
        clean_timestamp = clean_timestamp.substr(0, dot_pos);
    }

    std::istringstream ss(clean_timestamp);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return std::mktime(&tm);
}

int main() {
    std::string line;
    std::map<std::string, std::vector<std::time_t>> requests;

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string timestamp_str, trace_id, event, message;

        iss >> timestamp_str >> trace_id >> event;
        std::getline(iss, message);

        if (!message.empty() && message[0] == ' ') {
            message = message.substr(1);
        }

        if (event == "Request") {
            std::time_t timestamp = parseTimestamp(timestamp_str);

            if (message.find("started") != std::string::npos) {
                requests[trace_id].push_back(timestamp);
            }
            else if (message.find("completed") != std::string::npos ||
                message.find("failed") != std::string::npos) {
                requests[trace_id].push_back(timestamp);
            }
        }
    }

    std::vector<std::pair<std::string, std::time_t>> durations;

    for (const auto& req : requests) {
        if (req.second.size() >= 2) {
            std::time_t start = req.second.front();
            std::time_t end = req.second.back();
            std::time_t duration = end - start;
            durations.push_back({ req.first, duration });
        }
    }

    std::sort(durations.begin(), durations.end(),
    [](const std::pair<std::string, std::time_t>& a,
       const std::pair<std::string, std::time_t>& b) {
        return a.second > b.second;
    });

    for (size_t i = 0; i < std::min(durations.size(), size_t(5)); i++) {
        std::cout << durations[i].first << std::endl;
    }

    return 0;
}