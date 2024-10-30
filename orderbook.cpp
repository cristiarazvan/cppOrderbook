// C
#ifndef _GLIBCXX_NO_ASSERT
#include <cassert>
#endif
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <ciso646>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#if __cplusplus >= 201103L
#include <ccomplex>
#include <cfenv>
#include <cinttypes>
#include <cstdbool>
#include <cstdint>
#include <ctgmath>
#include <cwchar>
#include <cwctype>
#endif

// C++
#include <algorithm>
#include <bitset>
#include <complex>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <typeinfo>
#include <utility>
#include <valarray>
#include <vector>

#if __cplusplus >= 201103L
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <forward_list>
#include <future>
#include <initializer_list>
#include <mutex>
#include <random>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <system_error>
#include <thread>
#include <tuple>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#endif

#define pb push_back

using namespace std;

enum class BookSide 
{
    bid,
    ask
};

enum class Side
{
    buy, 
    sell
};

enum class OrderType
{
    market,
    limit
};

inline uint64_t unix_time() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

struct Order
{
    int quantity;
    BookSide side;
    double price;
    uint64_t timestamp;
    
    Order(int quantity, double price, BookSide side, uint64_t timestamp = unix_time())
        : quantity(quantity), price(price), side(side), timestamp(timestamp) {}
};

class Orderbook
{
private:
    map<double, vector<unique_ptr<Order>>, less<double>> bids_;
    map<double, vector<unique_ptr<Order>>, greater<double>> asks_;

public:
    const auto& get_bids() { return bids_; }
    const auto& get_asks() { return asks_; }

    Orderbook(bool generate_fake_content)
    {
        srand(12); // For reproducibility 
    
        if (generate_fake_content) {
            // Generate some fake content
            for (int i = 0; i < 3; i++) { 
                double random_price = 90.0 + (rand() % 1001) / 100.0;
                int random_quantity1 = rand() % 100 + 1; 
                int random_quantity2 = rand() % 100 + 1;
                
                add_single_order(random_quantity1, random_price, BookSide::bid);
                this_thread::sleep_for(chrono::milliseconds(1)); 
                add_single_order(random_quantity2, random_price, BookSide::bid);
            }
            for (int i = 0; i < 3; i++) {
                double random_price = 100.0 + (rand() % 1001) / 100.0;
                int random_quantity1 = rand() % 100 + 1; 
                int random_quantity2 = rand() % 100 + 1; 			
                
                add_single_order(random_quantity1, random_price, BookSide::ask);
                this_thread::sleep_for(chrono::milliseconds(1)); 
                add_single_order(random_quantity2, random_price, BookSide::ask);
            }
        }
    }

    void add_single_order(int quantity, double price, BookSide side)
    {
        auto order = make_unique<Order>(quantity, price, side);
        if (side == BookSide::bid)
            bids_[price].pb(move(order));
        else 
            asks_[price].pb(move(order));
    }

    template<typename Comparator>
    void clean_level(map<double, vector<unique_ptr<Order>>, Comparator>& order_map)
    {
        for (auto it = order_map.begin(); it != order_map.end();) {
            if (it->second.empty())
                it = order_map.erase(it);
            else 
                ++it;
        }
    }

    pair<int, double> order_handle(OrderType type, int quantity, Side side, double price = 0)
    {
        int units_filled = 0;
        double total_value = 0;

        if (type == OrderType::market) {
            if (side == Side::buy)
                return order_fill(asks_, type, side, quantity, price, units_filled, total_value);
            else if (side == Side::sell)
                return order_fill(bids_, type, side, quantity, price, units_filled, total_value);
        } else if (type == OrderType::limit) {
            if (side == Side::buy) {
                if (best(BookSide::ask) <= price) {
                    // Partially filled
                    auto filled = order_fill(asks_, type, side, quantity, price, units_filled, total_value);
                    add_single_order(quantity, price, BookSide::bid);
                    return filled;
                } else {
                    // Impossible to fill
                    add_single_order(quantity, price, BookSide::bid);
                    return make_pair(units_filled, total_value);
                }
            } else {
                if (best(BookSide::bid) >= price) {
                    // Partially filled
                    auto filled = order_fill(bids_, type, side, quantity, price, units_filled, total_value);
                    add_single_order(quantity, price, BookSide::ask);
                    return filled;
                } else {
                    // Impossible to fill
                    add_single_order(quantity, price, BookSide::ask);
                    return make_pair(units_filled, total_value);
                }
            }
        } else {
            throw runtime_error("Invalid order encountered");
        }
    }

