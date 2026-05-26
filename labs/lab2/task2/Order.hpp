struct Order {
    int id;
    int priority; // чем меньше тем важнее

    bool operator<(const Order& other) const {
        return priority > other.priority; // min priority сверху
    }
};