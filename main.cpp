#include <vector>
#include <queue>
#include <set>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
using namespace std;

vector<string> ReadFileLines(const string &path) {
    vector<string> lines;
    fstream file(path.c_str());
    
    if (file.fail()) {
        cout << "\nERROR: Can't open the file: " << path << "\n";
        return lines;
    }
    
    string line;
    while (getline(file, line)) {
        if (!line.empty())
            lines.push_back(line);
    }
    
    file.close();
    return lines;
}

void WriteFileLines(const string &path, const vector<string> &lines, bool append = true) {
    auto mode = append ? ios::app : ios::trunc;
    fstream file(path.c_str(), ios::in | ios::out | mode);
    
    if (file.fail()) {
        cout << "\nERROR: Can't open the file: " << path << "\n";
        return;
    }
    
    for (const auto &line : lines)
        file << line << "\n";
    
    file.close();
}

vector<string> SplitString(const string &str, const string &delimiter = ",") {
    vector<string> result;
    size_t start = 0, end;
    
    while ((end = str.find(delimiter, start)) != string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    
    result.push_back(str.substr(start));
    return result;
}

int ToInt(const string &str) {
    istringstream iss(str);
    int num;
    iss >> num;
    return num;
}

int ReadInt(int low, int high) {
    cout << "\nEnter number in range " << low << " - " << high << ": ";
    int value;
    cin >> value;
    
    if (low <= value && value <= high)
        return value;
    
    cout << "ERROR: Invalid number. Try again\n";
    return ReadInt(low, high);
}

int ShowMenu(const vector<string> &choices) {
    cout << "\nMenu:\n";
    for (int i = 0; i < choices.size(); ++i) {
        cout << "\t" << i + 1 << ": " << choices[i] << "\n";
    }
    return ReadInt(1, choices.size());
}

class Question {
private:
    int question_id;
    int parent_question_id;  // -1 means no parent
    int from_user_id;
    int to_user_id;
    int is_anonymous;        // 0 or 1
    string question_text;
    string answer_text;      // empty = not answered

public:
    Question() : 
        question_id(-1), parent_question_id(-1), 
        from_user_id(-1), to_user_id(-1), is_anonymous(1) {}
    
    Question(const string &line) {
        vector<string> parts = SplitString(line);
        if (parts.size() != 7) {
            cout << "ERROR: Invalid question format\n";
            return;
        }
        
        question_id = ToInt(parts[0]);
        parent_question_id = ToInt(parts[1]);
        from_user_id = ToInt(parts[2]);
        to_user_id = ToInt(parts[3]);
        is_anonymous = ToInt(parts[4]);
        question_text = parts[5];
        answer_text = parts[6];
    }
    
    string ToString() const {
        ostringstream oss;
        oss << question_id << "," << parent_question_id << ","
            << from_user_id << "," << to_user_id << ","
            << is_anonymous << "," << question_text << ","
            << answer_text;
        return oss.str();
    }
    
    void PrintQuestion(bool is_to_me) const {
        string prefix = "";
        if (parent_question_id != -1)
            prefix = "\tThread: ";
        
        cout << prefix << "Question ID (" << question_id << ")";
        
        if (is_to_me) {
            if (!is_anonymous)
                cout << " from user ID(" << from_user_id << ")";
            cout << "\tQuestion: " << question_text << "\n";
            
            if (!answer_text.empty())
                cout << prefix << "\tAnswer: " << answer_text << "\n";
        } else {
            if (!is_anonymous)
                cout << " !Anonymous";
            
            cout << " to user ID(" << to_user_id << ")";
            cout << "\tQuestion: " << question_text;
            
            if (!answer_text.empty())
                cout << "\tAnswer: " << answer_text << "\n";
            else
                cout << "\tNOT Answered YET\n";
        }
        
        cout << "\n";
    }
    
    void PrintFeed() const {
        if (parent_question_id != -1)
            cout << "Thread Parent Question ID (" << parent_question_id << ") ";
        
        cout << "Question ID (" << question_id << ")";
        if (!is_anonymous)
            cout << " from user ID(" << from_user_id << ")";
        
        cout << " to user ID(" << to_user_id << ")";
        cout << "\tQuestion: " << question_text << "\n";
        
        if (!answer_text.empty())
            cout << "\tAnswer: " << answer_text << "\n";
    }
    
    int GetId() const { return question_id; }
    void SetId(int id) { question_id = id; }
    
    int GetParentId() const { return parent_question_id; }
    void SetParentId(int id) { parent_question_id = id; }
    
    int GetFromUserId() const { return from_user_id; }
    void SetFromUserId(int id) { from_user_id = id; }
    
    int GetToUserId() const { return to_user_id; }
    void SetToUserId(int id) { to_user_id = id; }
    
    int IsAnonymous() const { return is_anonymous; }
    void SetAnonymous(int anon) { is_anonymous = anon; }
    
    const string& GetQuestion() const { return question_text; }
    void SetQuestion(const string& text) { question_text = text; }
    
    const string& GetAnswer() const { return answer_text; }
    void SetAnswer(const string& text) { answer_text = text; }
    
    bool IsAnswered() const { return !answer_text.empty(); }
};

class User {
private:
    int user_id;
    string username;
    string password;
    string name;
    string email;
    int allow_anonymous;  // 0 or 1
    
    vector<int> questions_from_me;
    map<int, vector<int>> questions_to_me;  // parent_id -> [question_ids]

public:
    User() : user_id(-1), allow_anonymous(-1) {}
    
    User(const string &line) {
        vector<string> parts = SplitString(line);
        if (parts.size() != 6) {
            cout << "ERROR: Invalid user format\n";
            return;
        }
        
        user_id = ToInt(parts[0]);
        username = parts[1];
        password = parts[2];
        name = parts[3];
        email = parts[4];
        allow_anonymous = ToInt(parts[5]);
    }
    
    string ToString() const {
        ostringstream oss;
        oss << user_id << "," << username << "," << password << ","
            << name << "," << email << "," << allow_anonymous;
        return oss.str();
    }
    
    void Print() const {
        cout << "User " << user_id << ", " << username << ", " 
             << name << ", " << email << "\n";
    }
    
    void InputUserData(const string &user_name, int id) {
        username = user_name;
        user_id = id;
        
        cout << "Enter password: ";
        cin >> password;
        
        cout << "Enter name: ";
        cin >> name;
        
        cout << "Enter email: ";
        cin >> email;
        
        cout << "Allow anonymous questions? (0 or 1): ";
        cin >> allow_anonymous;
    }
    
    void SetQuestionsToMe(const map<int, vector<int>>& questions) {
        questions_to_me = questions;
    }
    
    void SetQuestionsFromMe(const vector<int>& questions) {
        questions_from_me = questions;
    }
    
    int GetId() const { return user_id; }
    const string& GetUsername() const { return username; }
    const string& GetPassword() const { return password; }
    const string& GetName() const { return name; }
    int AllowsAnonymous() const { return allow_anonymous; }
    
    const vector<int>& GetQuestionsFromMe() const { return questions_from_me; }
    const map<int, vector<int>>& GetQuestionsToMe() const { return questions_to_me; }
};

class QuestionManager {
private:
    map<int, vector<int>> thread_questions;  // parent_id -> [question_ids]
    map<int, Question> questions;            // question_id -> Question
    int next_id;

public:
    QuestionManager() : next_id(0) {}
    
    void LoadDatabase() {
        next_id = 0;
        thread_questions.clear();
        questions.clear();
        
        vector<string> lines = ReadFileLines("questions.txt");
        for (const auto &line : lines) {
            Question question(line);
            next_id = max(next_id, question.GetId());
            
            questions[question.GetId()] = question;
            
            if (question.GetParentId() == -1) {
                thread_questions[question.GetId()].push_back(question.GetId());
            } else {
                thread_questions[question.GetParentId()].push_back(question.GetId());
            }
        }
    }
    
    void SaveDatabase() {
        vector<string> lines;
        for (const auto &pair : questions) {
            lines.push_back(pair.second.ToString());
        }
        WriteFileLines("questions.txt", lines, false);
    }
    
    map<int, vector<int>> GetQuestionsToUser(int user_id) const {
        map<int, vector<int>> result;
        
        for (const auto &thread : thread_questions) {
            for (int q_id : thread.second) {
                const Question &q = questions.at(q_id);
                if (q.GetToUserId() == user_id) {
                    int parent_id = (q.GetParentId() == -1) ? q.GetId() : q.GetParentId();
                    result[parent_id].push_back(q.GetId());
                }
            }
        }
        
        return result;
    }
    
    vector<int> GetQuestionsFromUser(int user_id) const {
        vector<int> result;
        
        for (const auto &pair : questions) {
            if (pair.second.GetFromUserId() == user_id) {
                result.push_back(pair.first);
            }
        }
        
        return result;
    }
    
    void PrintUserQuestions(const User &user, bool to_me) const {
        cout << "\n";
        
        if (to_me) {
            const auto &q_to_me = user.GetQuestionsToMe();
            if (q_to_me.empty()) {
                cout << "No questions to you.\n";
                return;
            }
            
            for (const auto &thread : q_to_me) {
                for (int q_id : thread.second) {
                    const Question &q = questions.at(q_id);
                    q.PrintQuestion(true);
                }
            }
        } else {  // from me
            const auto &q_from_me = user.GetQuestionsFromMe();
            if (q_from_me.empty()) {
                cout << "You haven't asked any questions.\n";
                return;
            }
            
            for (int q_id : q_from_me) {
                const Question &q = questions.at(q_id);
                q.PrintQuestion(false);
            }
        }
        
        cout << "\n";
    }
    
    int ReadQuestionIdForUser(int user_id, bool for_answering = false) const {
        int question_id;
        cout << "Enter Question ID or -1 to cancel: ";
        cin >> question_id;
        
        if (question_id == -1)
            return -1;
        
        if (questions.find(question_id) == questions.end()) {
            cout << "\nERROR: No question with such ID. Try again\n\n";
            return ReadQuestionIdForUser(user_id, for_answering);
        }
        
        if (for_answering && questions.at(question_id).GetToUserId() != user_id) {
            cout << "\nERROR: This question wasn't directed to you. Try again\n\n";
            return ReadQuestionIdForUser(user_id, for_answering);
        }
        
        return question_id;
    }
    
    int ReadThreadQuestionId() const {
        int question_id;
        cout << "For thread question: Enter Question ID or -1 for new question: ";
        cin >> question_id;
        
        if (question_id == -1)
            return -1;
        
        if (thread_questions.find(question_id) == thread_questions.end()) {
            cout << "No thread question with such ID. Try again\n";
            return ReadThreadQuestionId();
        }
        
        return question_id;
    }
    
    void AnswerQuestion(int user_id) {
        int question_id = ReadQuestionIdForUser(user_id, true);
        if (question_id == -1)
            return;
        
        Question &question = questions.at(question_id);
        question.PrintQuestion(true);
        
        if (question.IsAnswered())
            cout << "\nWarning: Already answered. Answer will be updated\n";
        
        cout << "Enter answer: ";
        string answer;
        cin.ignore();  // Clear the input buffer
        getline(cin, answer);
        question.SetAnswer(answer);
        
        SaveDatabase();
    }
    
    void DeleteQuestion(int user_id) {
        int question_id = ReadQuestionIdForUser(user_id, true);
        if (question_id == -1)
            return;
        
        vector<int> to_remove;
        
        // Check if it's a thread parent
        if (thread_questions.find(question_id) != thread_questions.end()) {
            to_remove = thread_questions[question_id];
            thread_questions.erase(question_id);
        } else {
            to_remove.push_back(question_id);
            
            // Find and remove from its thread
            for (auto &thread : thread_questions) {
                auto &questions_list = thread.second;
                auto it = find(questions_list.begin(), questions_list.end(), question_id);
                if (it != questions_list.end()) {
                    questions_list.erase(it);
                    break;
                }
            }
        }
        
        // Remove all questions
        for (int id : to_remove) {
            questions.erase(id);
        }
        
        SaveDatabase();
    }
    
    void AskQuestion(const User &from_user, int to_user_id, bool allows_anonymous) {
        Question question;
        
        if (!allows_anonymous) {
            cout << "Note: Anonymous questions are not allowed for this user\n";
            question.SetAnonymous(0);
        } else {
            cout << "Ask anonymously? (0 or 1): ";
            int anon;
            cin >> anon;
            question.SetAnonymous(anon);
        }
        
        int parent_id = ReadThreadQuestionId();
        question.SetParentId(parent_id);
        
        cout << "Enter question text: ";
        string text;
        cin.ignore();  // Clear the input buffer
        getline(cin, text);
        question.SetQuestion(text);
        
        question.SetFromUserId(from_user.GetId());
        question.SetToUserId(to_user_id);
        question.SetId(++next_id);
        
        questions[question.GetId()] = question;
        
        if (parent_id == -1) {
            thread_questions[question.GetId()].push_back(question.GetId());
        } else {
            thread_questions[parent_id].push_back(question.GetId());
        }
        
        SaveDatabase();
    }
    
    void ListFeed() const {
        bool found = false;
        
        for (const auto &pair : questions) {
            const Question &q = pair.second;
            if (q.IsAnswered()) {
                q.PrintFeed();
                found = true;
            }
        }
        
        if (!found) {
            cout << "No answered questions in the feed.\n";
        }
    }
};

class UserManager {
private:
    map<string, User> users;
    User current_user;
    int next_id;
    
public:
    UserManager() : next_id(0) {}
    
    void LoadDatabase() {
        next_id = 0;
        users.clear();
        
        vector<string> lines = ReadFileLines("users.txt");
        for (const auto &line : lines) {
            User user(line);
            users[user.GetUsername()] = user;
            next_id = max(next_id, user.GetId());
        }
    }
    
    void SaveDatabase() {
        vector<string> lines;
        for (const auto &pair : users) {
            lines.push_back(pair.second.ToString());
        }
        WriteFileLines("users.txt", lines, false);
    }
    
    bool Login() {
        string username, password;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;
        
        if (users.find(username) == users.end() || 
            users[username].GetPassword() != password) {
            cout << "\nInvalid username or password. Try again.\n\n";
            return false;
        }
        
        current_user = users[username];
        return true;
    }
    
    void Signup() {
        string username;
        while (true) {
            cout << "Enter username (no spaces): ";
            cin >> username;
            
            if (users.find(username) != users.end()) {
                cout << "Username already taken. Try another.\n";
            } else {
                break;
            }
        }
        
        User new_user;
        new_user.InputUserData(username, ++next_id);
        
        users[username] = new_user;
        current_user = new_user;
        
        SaveUser(current_user);
    }
    
    void ListUsers() const {
        cout << "\nSystem Users:\n";
        for (const auto &pair : users) {
            const User &user = pair.second;
            cout << "ID: " << user.GetId() << "\tName: " << user.GetName() << "\n";
        }
    }
    
    pair<int, bool> ReadUserId() const {
        int user_id;
        cout << "Enter User ID or -1 to cancel: ";
        cin >> user_id;
        
        if (user_id == -1)
            return {-1, false};
        
        for (const auto &pair : users) {
            if (pair.second.GetId() == user_id) {
                return {user_id, pair.second.AllowsAnonymous()};
            }
        }
        
        cout << "Invalid User ID. Try again.\n";
        return ReadUserId();
    }
    
    void SaveUser(const User &user) {
        users[user.GetUsername()] = user;
        vector<string> lines = {user.ToString()};
        WriteFileLines("users.txt", lines);
    }
    
    User& GetCurrentUser() { return current_user; }
    
    void UpdateUserQuestions(const map<int, vector<int>> &to_questions, 
                           const vector<int> &from_questions) {
        current_user.SetQuestionsToMe(to_questions);
        current_user.SetQuestionsFromMe(from_questions);
    }
};

class AskSystem {
private:
    UserManager user_manager;
    QuestionManager question_manager;
    
    void LoadData(bool refresh_user_questions = false) {
        user_manager.LoadDatabase();
        question_manager.LoadDatabase();
        
        if (refresh_user_questions) {
            RefreshUserQuestions();
        }
    }
    
    void RefreshUserQuestions() {
        User &user = user_manager.GetCurrentUser();
        auto to_questions = question_manager.GetQuestionsToUser(user.GetId());
        auto from_questions = question_manager.GetQuestionsFromUser(user.GetId());
        user_manager.UpdateUserQuestions(to_questions, from_questions);
    }
    
    void RunUserSession() {
        vector<string> menu = {
            "View Questions To Me",
            "View Questions From Me",
            "Answer Question",
            "Delete Question",
            "Ask Question",
            "List System Users",
            "View Feed",
            "Logout"
        };
        
        while (true) {
            int choice = ShowMenu(menu);
            LoadData(true);  // Refresh data before each action
            
            switch (choice) {
                case 1:  // View Questions To Me
                    question_manager.PrintUserQuestions(user_manager.GetCurrentUser(), true);
                    break;
                    
                case 2:  // View Questions From Me
                    question_manager.PrintUserQuestions(user_manager.GetCurrentUser(), false);
                    break;
                    
                case 3:  // Answer Question
                    question_manager.AnswerQuestion(user_manager.GetCurrentUser().GetId());
                    break;
                    
                case 4:  // Delete Question
                    question_manager.DeleteQuestion(user_manager.GetCurrentUser().GetId());
                    RefreshUserQuestions();
                    break;
                    
                case 5: {  // Ask Question
                    auto [user_id, allows_anon] = user_manager.ReadUserId();
                    if (user_id != -1) {
                        question_manager.AskQuestion(user_manager.GetCurrentUser(), 
                                                   user_id, allows_anon);
                    }
                    break;
                }
                
                case 6:  // List System Users
                    user_manager.ListUsers();
                    break;
                    
                case 7:  // View Feed
                    question_manager.ListFeed();
                    break;
                    
                case 8:  // Logout
                    return;
            }
        }
    }
    
    void AccessSystem() {
        while (true) {
            int choice = ShowMenu({"Login", "Sign Up"});
            
            if (choice == 1) {  // Login
                LoadData();  // Refresh user database
                if (user_manager.Login()) {
                    RefreshUserQuestions();
                    return;
                }
            } else {  // Sign Up
                user_manager.Signup();
                return;
            }
        }
    }
    
public:
    void Run() {
        while (true) {
            AccessSystem();
            RunUserSession();
        }
    }
};

int main() {
    AskSystem system;
    system.Run();
    return 0;
}