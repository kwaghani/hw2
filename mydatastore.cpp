#include "mydatastore.h"
#include <iostream>
#include <set>
#include <algorithm>

using namespace std;

MyDataStore::MyDataStore() {}

MyDataStore::~MyDataStore()
{
    for (Product* p : products_) {
        delete p;
    }
    for (User* u : users_) {
        delete u;
    }
}

void MyDataStore::addProduct(Product* p)
{
    products_.push_back(p);
    std::set<std::string> keywords = p->keywords();
    for (const std::string& keyword : keywords) {
        keywordMap_[convToLower(keyword)].insert(p);
    }
}

void MyDataStore::addUser(User* u)
{
    users_.push_back(u);
}

std::vector<Product*> MyDataStore::search(std::vector<std::string>& terms, int type)
{
    std::set<Product*> resultSet;
    if (terms.empty()) return {};
    auto it = keywordMap_.find(convToLower(terms[0]));
    if (it != keywordMap_.end()) resultSet = it->second;
    for (size_t i = 1; i < terms.size(); ++i) {
        auto it = keywordMap_.find(convToLower(terms[i]));
        if (it != keywordMap_.end()) {
            resultSet = (type == 0) ? setIntersection(resultSet, it->second)
                                    : setUnion(resultSet, it->second);
        } else if (type == 0) {
            return {};
        }
    }
    return std::vector<Product*>(resultSet.begin(), resultSet.end());
}

void MyDataStore::addToCart(const std::string& username, Product* p) {
    std::string lowerUsername = convToLower(username);
    if (!userExists(lowerUsername)) {
        std::cout << "Invalid username" << std::endl;
        return;
    }
    userCarts_[lowerUsername].push_back(p);
}

void MyDataStore::viewCart(const std::string& username) {
    std::string lowerUsername = convToLower(username);
    auto it = userCarts_.find(lowerUsername);
    if (it == userCarts_.end()) {
        std::cout << "Invalid username" << std::endl;
        return;
    }
    std::vector<Product*>& cart = it->second;
    if (cart.empty()) {
        std::cout << "No items in cart." << std::endl;
        return;
    }
    int index = 1;
    for (Product* p : cart) {
        std::cout << "Item " << index++ << ":\n" << p->displayString() << "\n\n";
    }
}

void MyDataStore::buyCart(const std::string& username) {
    std::string lowerUsername = convToLower(username);
    auto it = userCarts_.find(lowerUsername);
    if (it == userCarts_.end() || it->second.empty()) {
        std::cout << "No items in cart." << std::endl;
        return;
    }
    std::vector<Product*>& cart = it->second;
    std::vector<Product*> remainingCart;
    auto userIt = std::find_if(users_.begin(), users_.end(),
        [&](User* u) { return convToLower(u->getName()) == lowerUsername; });
    if (userIt == users_.end()) {
        std::cout << "Invalid username" << std::endl;
        return;
    }
    User* user = *userIt;
    for (Product* p : cart) {
        if (p->getQty() > 0 && user->getBalance() >= p->getPrice()) {
            user->deductAmount(p->getPrice());
            p->subtractQty(1);
        } else {
            remainingCart.push_back(p);
        }
    }
    cart = remainingCart;
}

bool MyDataStore::userExists(const std::string& username) {
    std::string lowerUsername = convToLower(username);
    return std::any_of(users_.begin(), users_.end(),
        [&](User* u) { return convToLower(u->getName()) == lowerUsername; });
}

void MyDataStore::dump(std::ostream& ofile) {
    ofile << "<products>\n";
    for (Product* p : products_) {
        p->dump(ofile);
    }
    ofile << "</products>\n";
    ofile << "<users>\n";
    for (User* u : users_) {
        u->dump(ofile);
    }
    ofile << "</users>\n";
}
