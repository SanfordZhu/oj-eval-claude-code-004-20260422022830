#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstring>

using namespace std;

// Constants
const string ROOT_USERNAME = "root";
const string ROOT_PASSWORD = "sjtu";
const int ROOT_PRIVILEGE = 7;

// Data structures
struct User {
    string userID;
    string password;
    string username;
    int privilege;

    User(string id = "", string pwd = "", string name = "", int priv = 1)
        : userID(id), password(pwd), username(name), privilege(priv) {}
};

struct Book {
    string ISBN;
    string bookName;
    string author;
    string keyword;
    double price;
    int stockQuantity;

    Book(string isbn = "", string name = "", string auth = "", string kw = "", double p = 0.0, int qty = 0)
        : ISBN(isbn), bookName(name), author(auth), keyword(kw), price(p), stockQuantity(qty) {}
};

// Global variables
vector<User> users;
vector<Book> books;
vector<string> loginStack; // Stores userIDs in login stack
map<string, string> selectedBooks; // userID -> selected ISBN

// File names for persistence
const string USER_FILE = "users.dat";
const string BOOK_FILE = "books.dat";
const string FINANCE_FILE = "finance.dat";

// Function declarations
void initializeSystem();
void loadData();
void saveData();
void processCommand(const string& cmd);
bool isValidUserID(const string& id);
bool isValidPassword(const string& pwd);
bool isValidUsername(const string& name);
bool isValidPrivilege(int priv);
bool isValidISBN(const string& isbn);
bool isValidBookName(const string& name);
bool isValidAuthor(const string& auth);
bool isValidKeyword(const string& kw);
bool isValidPrice(const string& priceStr);
bool isValidQuantity(const string& qtyStr);
bool isValidTotalCost(const string& costStr);
User* findUser(const string& userID);
Book* findBook(const string& isbn);
int getCurrentPrivilege();
string getCurrentUserID();
void executeSu(const vector<string>& tokens);
void executeLogout();
void executeRegister(const vector<string>& tokens);
void executePasswd(const vector<string>& tokens);
void executeUseradd(const vector<string>& tokens);
void executeDelete(const vector<string>& tokens);
void executeShow(const vector<string>& tokens);
void executeBuy(const vector<string>& tokens);
void executeSelect(const vector<string>& tokens);
void executeModify(const vector<string>& tokens);
void executeImport(const vector<string>& tokens);
void executeShowFinance(const vector<string>& tokens);
void executeLog();
void executeReportFinance();
void executeReportEmployee();

int main() {
    initializeSystem();
    loadData();

    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;
        processCommand(line);
    }

    saveData();
    return 0;
}

void initializeSystem() {
    // Check if this is first run by checking if user file exists
    ifstream userFile(USER_FILE);
    if (!userFile.good()) {
        // First run - create root user
        users.push_back(User(ROOT_USERNAME, ROOT_PASSWORD, "superadmin", ROOT_PRIVILEGE));
        saveData();

        // Create empty finance file
        ofstream financeFile(FINANCE_FILE, ios::binary);
        if (financeFile) {
            size_t recordCount = 0;
            financeFile.write(reinterpret_cast<const char*>(&recordCount), sizeof(recordCount));
        }
    }
}

