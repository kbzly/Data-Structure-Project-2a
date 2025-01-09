// Project identifier: 0E04A31E0D60C01986ACB20081C9D8722A1899B6
#include <iostream>
#include <queue>
#include <vector>
#include <getopt.h>
#include <string>
#include "P2random.h"

using namespace std;

struct Deal
{
    int UID;
    int Trader_ID;
    int Price;
    int Quantity;
    Deal(int u, int t, int p, int q) : UID(u), Trader_ID(t), Price(p), Quantity(q) {}
};

struct Trader
{
    int net_transfer = 0;
    int bought = 0;
    int sold = 0;
};

struct SellerSort
{
    bool operator()(const Deal &d1, const Deal &d2) const {
        return d1.Price > d2.Price; // Min-heap: smallest price at the top
    }
};

struct BuyerSort
{
    bool operator()(const Deal &d1, const Deal &d2) const {
        return d1.Price < d2.Price; // Max-heap: largest price at the top
    }
};

struct UIDSort
{
    bool operator()(const Deal &d1, const Deal &d2) const {
        return d1.UID > d2.UID; // Min-heap: smallest UID at the top
    }
};

struct Travelers
{
    int price;
    int timestamp;
    Travelers() : price(0), timestamp(0) {}
};


enum TimeStatus {
    NoTrades,
    CanBuy,
    Completed,
    Potential
};

class Median {
public:  
    priority_queue<int> maxHeap; //smaller half
    priority_queue<int, vector<int>, greater<int>> minHeap;

    void addNumber(unsigned num) {
        if (maxHeap.empty() || num <= static_cast<unsigned int>(maxHeap.top())) {
            maxHeap.push(num);
        } else {
            minHeap.push(num);
        }

        if (maxHeap.size() > minHeap.size() + 1) {
            minHeap.push(maxHeap.top()); maxHeap.pop();
        } else if (minHeap.size() > maxHeap.size()) {
            maxHeap.push(minHeap.top()); minHeap.pop();
        }
    }

    int findMedian() {
        if (maxHeap.size() > minHeap.size()) {
            return maxHeap.top();
        } else {
            return (maxHeap.top() + minHeap.top()) / 2;
        }
    }

    void printMedian(int stock_ID, int timestamp) {
        if (!maxHeap.empty())
        {
            cout << "Median match price of Stock " << stock_ID << " at time " << timestamp << " is $" << findMedian() << '\n';
        }
    };

};

class Stock {
public:
    unsigned STOCK_ID;
    vector<Trader>& trader;
    vector<Median>& midian;
    priority_queue<Deal, std::vector<Deal>, BuyerSort> Buyers;
    priority_queue<Deal, std::vector<Deal>, SellerSort> Sellers;
    TimeStatus timeTravelers = NoTrades;
    Travelers minBuy;
    Travelers maxSell;
    Travelers potentialBuy;
    int dealmaked = 0;
    Stock(vector<Trader>& t, unsigned s, vector<Median>& m) : STOCK_ID{s}, trader(t), midian(m) {}


    void addBuyer(const Deal& deal) {
        Buyers.push(deal);
    }

    void addSeller(const Deal& deal) {
        Sellers.push(deal);
    }

    void deal(bool verbose_was_set, bool median_was_set, bool trader_info_was_set);
    void updataBuyer(unsigned trader_ID, unsigned price, unsigned quantity);
    void updataSeller(unsigned trader_ID, unsigned price, unsigned quantity);
    void updatamidian(unsigned price);
    void updataTimeTraveler(int price, string action, int timestamp);

};


static option long_options[] = {
    {"verbose",         no_argument,       nullptr, 'v'},
    {"median",          no_argument,       nullptr, 'm'},
    {"trader_info",     no_argument,       nullptr, 'i'},
    {"time_travelers",  no_argument,       nullptr, 't'},
    {nullptr,                     0,       nullptr,  0}
};

