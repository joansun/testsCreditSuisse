#include <cstdlib>
#include <string>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

vector<string> split(const string &str, const string &pattern) {
    //const char* convert to char*
    char * strc = new char[strlen(str.c_str()) + 1];
    strcpy(strc, str.c_str());
    vector<string> resultVec;
    char* tmpStr = strtok(strc, pattern.c_str());
    while (tmpStr != NULL) {
        resultVec.push_back(string(tmpStr));
        tmpStr = strtok(NULL, pattern.c_str());
    }

    delete[] strc;

    return resultVec;
}

template <class Type>
Type stringToNum(const string& str) {
    istringstream iss(str);
    Type num;
    iss >> num;
    return num;
}

struct Order {
    char action; // A, X, M
    int orderid;
    char side; // B, S 
    int quantity; 
    double price; 

    void print() const {
        cout << action << " " << orderid << " " << side << " " << quantity << " " << price << endl;
    }
};

struct Trade {
    int bOrderid;
    int sOrderid;
    int quantity; 
    double price; 

    void print() const {
        cout << bOrderid << " " << sOrderid << " " << quantity << " " << price << endl;
    }
};

class FeedHandler {
private:
    vector<Order> m_B_Order;
    vector<Order> m_S_Order;
    vector<Trade> m_Trade;

public:

    FeedHandler() {

    }

    void ProcessMessage(const std::string &line) {
        // A,100000,S,1,1075
        // A, X, M
        vector<string> info = split(line, ",");
        Order o;

        o.action = info[0][0];
        o.orderid = stringToNum<int>(info[1]);
        o.side = info[2][0];
        o.quantity = stringToNum<int>(info[3]);
        o.price = stringToNum<double>(info[4]);

        if (o.side == 'B') {
            if (o.action == 'A') {
                m_B_Order.push_back(o);
            } else if (o.action == 'X') {
                for (int i = 0; i < m_B_Order.size(); i++) {
                    if (m_B_Order[i].orderid == o.orderid) {
                        m_B_Order.erase(m_B_Order.begin() + i);
                        i--;
                    }
                }
            } else if (o.action == 'M') {
                for (int i = 0; i < m_B_Order.size(); i++) {
                    if (m_B_Order[i].orderid == o.orderid) {
                        m_B_Order[i].quantity = o.quantity;
                        m_B_Order[i].price = o.price;
                    }
                }
            }
        }
        if (o.side == 'S') {
            if (o.action == 'A') {
                m_S_Order.push_back(o);
            } else if (o.action == 'X') {
                for (int i = 0; i < m_S_Order.size(); i++) {
                    if (m_S_Order[i].orderid == o.orderid) {
                        m_S_Order.erase(m_S_Order.begin() + i);
                        i--;
                    }
                }
            } else if (o.action == 'M') {
                for (int i = 0; i < m_S_Order.size(); i++) {
                    if (m_S_Order[i].orderid == o.orderid) {
                        m_S_Order[i].quantity = o.quantity;
                        m_S_Order[i].price = o.price;
                    }
                }
            }
        }

        if (o.action == 'X' || o.action == 'M') {
            if (o.side == 'B') {
                for (int i = 0; i < m_Trade.size(); i++) {
                    if (m_Trade[i].bOrderid == o.orderid) {
                        for (int j = 0; j < m_S_Order.size(); j++) {
                            if (m_S_Order[j].orderid == o.orderid) {
                                m_S_Order[j].quantity += m_Trade[i].quantity;
                            }
                        }
                        for (int k = 0; k < m_B_Order.size(); k++) {
                            if (m_B_Order[k].orderid == o.orderid) {
                                m_B_Order[k].quantity += m_Trade[i].quantity;
                            }
                        }
                        m_Trade.erase(m_Trade.begin() + i);
                        i--;
                    }
                }
            }
        }
        if (o.action == 'X' || o.action == 'M') {
            if (o.side == 'S') {
                for (int i = 0; i < m_Trade.size(); i++) {
                    if (m_Trade[i].sOrderid == o.orderid) {
                        for (int j = 0; j < m_S_Order.size(); j++) {
                            if (m_S_Order[j].orderid == o.orderid) {
                                m_S_Order[j].quantity += m_Trade[i].quantity;
                            }
                        }
                        for (int k = 0; k < m_B_Order.size(); k++) {
                            if (m_B_Order[k].orderid == o.orderid) {
                                m_B_Order[k].quantity += m_Trade[i].quantity;
                            }
                        }
                        cout << "D T" << endl;
                        m_Trade[i].print();
                        cout << "D O" << endl;
                        o.print();
                        m_Trade.erase(m_Trade.begin() + i);
                        i--;
                    }
                }
            }
        }

        for (int i = 0; i < m_B_Order.size(); i++) {
            for (int j = 0; j < m_S_Order.size(); j++) {
                if (m_B_Order[i].price == m_S_Order[j].price) {
                    Trade t;
                    t.bOrderid = m_B_Order[i].orderid;
                    t.sOrderid = m_S_Order[j].orderid;
                    t.quantity = min(m_B_Order[i].quantity, m_S_Order[j].quantity);
                    t.price = m_B_Order[i].price;
                    m_Trade.push_back(t);

                    m_B_Order[i].quantity -= t.quantity;
                    m_S_Order[j].quantity -= t.quantity;
                    if (m_B_Order[i].quantity == 0) {
                        m_B_Order.erase(m_B_Order.begin() + i);
                        i--;
                    }
                    if (m_S_Order[j].quantity == 0) {
                        m_S_Order.erase(m_S_Order.begin() + j);
                        j--;
                    }
                }
            }
        }
    }

    void PrintCurrentOrderBook(std::ostream & os) const {
//        cout << "B:" << endl;
//        for (int i = 0; i < m_B_Order.size(); i++) {
//            m_B_Order[i].print();
//        }
//        cout << "S:" << endl;
//        for (int i = 0; i < m_S_Order.size(); i++) {
//            m_S_Order[i].print();
//        }
//        cout << "T:" << endl;
        for (int i = 0; i < m_Trade.size(); i++) {
            m_Trade[i].print();
        }
    }
};

/*
 * 
 */
int main(int argc, char** argv) {
    cout << argv[0] << " " << argv[1] << endl;
    FeedHandler feed;
    std::string line;
    const std::string filename(argv[1]);
    std::ifstream infile(filename, std::ios::in);
    int counter = 0;
    while (std::getline(infile, line)) {
        feed.ProcessMessage(line);
        if (++counter % 10 == 0) {
            feed.PrintCurrentOrderBook(std::cerr);
        }
    }
    feed.PrintCurrentOrderBook(std::cout);
    return 0;
}
/*
A,100000,S,1,1075
A,100001,B,9,1000
A,100002,B,30,975
A,100003,S,10,1050
A,100004,B,10,950
A,100005,S,2,1025
A,100006,B,1,1035
X,100004,B,10,950
A,100007,S,5,1035
A,100008,B,3,1050
X,100008,B,3,1050
X,100005,S,2,1025
M,100000,S,4,1075
 */