void loadData() {
    // Load users
    ifstream userFile(USER_FILE, ios::binary);
    if (userFile) {
        size_t userCount;
        userFile.read(reinterpret_cast<char*>(&userCount), sizeof(userCount));
        for (size_t i = 0; i < userCount; i++) {
            User user;
            size_t len;

            // Read userID
            userFile.read(reinterpret_cast<char*>(&len), sizeof(len));
            user.userID.resize(len);
            userFile.read(&user.userID[0], len);

            // Read password
            userFile.read(reinterpret_cast<char*>(&len), sizeof(len));
            user.password.resize(len);
            userFile.read(&user.password[0], len);

            // Read username
            userFile.read(reinterpret_cast<char*>(&len), sizeof(len));
            user.username.resize(len);
            userFile.read(&user.username[0], len);

            // Read privilege
            userFile.read(reinterpret_cast<char*>(&user.privilege), sizeof(user.privilege));

            users.push_back(user);
        }
    }

    // Load books
    ifstream bookFile(BOOK_FILE, ios::binary);
    if (bookFile) {
        size_t bookCount;
        bookFile.read(reinterpret_cast<char*>(&bookCount), sizeof(bookCount));
        for (size_t i = 0; i < bookCount; i++) {
            Book book;
            size_t len;

            // Read ISBN
            bookFile.read(reinterpret_cast<char*>(&len), sizeof(len));
            book.ISBN.resize(len);
            bookFile.read(&book.ISBN[0], len);

            // Read bookName
            bookFile.read(reinterpret_cast<char*>(&len), sizeof(len));
            book.bookName.resize(len);
            bookFile.read(&book.bookName[0], len);

            // Read author
            bookFile.read(reinterpret_cast<char*>(&len), sizeof(len));
            book.author.resize(len);
            bookFile.read(&book.author[0], len);

            // Read keyword
            bookFile.read(reinterpret_cast<char*>(&len), sizeof(len));
            book.keyword.resize(len);
            bookFile.read(&book.keyword[0], len);

            // Read price
            bookFile.read(reinterpret_cast<char*>(&book.price), sizeof(book.price));

            // Read stockQuantity
            bookFile.read(reinterpret_cast<char*>(&book.stockQuantity), sizeof(book.stockQuantity));

            books.push_back(book);
        }
    }
}

void saveData() {
    // Save users
    ofstream userFile(USER_FILE, ios::binary);
    if (userFile) {
        size_t userCount = users.size();
        userFile.write(reinterpret_cast<const char*>(&userCount), sizeof(userCount));
        for (const auto& user : users) {
            // Write userID
            size_t len = user.userID.size();
            userFile.write(reinterpret_cast<const char*>(&len), sizeof(len));
            userFile.write(user.userID.c_str(), len);

            // Write password
            len = user.password.size();
            userFile.write(reinterpret_cast<const char*>(&len), sizeof(len));
            userFile.write(user.password.c_str(), len);

            // Write username
            len = user.username.size();
            userFile.write(reinterpret_cast<const char*>(&len), sizeof(len));
            userFile.write(user.username.c_str(), len);

            // Write privilege
            userFile.write(reinterpret_cast<const char*>(&user.privilege), sizeof(user.privilege));
        }
    }

    // Save books
    ofstream bookFile(BOOK_FILE, ios::binary);
    if (bookFile) {
        size_t bookCount = books.size();
        bookFile.write(reinterpret_cast<const char*>(&bookCount), sizeof(bookCount));
        for (const auto& book : books) {
            // Write ISBN
            size_t len = book.ISBN.size();
            bookFile.write(reinterpret_cast<const char*>(&len), sizeof(len));
            bookFile.write(book.ISBN.c_str(), len);

            // Write bookName
            len = book.bookName.size();
            bookFile.write(reinterpret_cast<const char*>(&len), sizeof(len));
            bookFile.write(book.bookName.c_str(), len);

            // Write author
            len = book.author.size();
            bookFile.write(reinterpret_cast<const char*>(&len), sizeof(len));
            bookFile.write(book.author.c_str(), len);

            // Write keyword
            len = book.keyword.size();
            bookFile.write(reinterpret_cast<const char*>(&len), sizeof(len));
            bookFile.write(book.keyword.c_str(), len);

            // Write price
            bookFile.write(reinterpret_cast<const char*>(&book.price), sizeof(book.price));

            // Write stockQuantity
            bookFile.write(reinterpret_cast<const char*>(&book.stockQuantity), sizeof(book.stockQuantity));
        }
    }
}