void printTraders(vector<Trader>& t);
void printTimeTraveler(vector<Stock>& s);
void errorReporter(int TIMESTAMP, int preTIMESTAMP, int TRADER_ID, int STOCK_ID, int PRICE, int QUANTITY, int NUM_TRADER, int NUM_STOCKS);

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);
    try
    {   
        bool verbose_was_set = false;
        bool median_was_set = false;
        bool trader_info_was_set = false;
        bool time_travelers_was_set = false;
        int choice = 0;
        while ((choice = getopt_long(argc, argv, "vmit", long_options, nullptr)) != -1) {
            switch (choice) {
            case 'v':
                verbose_was_set = true;
                break;
            case 'm':
                median_was_set = true;
                break;
            case 'i':
                trader_info_was_set = true;
                break;
            case 't':
                time_travelers_was_set = true;
                break;
            default:
                // unrecognized option
                throw "unrecognized option";
            }
        }

        string text, mode;
        getline(cin, text);
        cin >> text >> mode;

        int NUM_TRADER, NUM_STOCKS;
        cin >> text >> NUM_TRADER;
        cin >> text >> NUM_STOCKS;

        vector<Trader> trader;
        trader.resize(NUM_TRADER);
        vector<Median> median;
        median.resize(NUM_STOCKS);
        vector<Stock> stock;
        for (int i = 0; i < NUM_STOCKS; i++)
        {
            stock.push_back(Stock(trader, i, median));
        }

        int TIMESTAMP = 0;
        int preTIMESTAMP = 0;
        string ACTION;
        char C;
        int TRADER_ID;
        int STOCK_ID;
        int PRICE;
        int QUANTITY;
        int UID = 0;

        cout << "Processing orders...\n";
        if (mode == "TL") {
            while (cin >> TIMESTAMP >> ACTION >> C >> TRADER_ID >> C >> STOCK_ID >> C >> PRICE >> C >> QUANTITY)
            {   
                errorReporter(TIMESTAMP, preTIMESTAMP, TRADER_ID, STOCK_ID, PRICE, QUANTITY, NUM_TRADER, NUM_STOCKS);
                /////////////////////// Print Midian ///////////////////////////
                if (median_was_set && TIMESTAMP > preTIMESTAMP) {   
                    for (int i = 0; i < NUM_STOCKS; i++) {   
                        
                        median[i].printMedian(i, preTIMESTAMP);
                    }
                }
                preTIMESTAMP = TIMESTAMP;
                
                if (ACTION == "BUY") {
                    stock[STOCK_ID].addBuyer(Deal(UID++, TRADER_ID, PRICE, QUANTITY));
                    if (time_travelers_was_set) stock[STOCK_ID].updataTimeTraveler(PRICE, "BUY", TIMESTAMP);
                } else {
                    stock[STOCK_ID].addSeller(Deal(UID++, TRADER_ID, PRICE, QUANTITY));
                    if (time_travelers_was_set) stock[STOCK_ID].updataTimeTraveler(PRICE, "SELL", TIMESTAMP);
                }
                ///////////////////////////// Print verbose /////////////////////////////
                stock[STOCK_ID].deal(verbose_was_set, median_was_set, trader_info_was_set);
            }

            /////////////////////// Print Midian last term ///////////////////////////
            if (median_was_set) {
                for (int i = 0; i < NUM_STOCKS; i++) {
                        median[i].printMedian(i, preTIMESTAMP);
                    }
            }
        } else { // mode == "PR"
            stringstream ss;
            int RANDOM_SEED;
            int NUMBER_OF_ORDERS;
            int ARRIVAL_RATE;
            cin >> text >> RANDOM_SEED;
            cin >> text >> NUMBER_OF_ORDERS;
            cin >> text >> ARRIVAL_RATE;

            P2random::PR_init(ss, RANDOM_SEED, NUM_TRADER, NUM_STOCKS, NUMBER_OF_ORDERS, ARRIVAL_RATE);
            while (ss >> TIMESTAMP >> ACTION >> C >> TRADER_ID >> C >> STOCK_ID >> C >> PRICE >> C >> QUANTITY)
            {   
                // errorReporter(TIMESTAMP, preTIMESTAMP, TRADER_ID, STOCK_ID, PRICE, QUANTITY, NUM_TRADER, NUM_STOCKS);
                /////////////////////// Print Midian ///////////////////////////
                if (median_was_set && TIMESTAMP > preTIMESTAMP) {   
                    for (int i = 0; i < NUM_STOCKS; i++) {   
                        
                        median[i].printMedian(i, preTIMESTAMP);
                    }
                }
                preTIMESTAMP = TIMESTAMP;
                
                if (ACTION == "BUY") {
                    stock[STOCK_ID].addBuyer(Deal(UID++, TRADER_ID, PRICE, QUANTITY));
                    if (time_travelers_was_set) stock[STOCK_ID].updataTimeTraveler(PRICE, "BUY", TIMESTAMP);
                } else {
                    stock[STOCK_ID].addSeller(Deal(UID++, TRADER_ID, PRICE, QUANTITY));
                    if (time_travelers_was_set) stock[STOCK_ID].updataTimeTraveler(PRICE, "SELL", TIMESTAMP);
                }
                ///////////////////////////// Print verbose /////////////////////////////
                stock[STOCK_ID].deal(verbose_was_set, median_was_set, trader_info_was_set);
            }

            /////////////////////// Print Midian last term ///////////////////////////
            if (median_was_set) {
                for (int i = 0; i < NUM_STOCKS; i++) {
                        median[i].printMedian(i, preTIMESTAMP);
                    }
            }
            
        }
        

        cout << "---End of Day---\n";
        long int Trades_Completed = 0;
        for (int i = 0; i < NUM_STOCKS; i++)
        {
            Trades_Completed += stock[i].dealmaked;
        }
        cout << "Trades Completed: " << Trades_Completed << '\n';
        ///////////////////////// Print trader info //////////////////////////////////
        if (trader_info_was_set) printTraders(trader);
        /////////////////////////// Print time traveler //////////////////////////////
        if (time_travelers_was_set) printTimeTraveler(stock);

        

    }
    catch(const char* err)
    {
        std::cerr << err << std::endl; // print the error message to std::cerr
        return 1; // returning 1 from main indicates that an error occurred
    }
    
}

