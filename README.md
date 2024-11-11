# C++ Order Book

## Project Overview
This project is a high-performance Order Book implemented in C++. The Order Book is designed to handle real-time financial data efficiently, supporting key order book operations including order matching, price level tracking, and order prioritization. This project demonstrates my understanding of fundamental data structures, algorithms, and my ability to implement these concepts in a performant, scalable way.

## Features
- **Order Management**: Supports placing orders with low latency.
- **Price Level Aggregation**: Groups orders by price level, providing efficient querying of price and volume data.
- **Matching Engine**: Automatically matches incoming orders with existing orders based on predefined rules (price-time priority).
- **Performance Optimization**: Leveraged C++ standard libraries and custom data structures for minimal latency.

## Technologies Used
- **Language**: C++
- **Libraries**: Standard Template Library (STL)
## Installation
To clone and run the project locally, ensure you have `git` and `g++` installed.

```bash
# Clone the repository
git clone https://github.com/cristiarazvan/CppOrderbook.git

# Navigate to the project directory
cd CppOrderbook

# Compile the project
g++ -std=c++14 main.cpp -o main 

# Run the executable
./main