void processCommand(const string& cmd) {
    // Trim leading/trailing spaces
    string trimmed = cmd;
    size_t start = trimmed.find_first_not_of(" ");
    size_t end = trimmed.find_last_not_of(" ");
    if (start != string::npos && end != string::npos) {
        trimmed = trimmed.substr(start, end - start + 1);
    } else {
        trimmed = "";
    }

    if (trimmed.empty()) {
        // Empty command (only spaces) is legal
        return;
    }

    // Tokenize command
    vector<string> tokens;
    stringstream ss(trimmed);
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        cout << "Invalid\n";
        return;
    }

    string command = tokens[0];

    if (command == "quit" || command == "exit") {
        // Exit system
        exit(0);
    } else if (command == "su") {
        executeSu(tokens);
    } else if (command == "logout") {
        executeLogout();
    } else if (command == "register") {
        executeRegister(tokens);
    } else if (command == "passwd") {
        executePasswd(tokens);
    } else if (command == "useradd") {
        executeUseradd(tokens);
    } else if (command == "delete") {
        executeDelete(tokens);
    } else if (command == "show") {
        if (tokens.size() > 1 && tokens[1] == "finance") {
            executeShowFinance(tokens);
        } else {
            executeShow(tokens);
        }
    } else if (command == "buy") {
        executeBuy(tokens);
    } else if (command == "select") {
        executeSelect(tokens);
    } else if (command == "modify") {
        executeModify(tokens);
    } else if (command == "import") {
        executeImport(tokens);
    } else if (command == "log") {
        executeLog();
    } else if (command == "report") {
        if (tokens.size() > 1) {
            if (tokens[1] == "finance") {
                executeReportFinance();
            } else if (tokens[1] == "employee") {
                executeReportEmployee();
            } else {
                cout << "Invalid\n";
            }
        } else {
            cout << "Invalid\n";
        }
    } else {
        cout << "Invalid\n";
    }
}

