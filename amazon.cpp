#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include "product.h"
#include "db_parser.h"
#include "product_parser.h"
#include "util.h"
#include "mydatastore.h"

using namespace std;

struct ProdNameSorter {
    bool operator()(Product* p1, Product* p2) {
        return (p1->getName() < p2->getName());
    }
};

void displayProducts(vector<Product*>& hits);

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "Please specify a database file" << endl;
        return 1;
    }

    // Create a MyDataStore instance
    MyDataStore ds;

    // Instantiate the individual section and product parsers we want
    ProductSectionParser* productSectionParser = new ProductSectionParser;
    productSectionParser->addProductParser(new ProductBookParser);
    productSectionParser->addProductParser(new ProductClothingParser);
    productSectionParser->addProductParser(new ProductMovieParser);
    UserSectionParser* userSectionParser = new UserSectionParser;

    // Instantiate the parser and add section parsers
    DBParser parser;
    parser.addSectionParser("products", productSectionParser);
    parser.addSectionParser("users", userSectionParser);

    // Parse the database to populate the DataStore
    if (parser.parse(argv[1], ds)) {
        cerr << "Error parsing!" << endl;
        return 1;
    }

    cout << "=====================================" << endl;
    cout << "Menu: " << endl;
    cout << "  AND term term ...                  " << endl;
    cout << "  OR term term ...                   " << endl;
    cout << "  ADD username search_hit_number     " << endl;
    cout << "  VIEWCART username                  " << endl;
    cout << "  BUYCART username                   " << endl;
    cout << "  QUIT new_db_filename               " << endl;
    cout << "====================================" << endl;

    vector<Product*> hits;
    bool done = false;
    while (!done) {
        cout << "\nEnter command: " << endl;
        string line;
        getline(cin, line);
        stringstream ss(line);
        string cmd;
        if (ss >> cmd) {
            if (cmd == "AND") {
                string term;
                vector<string> terms;
                while (ss >> term) {
                    term = convToLower(term);
                    terms.push_back(term);
                }
                hits = ds.search(terms, 0);
                displayProducts(hits);
            }
            else if (cmd == "OR") {
                string term;
                vector<string> terms;
                while (ss >> term) {
                    term = convToLower(term);
                    terms.push_back(term);
                }
                hits = ds.search(terms, 1);
                displayProducts(hits);
            }
            else if (cmd == "QUIT") {
                string filename;
                if (ss >> filename) {
                    ofstream ofile(filename.c_str());
                    ds.dump(ofile);
                    ofile.close();
                }
                done = true;
            }
            else if (cmd == "ADD") {
                std::string username;
                int hitIndex;
                if (!(ss >> username >> hitIndex)) {
                    cout << "Invalid request" << endl;
                    cout.flush();
                } else {
                    std::string lowerUsername = convToLower(username);
                    if (!ds.userExists(lowerUsername)) {
                        cout << "Invalid username" << endl;
                        cout.flush();
                    } else if (hitIndex < 1 || hitIndex > (int)hits.size()) {
                        cout << "Invalid request" << endl;
                        cout.flush();
                    } else {
                        ds.addToCart(lowerUsername, hits[hitIndex - 1]);
                        cout << "Added to cart: " << hits[hitIndex - 1]->getName() << endl;
                        cout.flush();
                    }
                }
            }
            else if (cmd == "VIEWCART") {
                std::string username;
                if (ss >> username) {
                    if (ds.userExists(username)) {
                        ds.viewCart(username);
                    } else {
                        cout << "Invalid username" << endl;
                        cout.flush();
                    }
                } else {
                    cout << "Invalid request" << endl;
                    cout.flush();
                }
            }
            else if (cmd == "BUYCART") {
                std::string username;
                if (ss >> username) {
                    if (ds.userExists(username)) {
                        ds.buyCart(username);
                    } else {
                        cout << "Invalid username" << endl;
                        cout.flush();
                    }
                } else {
                    cout << "Invalid request" << endl;
                    cout.flush();
                }
            }
            else {
                cout << "Unknown command" << endl;
                cout.flush();
            }
        }
    }
    return 0;
}

void displayProducts(vector<Product*>& hits)
{
    int resultNo = 1;
    if (hits.empty()) {
        cout << "No results found!" << endl;
        return;
    }
    sort(hits.begin(), hits.end(), ProdNameSorter());
    for (size_t i = 0; i < hits.size(); i++) {
        cout << "Hit " << setw(3) << i + 1 << endl;
        cout << hits[i]->displayString() << endl;
        cout << endl;
    }
}
