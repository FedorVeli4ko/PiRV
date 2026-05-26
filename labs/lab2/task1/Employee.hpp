struct Employee {
    int id;
    bool high_priority;

    bool operator<(const Employee& other) const {
        return high_priority < other.high_priority; // VIP вперед
    }
};