// Validation functions
bool isValidUserID(const string& id) {
    if (id.empty() || id.size() > 30) return false;
    for (char c : id) {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

bool isValidPassword(const string& pwd) {
    if (pwd.empty() || pwd.size() > 30) return false;
    for (char c : pwd) {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

bool isValidUsername(const string& name) {
    if (name.empty() || name.size() > 30) return false;
    for (char c : name) {
        if (c < 32 || c > 126) return false; // ASCII printable characters only
    }
    return true;
}

bool isValidPrivilege(int priv) {
    return priv == 1 || priv == 3 || priv == 7;
}

bool isValidISBN(const string& isbn) {
    if (isbn.empty() || isbn.size() > 20) return false;
    for (char c : isbn) {
        if (c < 32 || c > 126) return false; // ASCII printable characters except invisible
    }
    return true;
}

bool isValidBookName(const string& name) {
    if (name.empty() || name.size() > 60) return false;
    for (char c : name) {
        if (c < 32 || c > 126 || c == '\"') return false;
    }
    return true;
}

bool isValidAuthor(const string& auth) {
    if (auth.empty() || auth.size() > 60) return false;
    for (char c : auth) {
        if (c < 32 || c > 126 || c == '\"') return false;
    }
    return true;
}

bool isValidKeyword(const string& kw) {
    if (kw.empty() || kw.size() > 60) return false;
    for (char c : kw) {
        if (c < 32 || c > 126 || c == '\"') return false;
    }
    return true;
}

bool isValidPrice(const string& priceStr) {
    // Check if it's a valid floating point number
    if (priceStr.empty() || priceStr.size() > 13) return false;

    bool dotSeen = false;
    for (char c : priceStr) {
        if (c == '.') {
            if (dotSeen) return false;
            dotSeen = true;
        } else if (!isdigit(c)) {
            return false;
        }
    }

    // Check if value is positive
    try {
        double price = stod(priceStr);
        if (price < 0) return false;
    } catch (...) {
        return false;
    }

    return true;
}

bool isValidQuantity(const string& qtyStr) {
    if (qtyStr.empty() || qtyStr.size() > 10) return false;
    for (char c : qtyStr) {
        if (!isdigit(c)) return false;
    }

    try {
        long long qty = stoll(qtyStr);
        if (qty <= 0 || qty > 2147483647) return false;
    } catch (...) {
        return false;
    }

    return true;
}

bool isValidTotalCost(const string& costStr) {
    return isValidPrice(costStr); // Same validation as price
}

User* findUser(const string& userID) {
    for (auto& user : users) {
        if (user.userID == userID) {
            return &user;
        }
    }
    return nullptr;
}

Book* findBook(const string& isbn) {
    for (auto& book : books) {
        if (book.ISBN == isbn) {
            return &book;
        }
    }
    return nullptr;
}

int getCurrentPrivilege() {
    if (loginStack.empty()) {
        return 0;
    }
    string currentUserID = loginStack.back();
    User* user = findUser(currentUserID);
    if (user) {
        return user->privilege;
    }
    return 0;
}

string getCurrentUserID() {
    if (loginStack.empty()) {
        return "";
    }
    return loginStack.back();
}

// Command execution functions
void executeSu(const vector<string>& tokens) {
    // su [UserID] ([Password])?
    if (tokens.size() < 2 || tokens.size() > 3) {
        cout << "Invalid\n";
        return;
    }

    string userID = tokens[1];
    if (!isValidUserID(userID)) {
        cout << "Invalid\n";
        return;
    }

    User* user = findUser(userID);
    if (!user) {
        cout << "Invalid\n";
        return;
    }

    if (tokens.size() == 3) {
        // Password provided
        string password = tokens[2];
        if (!isValidPassword(password)) {
            cout << "Invalid\n";
            return;
        }

        if (user->password != password) {
            cout << "Invalid\n";
            return;
        }
    } else {
        // No password provided - check if current privilege is higher
        int currentPrivilege = getCurrentPrivilege();
        if (currentPrivilege <= user->privilege) {
            cout << "Invalid\n";
            return;
        }
    }

    // Login successful
    loginStack.push_back(userID);
    // Clear selected book for this user (if any)
    selectedBooks.erase(userID);
}

void executeLogout() {
    if (loginStack.empty()) {
        cout << "Invalid\n";
        return;
    }

    loginStack.pop_back();
}

void executeRegister(const vector<string>& tokens) {
    // register [UserID] [Password] [Username]
    if (tokens.size() != 4) {
        cout << "Invalid\n";
        return;
    }

    string userID = tokens[1];
    string password = tokens[2];
    string username = tokens[3];

    if (!isValidUserID(userID) || !isValidPassword(password) || !isValidUsername(username)) {
        cout << "Invalid\n";
        return;
    }

    if (findUser(userID)) {
        cout << "Invalid\n";
        return;
    }

    // Create new user with privilege 1
    users.push_back(User(userID, password, username, 1));
    cout << "\n"; // Successful registration outputs empty line
}

void executePasswd(const vector<string>& tokens) {
    // passwd [UserID] ([CurrentPassword])? [NewPassword]
    if (tokens.size() < 3 || tokens.size() > 4) {
        cout << "Invalid\n";
        return;
    }

    string userID = tokens[1];
    if (!isValidUserID(userID)) {
        cout << "Invalid\n";
        return;
    }

    User* user = findUser(userID);
    if (!user) {
        cout << "Invalid\n";
        return;
    }

    int currentPrivilege = getCurrentPrivilege();

    if (tokens.size() == 4) {
        // Format: passwd [UserID] [CurrentPassword] [NewPassword]
        string currentPassword = tokens[2];
        string newPassword = tokens[3];

        if (!isValidPassword(currentPassword) || !isValidPassword(newPassword)) {
            cout << "Invalid\n";
            return;
        }

        if (user->password != currentPassword) {
            cout << "Invalid\n";
            return;
        }

        user->password = newPassword;
    } else if (tokens.size() == 3) {
        // Format: passwd [UserID] [NewPassword] (only allowed for privilege 7)
        string newPassword = tokens[2];

        if (!isValidPassword(newPassword)) {
            cout << "Invalid\n";
            return;
        }

        if (currentPrivilege != 7) {
            cout << "Invalid\n";
            return;
        }

        user->password = newPassword;
    }

    cout << "\n"; // Successful password change outputs empty line
}

void executeUseradd(const vector<string>& tokens) {
    // useradd [UserID] [Password] [Privilege] [Username]
    if (tokens.size() != 5) {
        cout << "Invalid\n";
        return;
    }

    string userID = tokens[1];
    string password = tokens[2];
    string privilegeStr = tokens[3];
    string username = tokens[4];

    if (!isValidUserID(userID) || !isValidPassword(password) || !isValidUsername(username)) {
        cout << "Invalid\n";
        return;
    }

    // Validate privilege
    if (privilegeStr.size() != 1 || !isdigit(privilegeStr[0])) {
        cout << "Invalid\n";
        return;
    }

    int privilege = privilegeStr[0] - '0';
    if (!isValidPrivilege(privilege)) {
        cout << "Invalid\n";
        return;
    }

    // Check current privilege
    int currentPrivilege = getCurrentPrivilege();
    if (currentPrivilege < 3) {
        cout << "Invalid\n";
        return;
    }

    // Check if privilege to create is >= current privilege
    if (privilege >= currentPrivilege) {
        cout << "Invalid\n";
        return;
    }

    // Check if user already exists
    if (findUser(userID)) {
        cout << "Invalid\n";
        return;
    }

    // Create new user
    users.push_back(User(userID, password, username, privilege));
    cout << "\n"; // Successful useradd outputs empty line
}

void executeDelete(const vector<string>& tokens) {
    // delete [UserID]
    if (tokens.size() != 2) {
        cout << "Invalid\n";
        return;
    }

    string userID = tokens[1];
    if (!isValidUserID(userID)) {
        cout << "Invalid\n";
        return;
    }

    // Check current privilege
    int currentPrivilege = getCurrentPrivilege();
    if (currentPrivilege != 7) {
        cout << "Invalid\n";
        return;
    }

    // Find user
    User* user = findUser(userID);
    if (!user) {
        cout << "Invalid\n";
        return;
    }

    // Check if user is logged in
    for (const auto& loggedInUser : loginStack) {
        if (loggedInUser == userID) {
            cout << "Invalid\n";
            return;
        }
    }

    // Delete user
    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it->userID == userID) {
            users.erase(it);
            cout << "\n"; // Successful delete outputs empty line
            return;
        }
    }

    cout << "Invalid\n";
}

void executeShow(const vector<string>& tokens) {
    // show (-ISBN=[ISBN] | -name="[BookName]" | -author="[Author]" | -keyword="[Keyword]")?
    // Check privilege
    if (getCurrentPrivilege() < 1) {
        cout << "Invalid\n";
        return;
    }

    vector<Book*> matchingBooks;

    if (tokens.size() == 1) {
        // No filter - show all books
        for (auto& book : books) {
            matchingBooks.push_back(&book);
        }
    } else if (tokens.size() == 2) {
        string filter = tokens[1];

        if (filter.find("-ISBN=") == 0) {
            string isbn = filter.substr(6);
            if (!isValidISBN(isbn) || isbn.empty()) {
                cout << "Invalid\n";
                return;
            }

            for (auto& book : books) {
                if (book.ISBN == isbn) {
                    matchingBooks.push_back(&book);
                }
            }
        } else if (filter.find("-name=\"") == 0 && filter.back() == '\"') {
            string name = filter.substr(7, filter.size() - 8);
            if (!isValidBookName(name) || name.empty()) {
                cout << "Invalid\n";
                return;
            }

            for (auto& book : books) {
                if (book.bookName == name) {
                    matchingBooks.push_back(&book);
                }
            }
        } else if (filter.find("-author=\"") == 0 && filter.back() == '\"') {
            string author = filter.substr(9, filter.size() - 10);
            if (!isValidAuthor(author) || author.empty()) {
                cout << "Invalid\n";
                return;
            }

            for (auto& book : books) {
                if (book.author == author) {
                    matchingBooks.push_back(&book);
                }
            }
        } else if (filter.find("-keyword=\"") == 0 && filter.back() == '\"') {
            string keyword = filter.substr(10, filter.size() - 11);
            if (!isValidKeyword(keyword) || keyword.empty()) {
                cout << "Invalid\n";
                return;
            }

            // Check if keyword contains multiple keywords (contains '|')
            if (keyword.find('|') != string::npos) {
                cout << "Invalid\n";
                return;
            }

            for (auto& book : books) {
                if (book.keyword.find(keyword) != string::npos) {
                    matchingBooks.push_back(&book);
                }
            }
        } else {
            cout << "Invalid\n";
            return;
        }
    } else {
        cout << "Invalid\n";
        return;
    }

    // Sort by ISBN
    sort(matchingBooks.begin(), matchingBooks.end(), [](Book* a, Book* b) {
        return a->ISBN < b->ISBN;
    });

    // Output books
    for (auto book : matchingBooks) {
        cout << book->ISBN << "\t"
             << book->bookName << "\t"
             << book->author << "\t"
             << book->keyword << "\t"
             << fixed << setprecision(2) << book->price << "\t"
             << book->stockQuantity << "\n";
    }

    if (matchingBooks.empty()) {
        cout << "\n"; // Empty line when no books
    }
}

void addFinanceRecord(double income, double expenditure) {
    // Read current records
    vector<pair<double, double>> records;
    ifstream financeFile(FINANCE_FILE, ios::binary);
    if (financeFile) {
        size_t recordCount;
        financeFile.read(reinterpret_cast<char*>(&recordCount), sizeof(recordCount));
        for (size_t i = 0; i < recordCount; i++) {
            double inc, exp;
            financeFile.read(reinterpret_cast<char*>(&inc), sizeof(inc));
            financeFile.read(reinterpret_cast<char*>(&exp), sizeof(exp));
            records.push_back({inc, exp});
        }
    }

    // Add new record
    records.push_back({income, expenditure});

    // Write back
    ofstream financeFileOut(FINANCE_FILE, ios::binary);
    if (financeFileOut) {
        size_t recordCount = records.size();
        financeFileOut.write(reinterpret_cast<const char*>(&recordCount), sizeof(recordCount));
        for (const auto& record : records) {
            financeFileOut.write(reinterpret_cast<const char*>(&record.first), sizeof(record.first));
            financeFileOut.write(reinterpret_cast<const char*>(&record.second), sizeof(record.second));
        }
    }
}

void executeBuy(const vector<string>& tokens) {
    // buy [ISBN] [Quantity]
    if (tokens.size() != 3) {
        cout << "Invalid\n";
        return;
    }

    // Check privilege
    if (getCurrentPrivilege() < 1) {
        cout << "Invalid\n";
        return;
    }

    string isbn = tokens[1];
    string quantityStr = tokens[2];

    if (!isValidISBN(isbn) || !isValidQuantity(quantityStr)) {
        cout << "Invalid\n";
        return;
    }

    int quantity = stoi(quantityStr);

    // Find book
    Book* book = findBook(isbn);
    if (!book) {
        cout << "Invalid\n";
        return;
    }

    // Check stock
    if (book->stockQuantity < quantity) {
        cout << "Invalid\n";
        return;
    }

    // Update stock and calculate total
    book->stockQuantity -= quantity;
    double total = book->price * quantity;

    // Record income
    addFinanceRecord(total, 0.0);

    cout << fixed << setprecision(2) << total << "\n";
}

void executeSelect(const vector<string>& tokens) {
    // select [ISBN]
    if (tokens.size() != 2) {
        cout << "Invalid\n";
        return;
    }

    // Check privilege
    if (getCurrentPrivilege() < 3) {
        cout << "Invalid\n";
        return;
    }

    string isbn = tokens[1];
    if (!isValidISBN(isbn)) {
        cout << "Invalid\n";
        return;
    }

    string currentUserID = getCurrentUserID();
    if (currentUserID.empty()) {
        cout << "Invalid\n";
        return;
    }

    // Find or create book
    Book* book = findBook(isbn);
    if (!book) {
        // Create new book with only ISBN
        books.push_back(Book(isbn, "", "", "", 0.0, 0));
        book = &books.back();
    }

    // Set selected book for current user
    selectedBooks[currentUserID] = isbn;
    cout << "\n"; // Successful select outputs empty line
}

void executeModify(const vector<string>& tokens) {
    // modify (-ISBN=[ISBN] | -name="[BookName]" | -author="[Author]" | -keyword="[Keyword]" | -price=[Price])+
    if (tokens.size() < 2) {
        cout << "Invalid\n";
        return;
    }

    // Check privilege
    if (getCurrentPrivilege() < 3) {
        cout << "Invalid\n";
        return;
    }

    string currentUserID = getCurrentUserID();
    if (currentUserID.empty()) {
        cout << "Invalid\n";
        return;
    }

    // Get selected book
    auto it = selectedBooks.find(currentUserID);
    if (it == selectedBooks.end()) {
        cout << "Invalid\n";
        return;
    }

    string selectedISBN = it->second;
    Book* book = findBook(selectedISBN);
    if (!book) {
        cout << "Invalid\n";
        return;
    }

    // Parse modification parameters
    map<string, string> modifications;
    bool hasISBNMod = false;
    string newISBN;

    for (size_t i = 1; i < tokens.size(); i++) {
        string param = tokens[i];

        if (param.find("-ISBN=") == 0) {
            if (hasISBNMod) {
                cout << "Invalid\n"; // Duplicate -ISBN parameter
                return;
            }
            hasISBNMod = true;
            newISBN = param.substr(6);
            if (!isValidISBN(newISBN) || newISBN.empty()) {
                cout << "Invalid\n";
                return;
            }
            if (newISBN == book->ISBN) {
                cout << "Invalid\n"; // Cannot change to same ISBN
                return;
            }
            if (findBook(newISBN)) {
                cout << "Invalid\n"; // ISBN already exists
                return;
            }
            modifications["ISBN"] = newISBN;
        } else if (param.find("-name=\"") == 0 && param.back() == '\"') {
            if (modifications.count("name")) {
                cout << "Invalid\n"; // Duplicate parameter
                return;
            }
            string name = param.substr(7, param.size() - 8);
            if (!isValidBookName(name) || name.empty()) {
                cout << "Invalid\n";
                return;
            }
            modifications["name"] = name;
        } else if (param.find("-author=\"") == 0 && param.back() == '\"') {
            if (modifications.count("author")) {
                cout << "Invalid\n"; // Duplicate parameter
                return;
            }
            string author = param.substr(9, param.size() - 10);
            if (!isValidAuthor(author) || author.empty()) {
                cout << "Invalid\n";
                return;
            }
            modifications["author"] = author;
        } else if (param.find("-keyword=\"") == 0 && param.back() == '\"') {
            if (modifications.count("keyword")) {
                cout << "Invalid\n"; // Duplicate parameter
                return;
            }
            string keyword = param.substr(10, param.size() - 11);
            if (!isValidKeyword(keyword) || keyword.empty()) {
                cout << "Invalid\n";
                return;
            }
            // Check for duplicate keyword segments
            vector<string> segments;
            stringstream kwss(keyword);
            string segment;
            while (getline(kwss, segment, '|')) {
                if (segment.empty()) {
                    cout << "Invalid\n";
                    return;
                }
                if (find(segments.begin(), segments.end(), segment) != segments.end()) {
                    cout << "Invalid\n"; // Duplicate keyword segment
                    return;
                }
                segments.push_back(segment);
            }
            modifications["keyword"] = keyword;
        } else if (param.find("-price=") == 0) {
            if (modifications.count("price")) {
                cout << "Invalid\n"; // Duplicate parameter
                return;
            }
            string priceStr = param.substr(7);
            if (!isValidPrice(priceStr) || priceStr.empty()) {
                cout << "Invalid\n";
                return;
            }
            modifications["price"] = priceStr;
        } else {
            cout << "Invalid\n";
            return;
        }
    }

    // Apply modifications
    if (modifications.count("ISBN")) {
        book->ISBN = modifications["ISBN"];
        // Update selected book reference
        selectedBooks[currentUserID] = book->ISBN;
    }
    if (modifications.count("name")) {
        book->bookName = modifications["name"];
    }
    if (modifications.count("author")) {
        book->author = modifications["author"];
    }
    if (modifications.count("keyword")) {
        book->keyword = modifications["keyword"];
    }
    if (modifications.count("price")) {
        book->price = stod(modifications["price"]);
    }

    cout << "\n"; // Successful modify outputs empty line
}

void executeImport(const vector<string>& tokens) {
    // import [Quantity] [TotalCost]
    if (tokens.size() != 3) {
        cout << "Invalid\n";
        return;
    }

    // Check privilege
    if (getCurrentPrivilege() < 3) {
        cout << "Invalid\n";
        return;
    }

    string currentUserID = getCurrentUserID();
    if (currentUserID.empty()) {
        cout << "Invalid\n";
        return;
    }

    // Get selected book
    auto it = selectedBooks.find(currentUserID);
    if (it == selectedBooks.end()) {
        cout << "Invalid\n";
        return;
    }

    string selectedISBN = it->second;
    Book* book = findBook(selectedISBN);
    if (!book) {
        cout << "Invalid\n";
        return;
    }

    string quantityStr = tokens[1];
    string totalCostStr = tokens[2];

    if (!isValidQuantity(quantityStr) || !isValidTotalCost(totalCostStr)) {
        cout << "Invalid\n";
        return;
    }

    int quantity = stoi(quantityStr);
    double totalCost = stod(totalCostStr);

    if (totalCost <= 0) {
        cout << "Invalid\n";
        return;
    }

    // Update stock
    book->stockQuantity += quantity;

    // Record expenditure
    addFinanceRecord(0.0, totalCost);

    cout << "\n"; // Successful import outputs empty line
}

void executeShowFinance(const vector<string>& tokens) {
    // show finance ([Count])?
    // Check privilege
    if (getCurrentPrivilege() != 7) {
        cout << "Invalid\n";
        return;
    }

    int count = -1; // -1 means all records
    if (tokens.size() == 3) {
        // show finance [Count]
        string countStr = tokens[2];
        if (!isValidQuantity(countStr)) {
            cout << "Invalid\n";
            return;
        }
        count = stoi(countStr);
        if (count == 0) {
            cout << "\n"; // Empty line when Count is 0
            return;
        }
    } else if (tokens.size() != 2) {
        cout << "Invalid\n";
        return;
    }

    // Read finance records
    ifstream financeFile(FINANCE_FILE, ios::binary);
    if (!financeFile) {
        // No finance records yet
        cout << "+ 0.00 - 0.00\n";
        return;
    }

    size_t recordCount;
    financeFile.read(reinterpret_cast<char*>(&recordCount), sizeof(recordCount));

    if (count > 0 && count > static_cast<int>(recordCount)) {
        cout << "Invalid\n";
        return;
    }

    double totalIncome = 0.0, totalExpenditure = 0.0;
    int recordsToRead = (count == -1) ? recordCount : min(count, static_cast<int>(recordCount));

    // Skip to the last 'recordsToRead' records
    financeFile.seekg(sizeof(size_t) + (recordCount - recordsToRead) * (sizeof(double) * 2), ios::beg);

    for (int i = 0; i < recordsToRead; i++) {
        double income, expenditure;
        financeFile.read(reinterpret_cast<char*>(&income), sizeof(income));
        financeFile.read(reinterpret_cast<char*>(&expenditure), sizeof(expenditure));
        totalIncome += income;
        totalExpenditure += expenditure;
    }

    cout << fixed << setprecision(2) << "+ " << totalIncome << " - " << totalExpenditure << "\n";
}

void executeLog() {
    // log
    // Check privilege
    if (getCurrentPrivilege() != 7) {
        cout << "Invalid\n";
        return;
    }

    // TODO: Implement log
    cout << "Log functionality not yet implemented\n";
}

void executeReportFinance() {
    // report finance
    // Check privilege
    if (getCurrentPrivilege() != 7) {
        cout << "Invalid\n";
        return;
    }

    // TODO: Implement finance report
    cout << "Finance report functionality not yet implemented\n";
}

void executeReportEmployee() {
    // report employee
    // Check privilege
    if (getCurrentPrivilege() != 7) {
        cout << "Invalid\n";
        return;
    }

    // TODO: Implement employee report
    cout << "Employee report functionality not yet implemented\n";
}