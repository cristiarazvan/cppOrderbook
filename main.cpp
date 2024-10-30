#include <iostream>


#include "orderbook.cpp"

void print_filled_order(pair<int, double> filledOrder, int quantity, u_int64_t startTime, u_int64_t endTime) {
    cout << "\033[33mFilled " << filledOrder.first << "/" << quantity << " units @ $" 
         << filledOrder.second / filledOrder.first << " average price. Time taken: " 
         << (endTime - startTime) << " nano seconds\033[0m" << "\n";
}

int main() {
    
 

  ifstream file("ascii.txt");
    string line;
    while (getline(file, line)) {
        cout << line << "\n";
    }

    
    Orderbook orderbook(true);

    while (true) {
        int userChoice;
        cout << "Options\n————————————————————\n|1. Print Orderbook|\n|2. Submit order   |\n|3. Exit           |\n ————————————————————\nChoice: ";
        cin >> userChoice;
        cout << "\n";

        if (userChoice == 1) {
            orderbook.print();
        } else if (userChoice == 2) {
            int orderTypeInput, quantityInput, sideInput;
            double priceInput;

            cout << "Enter order type:\n0. Market order\n1. Limit order\nSelection: ";
            cin >> orderTypeInput;
            OrderType orderType = static_cast<OrderType>(orderTypeInput);

            cout << "\nEnter side:\n0. Buy\n1. Sell\nSelection: ";
            cin >> sideInput;
            Side orderSide = static_cast<Side>(sideInput);

            cout << "\nEnter order quantity: ";
            cin >> quantityInput;

            if (orderType == OrderType::market) {
                cout << "\nSubmitting market " << ((orderSide == Side::buy) ? "buy" : "sell") 
                     << " order for " << quantityInput << " units..\n";

                u_int64_t startTime = unix_time();
                pair<int, double> filledOrder = orderbook.order_handle(orderType, quantityInput, orderSide);
                u_int64_t endTime = unix_time();

                print_filled_order(filledOrder, quantityInput, startTime, endTime);
            }
            cout << '\n';
        }
        else if (userChoice == 3) {
            break;
        }
    }

    return 0;
}