    template<typename Comparator>
    pair<int, double> order_fill(map<double, vector<unique_ptr<Order>>, Comparator>& offers, OrderType type, Side side, int& order_quantity, double price, int& units_filled, double& total_value)
    {
        for (auto rit = offers.rbegin(); rit != offers.rend();) {
            double price_level = rit->first;
            auto& offer_list = rit->second;

            bool can_trade = true;
            if (type == OrderType::limit) {
                if (side == Side::buy && price_level > price)
                    can_trade = false;
                else if (side == Side::sell && price_level < price)
                    can_trade = false;
            }
            if (can_trade) {
                auto it = offer_list.begin();
                while (it != offer_list.end() && order_quantity > 0) {
                    int& current_offer_quantity = (*it)->quantity;
                    const double current_offer_price = (*it)->price;

                    if (current_offer_quantity > order_quantity) {
                        units_filled += order_quantity;
                        total_value += order_quantity * current_offer_price;

                        current_offer_quantity -= order_quantity;
                        order_quantity = 0;
                        break; // Order Completely Filled
                    } else {
                        units_filled += current_offer_quantity;
                        total_value += current_offer_quantity * current_offer_price;

                        order_quantity -= current_offer_quantity;
                        it = offer_list.erase(it);
                    }
                }
            }
            ++rit;
        }

        clean_level(bids_);
        clean_level(asks_);

        return make_pair(units_filled, total_value);
    }

    double best(BookSide side)
    {
        if (side == BookSide::bid)
            return prev(bids_.end())->first;
        else if (side == BookSide::ask)
            return next(asks_.begin())->first;
        else 
            return 0.00;
    }

    template <typename Comparator>
    void print_single_leg(map<double, vector<unique_ptr<Order>>, Comparator>& order_map, BookSide side)
    {
        if (side == BookSide::ask) {
            for (const auto& [price, orders] : order_map) {
                int total_quantity = 0;
                for (const auto& order : orders)
                    total_quantity += order->quantity;
                
                string color = "31";
                cout << "\t\033[1;" << color << "m" << "$" << setw(6) << fixed << setprecision(2) 
                     << price << setw(5) << total_quantity << "\033[0m ";
                for (int i = 0; i < total_quantity / 10; i++)
                    cout << "█";
                cout << "\n";
            }
        } else if (side == BookSide::bid) {
            for (auto rit = order_map.rbegin(); rit != order_map.rend(); ++rit) {
                int total_quantity = 0;
                for (const auto& order : rit->second)
                    total_quantity += order->quantity;
                
                string color = "32";
                cout << "\t\033[1;" << color << "m" << "$" << setw(6) << fixed << setprecision(2)
                     << rit->first << setw(5) << total_quantity << "\033[0m ";

                for (int i = 0; i < total_quantity / 10; i++)
                    cout << "█";
                cout << "\n";
            }
        }
    }

    void print()
    {
        cout << "--------------Orderbook-----------------\n";
        print_single_leg(asks_, BookSide::ask);
        cout << "===================================\n";
        print_single_leg(bids_, BookSide::bid);
        cout << "===================================\n";
        cout << "Best Bid: $" << best(BookSide::bid) << "\n";
        cout << "Best Ask: $" << best(BookSide::ask) << "\n";
        cout << "===================================\n";
    }
};