void Stock::deal(bool verbose_was_set, bool median_was_set, bool trader_info_was_set) {
        // deal until nothing to deal
        while (!Buyers.empty() && !Sellers.empty() && Buyers.top().Price >= Sellers.top().Price)
        {   
            // the same price deal sort with UID
            priority_queue<Deal, std::vector<Deal>, UIDSort> dealBuyers;
            priority_queue<Deal, std::vector<Deal>, UIDSort> dealSellers;

            dealBuyers.push(Buyers.top()); Buyers.pop();
            dealSellers.push(Sellers.top()); Sellers.pop();

            // select all deal with the same price
            while (!Buyers.empty() && Buyers.top().Price == dealBuyers.top().Price)
            {
                dealBuyers.push(Buyers.top()); Buyers.pop();
            } 
            while (!Sellers.empty() && Sellers.top().Price == dealSellers.top().Price)
            {
                dealSellers.push(Sellers.top()); Sellers.pop();
            } 

            while (!dealBuyers.empty() && !dealSellers.empty())
            {   
                Deal buy = dealBuyers.top(); dealBuyers.pop();
                Deal sell = dealSellers.top(); dealSellers.pop();
                unsigned deal_price = (buy.UID < sell.UID) ? buy.Price : sell.Price;
                if (buy.Quantity < sell.Quantity)
                {
                    if (verbose_was_set)
                    {
                        cout << "Trader " << buy.Trader_ID << " purchased " << buy.Quantity << " shares of Stock " << STOCK_ID << " from Trader " << sell.Trader_ID << " for $" << deal_price << "/share\n";
                    }

                    sell.Quantity = sell.Quantity - buy.Quantity;
                    dealSellers.push(sell);
                    if (trader_info_was_set)
                    {
                        updataBuyer(buy.Trader_ID, deal_price, buy.Quantity);
                        updataSeller(sell.Trader_ID, deal_price, buy.Quantity);
                    }
                } else if (buy.Quantity > sell.Quantity) {
                    if (verbose_was_set)
                    {
                        cout << "Trader " << buy.Trader_ID << " purchased " << sell.Quantity << " shares of Stock " << STOCK_ID << " from Trader " << sell.Trader_ID << " for $" << deal_price << "/share\n";
                    }
                    
                    buy.Quantity = buy.Quantity - sell.Quantity;
                    dealBuyers.push(buy);
                    if (trader_info_was_set)
                    {
                        updataBuyer(buy.Trader_ID, deal_price, sell.Quantity);
                        updataSeller(sell.Trader_ID, deal_price, sell.Quantity);
                    }
                } else { // buy.Quantity = sell.Quantity
                    if (verbose_was_set)
                    {
                        cout << "Trader " << buy.Trader_ID << " purchased " << buy.Quantity << " shares of Stock " << STOCK_ID << " from Trader " << sell.Trader_ID << " for $" << deal_price << "/share\n";
                    }
                    if (trader_info_was_set)
                    {
                        updataBuyer(buy.Trader_ID, deal_price, buy.Quantity);
                        updataSeller(sell.Trader_ID, deal_price, buy.Quantity);
                    }
                    // should be out of loop
                }
                if (median_was_set) updatamidian(deal_price);
                dealmaked++;
            }

            while (!dealBuyers.empty())
            {
                Buyers.push(dealBuyers.top()); dealBuyers.pop();
            }
            while (!dealSellers.empty())
            {
                Sellers.push(dealSellers.top()); dealSellers.pop();
            }

        }
    }

    void Stock::updataBuyer(unsigned trader_ID, unsigned price, unsigned quantity) {
        trader[trader_ID].bought += quantity;
        trader[trader_ID].net_transfer -= price * quantity;
    }

    void Stock::updataSeller(unsigned trader_ID, unsigned price, unsigned quantity) {
        trader[trader_ID].sold += quantity;
        trader[trader_ID].net_transfer += price * quantity;
    }

    void Stock::updatamidian(unsigned price) {
        midian[STOCK_ID].addNumber(price);
    }

    void Stock::updataTimeTraveler(int price, string action, int timestamp) {
        switch (timeTravelers)
        {
        case NoTrades:
            if (action == "SELL")
            {
                minBuy.price = price;
                minBuy.timestamp = timestamp;
                timeTravelers = CanBuy;
            }
            break;
        case CanBuy:
            if (action == "SELL" && price < minBuy.price)
            {
                minBuy.price = price;
                minBuy.timestamp = timestamp;
            } else if (action == "BUY" && price > minBuy.price) {
                maxSell.price = price;
                maxSell.timestamp = timestamp;
                timeTravelers = Completed;
            }
            break;
        case Completed:
            if (action == "BUY" && price > maxSell.price)
            {
                maxSell.price = price;
                maxSell.timestamp = timestamp;
            } else if (action == "SELL" && price < minBuy.price) {
                potentialBuy.price = price;
                potentialBuy.timestamp = timestamp;
                timeTravelers = Potential;
            }
            break;
        case Potential:
            if (action == "SELL" && price < potentialBuy.price) {
                potentialBuy.price = price;
                potentialBuy.timestamp = timestamp;
            } else if (action == "BUY" && price - potentialBuy.price > maxSell.price - minBuy.price) {
                minBuy = potentialBuy;
                maxSell.price = price;
                maxSell.timestamp = timestamp;
                timeTravelers = Completed;
            }
            break;
        default:
            break;
        }
    }

    void printTraders(vector<Trader>& t) {
        cout << "---Trader Info---\n";
        for (size_t i = 0; i < t.size(); i++)
        {
            cout << "Trader " << i << " bought " << t[i].bought <<" and sold " << t[i].sold << " for a net transfer of $" << t[i].net_transfer << '\n';
        }
    }

    void printTimeTraveler(vector<Stock>& s) {
        cout << "---Time Travelers---\n";
        for (size_t i = 0; i < s.size(); i++)
        {   
            if (s[i].timeTravelers == NoTrades || s[i].timeTravelers == CanBuy)
            {
                cout << "A time traveler could not make a profit on Stock " << i <<'\n';
            } else {
                cout << "A time traveler would buy Stock " << i << " at time " << s[i].minBuy.timestamp << " for $" << s[i].minBuy.price << " and sell it at time " << s[i].maxSell.timestamp << " for $" << s[i].maxSell.price << '\n';
            }
        }
        
    }

    void errorReporter(int TIMESTAMP, int preTIMESTAMP, int TRADER_ID, int STOCK_ID, int PRICE, int QUANTITY, int NUM_TRADER, int NUM_STOCKS){
        if (TIMESTAMP < 0)
        {
            throw "Negative timestamp";
        }
        if (TIMESTAMP < preTIMESTAMP)
        {
            throw "Decreasing timestamp";
        }
        if (TRADER_ID < 0 || TRADER_ID >= NUM_TRADER)
        {
            throw "Invalid trader ID";
        }
        if (STOCK_ID < 0 || STOCK_ID >= NUM_STOCKS)
        {
            throw "Invalid stock ID";
        }
        if (PRICE <= 0)
        {
            throw "Invalid price";
        }
        if (QUANTITY <= 0)
        {
            throw "Invalid quantity";
        }  
    }
    