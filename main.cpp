#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <unordered_map>
#include <ctime>
#include <iomanip>
#include <algorithm>

struct LogEntry {
    std::string timestamp;
    std::string trace_id;
    std::string event;
    std::string message;
};

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

LogEntry parseLogLine(const std::string& line) {
    LogEntry entry;
    std::istringstream iss(line);

    iss >> entry.timestamp >> entry.trace_id >> entry.event;

    std::string message_part;
    std::getline(iss, message_part);
    if (!message_part.empty() && message_part[0] == ' ') {
        entry.message = message_part.substr(1);
    } else {
        entry.message = message_part;
    }

    return entry;
}

int main() {
    std::string line;

    std::unordered_map<std::string, std::time_t> start_times;

    using DurationPair = std::pair<std::time_t, std::string>;
    auto cmp = [](const DurationPair& a, const DurationPair& b) {
        return a.first > b.first;
    };

    std::priority_queue<DurationPair,
                        std::vector<DurationPair>,
                        decltype(cmp)> top_5(cmp);

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        LogEntry entry = parseLogLine(line);

        if (entry.event == "Request") {
            std::time_t timestamp = parseTimestamp(entry.timestamp);

            if (entry.message.find("started") == 0) {
                start_times[entry.trace_id] = timestamp;
            }
            else if (entry.message.find("completed") == 0 ||
                     entry.message.find("failed") == 0) {

                auto it = start_times.find(entry.trace_id);
                if (it != start_times.end()) {
                    std::time_t duration = timestamp - it->second;

                    if (top_5.size() < 5) {
                        top_5.push({duration, entry.trace_id});
                    }
                    else if (duration > top_5.top().first) {
                        top_5.pop();
                        top_5.push({duration, entry.trace_id});
                    }

                    start_times.erase(it);
                }
            }
        }
    }

    std::vector<std::string> result;
    while (!top_5.empty()) {
        result.push_back(top_5.top().second);
        top_5.pop();
    }

    std::reverse(result.begin(), result.end());
    for (const auto& trace_id : result) {
        std::cout << trace_id << std::endl;
    }

    return 0;
}