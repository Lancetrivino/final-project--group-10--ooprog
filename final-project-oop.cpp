#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <limits>

using namespace std;


class Admin;
class Teacher;
class Student;

class UserActionStrategy {
public:
    virtual void execute() = 0; // Pure virtual function
    virtual ~UserActionStrategy() = default; // Virtual destructor
};


class InvalidCourseIndexException : public runtime_error {
public:
    InvalidCourseIndexException() : runtime_error("Invalid course index!") {}
};
class Validator {
public:
    static bool isValidEmail(const string& email) {
        // Basic email validation
        size_t atPos = email.find('@');
        size_t dotPos = email.rfind('.');
        return atPos != string::npos && dotPos != string::npos && 
               atPos < dotPos && atPos > 0 && 
               dotPos < email.length() - 1;
    }

    static bool isValidGrade(int grade) {
        return grade >= 0 && grade <= 100;
    }

    static bool isValidIndex(int index, int maxSize) {
        return index >= 0 && index < maxSize;
    }

    static bool isValidString(const string& str) {
        return !str.empty() && str.length() <= 100;  
    }

    static int getValidatedIntInput(const string& prompt, int min, int max) {
        int input;
        bool validInput = false;
        do {
            cout << prompt;
            if (cin >> input) {
                if (input >= min && input <= max) {
                    validInput = true;
                } else {
                    cout << "Please enter a number between " << min << " and " << max << ".\n";
                }
            } else {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a number.\n";
                
            }
        } while (!validInput);
        return input;
    }
};


class User {
protected:
    string username;
    string email;
    string password;
    unique_ptr<UserActionStrategy> actionStrategy; // Strategy member

public:
    User(string username, string email, string password)
        : username(username), email(email), password(password) {}

    void setActionStrategy(UserActionStrategy* strategy) {
        actionStrategy.reset(strategy); // Set the strategy
    }

    void performAction() {
        if (actionStrategy) {
            actionStrategy->execute(); // Execute the strategy
        }
    }

    virtual void displayMenu() = 0; // Pure virtual function
    virtual ~User() = default; // Virtual destructor

    string getEmail() const { return email; }
    string getPassword() const { return password; }
};

class ValidationException : public runtime_error {
public:
    ValidationException(const string& msg) : runtime_error(msg) {}
};

using UserPtr = shared_ptr<User>;
vector<UserPtr> users; 


class Course;
class LMSManager;


class Admin : public User {
public:
     Admin(string username, string email, string password)
        : User(username, email, password) {}

    void displayMenu() override;
    void manageCourses();
    void addCourse();
    void deleteCourse();
    void editCourse();
    void viewReports();
    void enrollStudent();    
    void removeStudent();   
};


class Teacher : public User {
public:
    Teacher(string username, string email, string password)
        : User(username, email, password) {}

    void displayMenu() override;
    void manageCourses();
    void viewCourse();
    void viewReports();
    void addGrade(); 
    void addContent(); 
    void viewAssignedStudents();
};


class Student : public User {
public:
    Student(string username, string email, string password)
        : User(username, email, password) {}

    void displayMenu() override;
    void viewEnrolledCourses();
    void viewGrades();
    void enrollInCourse();
};


// Concrete Strategies
class AdminActions : public UserActionStrategy {
    Admin* admin;

public:
    AdminActions(Admin* admin) : admin(admin) {}

    void execute() override {
        admin->displayMenu(); 
    }
};

class TeacherActions : public UserActionStrategy {
    Teacher* teacher;

public:
    TeacherActions(Teacher* teacher) : teacher(teacher) {}

    void execute() override {
        teacher->displayMenu(); 
    }
};

class StudentActions : public UserActionStrategy {
    Student* student;

public:
    StudentActions(Student* student) : student(student) {}

    void execute() override {
        student->displayMenu(); 
    }
};


class Course {
private:
    string courseName;
    string teacherEmail;
    vector<string> contents;
    vector<pair<string, int>> grades;
    vector<string> enrolledStudents;
    
     

public:
    Course(string courseName, string teacherEmail) {
        if (!Validator::isValidString(courseName)) {
            throw ValidationException("Invalid course name");
        }
        if (!Validator::isValidEmail(teacherEmail)) {
            throw ValidationException("Invalid teacher email");
        }
        this->courseName = courseName;
        this->teacherEmail = teacherEmail;
    }

    void addContent(string content) {
        if (!Validator::isValidString(content)) {
            throw ValidationException("Invalid content");
        }
        contents.push_back(content);
    }

    void removeContent(int index) {
        if (!Validator::isValidIndex(index, contents.size())) {
            throw InvalidCourseIndexException();
        }
        contents.erase(contents.begin() + index);
    }

    void displayContents() const {
        if (contents.empty()) {
        cout << "No content available for this course.\n";
        return;
    }

    cout << "Course Contents:\n";
    for (const auto& content : contents) {
        cout << "- " << content << endl;
    }
}

    void addGrade(const string& studentEmail, int grade) {
        if (!Validator::isValidEmail(studentEmail)) {
            throw ValidationException("Invalid student email");
        }
        if (!Validator::isValidGrade(grade)) {
            throw ValidationException("Invalid grade");
        }
        grades.push_back({studentEmail, grade});
    }

    const vector<pair<string, int>>& getGrades() const {
        return grades;
    }
    

    void displayGrades() const {
        for (const auto& grade : grades) {
            cout << grade.first << ": " << grade.second << "%" << endl;
        }
    }

    void enrollStudent(const string& studentEmail) {
    if (!Validator::isValidEmail(studentEmail)) {
        throw ValidationException("Invalid student email");
    }

    
    for (const auto& enrolledEmail : enrolledStudents) {
        if (enrolledEmail == studentEmail) {
            throw ValidationException("Student already enrolled");
        }
    }

    enrolledStudents.push_back(studentEmail); 
}

   void removeStudent(const string& studentEmail) {
   
    for (auto it = enrolledStudents.begin(); it != enrolledStudents.end(); ++it) {
        if (*it == studentEmail) {
            enrolledStudents.erase(it); 
            return; 
        }
    }
    
    
    throw ValidationException("Student not found");
}

    void displayStudents() const {
        for (const auto& student : enrolledStudents) {
            cout << student << endl;
        }
    }

    string getCourseName() const { return courseName; }
    string getTeacherEmail() const { return teacherEmail; }
    const vector<string>& getStudents() const { return enrolledStudents; }
    const vector<string>& getContents() const { return contents; }
};




class LMSManager {
private:
    vector<Course> courses;
    static unique_ptr<LMSManager> instance;
    LMSManager() = default;

public:
    static LMSManager* getInstance() {
        if (!instance) {
            instance = unique_ptr<LMSManager>(new LMSManager());
        }
        return instance.get();
    }

    void addCourse(const Course& course) {
        courses.push_back(course);
    }
    

    Course& getCourse(int index) {
        if (!Validator::isValidIndex(index, courses.size())) {
            throw InvalidCourseIndexException();
        }
        return courses[index];
    }

    void removeCourse(int index) {
        if (!Validator::isValidIndex(index, courses.size())) {
            throw InvalidCourseIndexException();
        }
        courses.erase(courses.begin() + index);
    }

    void displayCourses() const {
        if (courses.empty()) {
            cout << "There are no courses available.\n";
            return;
        }
        for (size_t i = 0; i < courses.size(); ++i) {
            cout << i + 1 << ": " << courses[i].getCourseName()
                 << " (Teacher: " << courses[i].getTeacherEmail() << ")" << endl;
        }
    }

    vector<Course>& getCourses() { return courses; }
};


unique_ptr<LMSManager> LMSManager::instance;


void Admin::displayMenu() {
    int choice;
    do {
        system("cls");
        cout << "\nAdmin Menu:\n";
        cout << "1. Manage Courses\n";
        cout << "2. View Reports\n";
        cout << "3. Enroll Student\n";
        cout << "4. Remove Student\n";
        cout << "5. Log Out\n";
        
        choice = Validator::getValidatedIntInput("Enter choice (1-5): ", 1, 5);

        switch (choice) {
            case 1:
                manageCourses();
                break;
            case 2:
                viewReports();
                break;
            case 3:
                enrollStudent();
                break;
            case 4:
                removeStudent();
                system("pause");
                break;
            case 5:
                cout << "Logging out...\n";
                system("pause");
                break;
        }
    } while (choice != 5);
}
void Admin::enrollStudent() {
    vector<Course>& courses = LMSManager::getInstance()->getCourses();
    if (courses.empty()) {
        cout << "There are no courses available for enrollment.\n";
        return;
    }

    LMSManager::getInstance()->displayCourses();
    int userIndex = Validator::getValidatedIntInput(
        "Enter course index to enroll student (1-" + to_string(courses.size()) + "): ",
        1, courses.size());

    try {
        Course& course = LMSManager::getInstance()->getCourse(userIndex - 1);
        
        string studentEmail;
        string studentPassword;
        bool validEmail = false;
        
        // Email validation
        do {
            cout << "Enter student's email: ";
            cin >> studentEmail;
            if (Validator::isValidEmail(studentEmail)) {
                
                // Check if student already exists
                bool studentExists = false;
                for (const auto& user : users) {
                    if (user->getEmail() == studentEmail) {
                        studentExists = true;
                        break;
                    }
                }
                
                if (studentExists) {
                    cout << "Student with this email already exists. Cannot create a duplicate account.\n";
                    return;
                }
                
                validEmail = true;
            } else {
                cout << "Invalid email format. Please try again.\n";
            }
        } while (!validEmail);
        
        // Password input
        cout << "Enter password for the student: ";
        cin >> studentPassword;
        
        // Create new student
        UserPtr newStudent = make_shared<Student>(
            studentEmail.substr(0, studentEmail.find('@')),  
            studentEmail, 
            studentPassword
        );
        
        // Add to users list
        users.push_back(newStudent);
        
        // Enroll in the course
        course.enrollStudent(studentEmail);
        
        cout << "Student enrolled successfully and account created.\n";
        cout << "Username: " << newStudent->getEmail() << endl;
        system("pause");
    } catch (const exception& e) {
        cout << e.what() << endl;
        system("pause");
    }
}

void Admin::removeStudent() {
    
    vector<Course>& courses = LMSManager::getInstance()->getCourses();
    if (courses.empty()) {
        cout << "There are no courses available.\n";
        return;
    }

    LMSManager::getInstance()->displayCourses();
    int userIndex;
    cout << "Enter course index to remove student (1-" << courses.size() << "): ";
    cin >> userIndex;

    try {
       
        int systemIndex = userIndex - 1;
        Course& course = LMSManager::getInstance()->getCourse(systemIndex);

        
        if (course.getStudents().empty()) {
            cout << "There is no student here.\n";
            return;
        }

        string studentEmail;
        cout << "Enter student's email to remove: ";
        cin >> studentEmail;

        try {
            course.removeStudent(studentEmail);
            cout << "Student removed successfully.\n";
        } catch (const runtime_error&) {
            cout << "Student not found in the course.\n";
        }
    } catch (InvalidCourseIndexException&) {
        cout << "Invalid course index. Please enter a number between 1 and " 
             << courses.size() << ".\n";
    }
}

void Admin::manageCourses() {
    int choice;
    do {
        system("cls");
        cout << "\nManage Courses:\n";
        cout << "1. Add Course\n";
        cout << "2. Delete Course\n";
        cout << "3. Edit Course\n";
        cout << "4. Display Courses\n";
        cout << "5. Back\n";

        choice = Validator::getValidatedIntInput("Enter choice (1-5): ", 1, 5);

        switch (choice) {
            case 1:
                addCourse();
                break;
            case 2:
                deleteCourse();
                system("pause");
                break;
            case 3:
                editCourse();
                break;
            case 4:
                LMSManager::getInstance()->displayCourses();
                system("pause");
                break;
            case 5:
                cout << "Returning...\n";
                system("pause");
                break;
        }
    } while (choice != 5);
}

void Admin::addCourse() {
    system("cls");
    string courseName, teacherEmail;
    
    cout << "Enter course name: ";
    cin.ignore();
    getline(cin, courseName);
    
    cout << "Enter teacher's email: ";
    cin >> teacherEmail;

    // Check if the teacher's email exists among the registered users
    bool teacherExists = false;
    for (const auto& user : users) {
        if (user->getEmail() == teacherEmail) {
            teacherExists = true;
            break;
        }
    }

    if (!teacherExists) {
        char addTeacher;
        cout << "Error: The email does not belong to a registered teacher.\n";
        cout << "Would you like to register this teacher? (y/n): ";
        cin >> addTeacher;
        cin.ignore(); 

        if (addTeacher == 'y' || addTeacher == 'Y') {
            string teacherName, teacherPassword;

            cout << "Enter teacher's name: ";
            getline(cin, teacherName);
            cout << "Enter teacher's password: ";
            getline(cin, teacherPassword);

            // Create a new Teacher object and add to the users
            auto newTeacher = make_shared<Teacher>(teacherName, teacherEmail, teacherPassword);
            users.push_back(newTeacher);
            cout << "Teacher registered successfully: " << teacherName << " (" << teacherEmail << ")\n";
        } else {
            cout << "Course addition canceled.\n";
            system("pause"); // Wait for user to see the message
            return; // Exit the function if the admin does not want to register the teacher
        }
    }

    // Ensure teacher is not managing multiple subjects
    vector<Course>& courses = LMSManager::getInstance()->getCourses();
    for (const auto& course : courses) {
        if (course.getTeacherEmail() == teacherEmail) {
            cout << "Error: Teacher is already assigned to another course.\n";
            system("pause");
            return;
        }
    }

    
    Course newCourse(courseName, teacherEmail);
    LMSManager::getInstance()->addCourse(newCourse);
    cout << "Course added successfully.\n";
    system("pause");
}
void Admin::deleteCourse() {
    // Check if there are any courses
    vector<Course>& courses = LMSManager::getInstance()->getCourses();
    if (courses.empty()) {
        cout << "There are no courses to delete.\n";
        system("pause");  
        return; 
    }
    
    
    LMSManager::getInstance()->displayCourses();
    int index;
    cout << "Enter course index to delete: ";
    cin >> index;
    
     try {
      
        if (index < 1 || index > courses.size()) {
            throw out_of_range("Invalid index");
        }
        
       
        Course courseToDelete = courses[index - 1];  
        
        LMSManager::getInstance()->removeCourse(index - 1);  
        cout << "Successfully deleted course: " << courseToDelete.getCourseName() << endl;
    } catch (InvalidCourseIndexException&) {
        cout << "Invalid course index.\n";
    } catch (out_of_range&) {
        
        cout << "Invalid course index.\n";
    }
}

void Admin::editCourse() {
    system("cls");  

    
    vector<Course>& courses = LMSManager::getInstance()->getCourses();
    if (courses.empty()) {
        cout << "There are no courses available.\n";
        system("pause");  
        return;  
    }

    
    LMSManager::getInstance()->displayCourses();
    int userIndex;
    cout << "Enter course index to edit (1-" << courses.size() << "): ";
    cin >> userIndex;

    try {
        
        int systemIndex = userIndex - 1;
        Course& course = LMSManager::getInstance()->getCourse(systemIndex);
        cout << "Editing course: " << course.getCourseName() << endl;
        
        cout << "Would you like to edit the course content? (y/n): ";
        char choice;
        cin >> choice;
        
        if (tolower(choice) == 'y') {
            int contentChoice;
            cout << "1. Add content\n2. Remove content\nEnter choice: ";
            cin >> contentChoice;

            if (contentChoice == 1) {
                string content;
                cout << "Enter content: ";
                cin.ignore();
                getline(cin, content);
                course.addContent(content);
                cout << "Content added successfully.\n";
            } 
            else if (contentChoice == 2) {
                // Check if there's any content to remove
                vector<string> contents = course.getContents();
                if (contents.empty()) {
                    cout << "There is no content to remove.\n";
                } 
                else {
                    
                    cout << "\nCurrent content:\n";
                    for (size_t i = 0; i < contents.size(); i++) {
                        cout << i + 1 << ". " << contents[i] << endl;
                    }

                    int userContentIndex;
                    cout << "Enter content index to remove (1-" << contents.size() << "): ";
                    cin >> userContentIndex;

                    try {
                        
                        course.removeContent(userContentIndex - 1);
                        cout << "Content removed successfully.\n";
                    } 
                    catch (const out_of_range&) {
                        cout << "Invalid content index. Please enter a number between 1 and " 
                             << contents.size() << ".\n";
                    }
                }
            } 
            else {
                cout << "Invalid choice. Please select 1 or 2.\n";
            }
        }
    } 
    catch (InvalidCourseIndexException&) {
        cout << "Invalid course index. Please enter a number between 1 and " 
             << courses.size() << ".\n";
    }
    
    system("pause"); 
}


void Admin::viewReports() {
    system("cls");  
    vector<Course>& courses = LMSManager::getInstance()->getCourses();

    if (courses.empty()) {
        cout << "No courses available to generate reports.\n";
        system("pause");
        return;
    }

    cout << "Courses Report:\n";
    for (auto& course : courses) {
        cout << "Course: " << course.getCourseName() << " (Teacher: " << course.getTeacherEmail() << ")\n";
        cout << "Enrolled Students:\n";
        course.displayStudents();
        cout << "Grades:\n";
        course.displayGrades();
        system("pause");   
        cout << "----------------------\n";
    }
    system("pause");
}

// Teacher class implementation
void Teacher::displayMenu() {
    int choice;
    do {
        system("cls");
        cout << "\nTeacher Menu:\n";
        cout << "1. Manage Courses\n";
        cout << "2. View Reports\n";
        cout << "3. Log Out\n";

        choice = Validator::getValidatedIntInput("Enter choice (1-3): ", 1, 3);

        switch (choice) {
            case 1:
                manageCourses();
                break;

            case 2:
                viewReports();
                break;

            case 3:
                cout << "Logging out...\n";
                system("pause");
                break;
        }
    } while (choice != 3);
}

void Teacher::addGrade() {
    system("cls");
    vector<Course>& courses = LMSManager::getInstance()->getCourses();
    
    if (courses.empty()) {
        cout << "No courses available.\n";
        system("pause");
        return; 
    }

    
    vector<Course> assignedCourses;

    // Filter courses to find those assigned to the current teacher
    for (const auto& course : courses) {
        if (course.getTeacherEmail() == getEmail()) {
            assignedCourses.push_back(course);
        }
    }

   
    if (assignedCourses.empty()) {
        cout << "You are not assigned to any courses. Cannot add grades.\n";
        system("pause");
        return; // Exit if no courses are assigned
    }

   
    cout << "Your Assigned Courses:\n";
    for (size_t i = 0; i < assignedCourses.size(); ++i) {
        cout << i + 1 << ". " << assignedCourses[i].getCourseName() << endl;
    }

    int courseIndex = Validator::getValidatedIntInput(
        "Enter course index (1-" + to_string(assignedCourses.size()) + "): ",
        1, assignedCourses.size());

    try {
        Course& course = assignedCourses[courseIndex - 1]; 
        
        string studentEmail;
        bool validEmail = false;
        do {
            cout << "Enter student's email: ";
            cin >> studentEmail;
            if (Validator::isValidEmail(studentEmail)) {
                validEmail = true;
            } else {
                cout << "Invalid email format. Please try again.\n";
            }
        } while (!validEmail);

       
        bool studentFound = false;
        for (const string& student : course.getStudents()) {
            if (student == studentEmail) {
                studentFound = true;
                break;
            }
        }

        if (!studentFound) {
            cout << "Student is not enrolled in this course.\n";
            system("pause");
            return;
        }

        int grade = Validator::getValidatedIntInput(
            "Enter grade (0-100): ",
            0, 100);

        course.addGrade(studentEmail, grade);
        cout << "Grade added successfully for student: " << studentEmail << endl;
        system("pause");
    } catch (const exception& e) {
        cout << e.what() << endl;
        system("pause");
    }
}

void Teacher::manageCourses() {
    int choice;
    do {
        system("cls");
        cout << "\nManage Courses:\n";
        cout << "1. View Course\n";
        cout << "2. Add Content\n";
        cout << "3. Add Grade\n";
        cout << "4. View Assigned Students\n";
        cout << "5. Back\n";

        choice = Validator::getValidatedIntInput("Enter choice (1-5): ", 1, 5);

        switch (choice) {
            case 1:
                viewCourse();
                break;
            case 2: 
                addContent();
                break;
                
            case 3:
                addGrade();
                break;
            case 4: {
                viewAssignedStudents();
                break;
            }
            case 5:
                cout << "Returning...\n";
                system("pause");
                break;
        }
    } while (choice != 5);
}


void Teacher::viewAssignedStudents() {
    system("cls");
    
    vector<Course>& courses = LMSManager::getInstance()->getCourses();
    
    if (courses.empty()) {
        cout << "No courses available.\n";
        system("pause");
        return; 
    }

    
    vector<Course> assignedCourses;

    // Filter courses to find those assigned to the current teacher
    for (const auto& course : courses) {
        if (course.getTeacherEmail() == getEmail()) {
            assignedCourses.push_back(course);
        }
    }

    
    if (assignedCourses.empty()) {
        cout << "You are not assigned to any courses. Cannot view students.\n";
        system("pause");
        return; 
    }

    
    cout << "Your Assigned Courses:\n";
    for (size_t i = 0; i < assignedCourses.size(); ++i) {
        cout << i + 1 << ". " << assignedCourses[i].getCourseName() << endl;
    }

    int index = Validator::getValidatedIntInput(
        "Enter course index (1-" + to_string(assignedCourses.size()) + "): ",
        1, assignedCourses.size());

    try {
    Course& course = assignedCourses[index - 1]; 

    // Check if there are any students in the course
    const auto& students = course.getStudents();
    cout << "Course: " << course.getCourseName() << " has " << students.size() << " students.\n"; // Debug print

    if (students.empty()) {
        system("pause");
        cout << "There are no students enrolled in this course.\n";
    } else {
        
        course.displayStudents();
        system("pause");
    }
    
    } catch (const exception& e) {
        cout << e.what() << endl;
        system("pause");
    }
}
void Teacher::addContent() {
    system("cls");
    
    vector<Course>& courses = LMSManager::getInstance()->getCourses();

    if (courses.empty()) {
        cout << "No courses available.\n";
        system("pause");
        return; 
    }

  
    vector<Course> assignedCourses;

    
    for (const auto& course : courses) {
        if (course.getTeacherEmail() == getEmail()) {
            assignedCourses.push_back(course);
        }
    }

   
    if (assignedCourses.empty()) {
        cout << "You are not assigned to any courses. Cannot add content.\n";
        system("pause");
        return; 
    }

    
    cout << "Your Assigned Courses:\n";
    for (size_t i = 0; i < assignedCourses.size(); ++i) {
        cout << i + 1 << ". " << assignedCourses[i].getCourseName() << endl;
    }

    int index = Validator::getValidatedIntInput(
        "Enter course index (1-" + to_string(assignedCourses.size()) + "): ",
        1, assignedCourses.size());

    try {
        Course& course = assignedCourses[index - 1]; 

        string content;
        cout << "Enter the content to add: ";
        cin.ignore();
        getline(cin, content);
        
       
        course.addContent(content); 
        
        cout << "Content added to the course: " << course.getCourseName() << endl;
        system("pause");
    } catch (const exception& e) {
        cout << e.what() << endl;
        system("pause");
    }
}

void Teacher::viewCourse() {
    system("cls"); 

    vector<Course>& courses = LMSManager::getInstance()->getCourses();
    if (courses.empty()) {
        cout << "No courses available to view.\n";
        system("pause");
        return; 
    }

   
    vector<Course> assignedCourses;

   
    for (const auto& course : courses) {
        if (course.getTeacherEmail() == this->getEmail()) {
            assignedCourses.push_back(course);
        }
    }

   
    if (assignedCourses.empty()) {
        cout << "No courses are assigned to you.\n";
        system("pause");
        return;  
    }

   
    cout << "Your Assigned Courses:\n";
    for (size_t i = 0; i < assignedCourses.size(); ++i) {
        cout << i + 1 << ". " << assignedCourses[i].getCourseName() << endl;
    }

    int index;
    cout << "Enter course index to view (1-based): ";
    cin >> index;

    try {
        
        if (index < 1 || index > assignedCourses.size()) {
            throw out_of_range("Invalid index");
        }

        
        Course& course = assignedCourses[index - 1];
        cout << "Viewing course: " << course.getCourseName() << endl;
        course.displayContents();
        system("pause");  
    } catch (InvalidCourseIndexException&) {
        cout << "Invalid course index.\n";
    } catch (out_of_range&) {
        cout << "Invalid course index.\n";
    }
}


void Teacher::viewReports() {
    system("cls");  
    vector<Course>& courses = LMSManager::getInstance()->getCourses();
    string teacherEmail = getEmail();

    bool hasCourses = false;
    cout << "Courses Report for " << teacherEmail << ":\n";
    for (auto& course : courses) {
        if (course.getTeacherEmail() == teacherEmail) {
            hasCourses = true;
            cout << "Course: " << course.getCourseName() << "\n";
            cout << "Enrolled Students:\n";
            course.displayStudents();
            cout << "Grades:\n";
            course.displayGrades();
        system("pause"); 
            cout << "----------------------\n";
        }
    }

    if (!hasCourses) {
        cout << "No courses assigned to you.\n";
    }
    system("pause");
}

// Student class implementation
void Student::displayMenu() {
    int choice;
    do {
        system("cls");
        cout << "\nStudent Menu:\n";
        cout << "1. View Enrolled Courses\n";
        cout << "2. View Grades\n";
        cout << "3. Log Out\n";

        choice = Validator::getValidatedIntInput("Enter choice (1-3): ", 1, 3);

        switch (choice) {
            case 1:
                viewEnrolledCourses();
                break;
            case 2:
                viewGrades();
                system("pause");
                break;
            case 3:
                cout << "Logging out...\n";
                system("pause");
                break;
        }
    } while (choice != 3);
}

void Student::viewEnrolledCourses() {
    vector<Course>& allCourses = LMSManager::getInstance()->getCourses();
    vector<Course> enrolledCourses;

    
    for (Course& course : allCourses) {
        for (const string& studentEmail : course.getStudents()) {
            if (studentEmail == email) {
                enrolledCourses.push_back(course);
                break;
            }
        }
    }

    
    if (enrolledCourses.empty()) {
        cout << "You are not enrolled in any courses.\n";
        return;
    }

    
    cout << "Your Enrolled Courses:\n";
    for (size_t i = 0; i < enrolledCourses.size(); ++i) {
        cout << i + 1 << ": " << enrolledCourses[i].getCourseName() 
             << " (Teacher: " << enrolledCourses[i].getTeacherEmail() << ")\n";
    }

    int index = Validator::getValidatedIntInput(
        "Enter course index to view content (or 0 to go back): ", 
        0, enrolledCourses.size()
    );

    if (index == 0) return;

   
    try {
        Course& selectedCourse = enrolledCourses[index - 1];
        cout << "Selected course: " << selectedCourse.getCourseName() << endl; 
        selectedCourse.displayContents();
        system("pause");
    } catch (const exception& e) {
        cout << "Error viewing course contents: " << e.what() << endl;
    }
}

void Student::viewGrades() {
    vector<Course>& allCourses = LMSManager::getInstance()->getCourses();
    vector<Course> enrolledCourses;

    
    for (Course& course : allCourses) {
        for (const string& studentEmail : course.getStudents()) {
            if (studentEmail == email) {
                enrolledCourses.push_back(course);
                break;
            }
        }
    }

    
    if (enrolledCourses.empty()) {
        cout << "You are not enrolled in any courses.\n";
        return;
    }

    
    cout << "Your Enrolled Courses:\n";
    for (size_t i = 0; i < enrolledCourses.size(); ++i) {
        cout << i + 1 << ": " << enrolledCourses[i].getCourseName() 
             << " (Teacher: " << enrolledCourses[i].getTeacherEmail() << ")\n";
    }

    int index = Validator::getValidatedIntInput(
        "Enter course index to view grades (or 0 to go back): ", 
        0, enrolledCourses.size()
    );

    if (index == 0) return;
    
   
    try {
        Course& selectedCourse = enrolledCourses[index - 1];
        
       
        bool gradeFound = false;
        for (auto& grade : selectedCourse.getGrades()) {
            if (grade.first == email) {
                cout << "Your Grade in " << selectedCourse.getCourseName() 
                     << ": " << grade.second << "%" << endl;
                gradeFound = true;
                break;
            }
        }

        if (!gradeFound) {
            cout << "No grade available for this course.\n";
        }
    } catch (const exception& e) {
        cout << "Error viewing grades: " << e.what() << endl;
    }
}

void Student::enrollInCourse() {
    vector<Course>& courses = LMSManager::getInstance()->getCourses();
    vector<Course> unenrolledCourses;

    
    for (Course& course : courses) {
        bool alreadyEnrolled = false;
        for (const string& studentEmail : course.getStudents()) {
            if (studentEmail == email) {
                alreadyEnrolled = true;
                break;
            }
        }
        
        if (!alreadyEnrolled) {
            unenrolledCourses.push_back(course);
        }
    }

   
    if (unenrolledCourses.empty()) {
        cout << "No courses available for enrollment.\n";
        return;
    }

   
    cout << "Available Courses:\n";
    for (size_t i = 0; i < unenrolledCourses.size(); ++i) {
        cout << i + 1 << ": " << unenrolledCourses[i].getCourseName() 
             << " (Teacher: " << unenrolledCourses[i].getTeacherEmail() << ")\n";
    }

    int courseIndex = Validator::getValidatedIntInput(
        "Enter course index to enroll (or 0 to go back): ", 
        0, unenrolledCourses.size()
    );

    if (courseIndex == 0) return;

    try {
        Course& course = unenrolledCourses[courseIndex - 1];
        course.enrollStudent(email);
        cout << "Successfully enrolled in the course: " 
             << course.getCourseName() << endl;
    } catch (const exception& e) {
        cout << e.what() << endl;
    }
}


int main() {
   try {
        LMSManager* lms = LMSManager::getInstance();

        Course course1("Mathematics", "teacher1@example.com");
        course1.addContent("Introduction to Algebra");
        course1.addContent("Advanced Calculus");

        Course course2("Physics", "teacher2@example.com");
        course2.addContent("Newton's Laws");
        course2.addContent("Thermodynamics");

        lms->addCourse(course1);
        lms->addCourse(course2);

        users.push_back(make_shared<Admin>("admin1", "admin1@example.com", "adminpass"));
        users.push_back(make_shared<Teacher>("teacher1", "teacher1@example.com", "teacherpass"));
        users.push_back(make_shared<Teacher>("teacher2", "teacher2@example.com", "teacherpass"));
        

        string email, password;
        bool loggedIn = false;

        while (true) {
            while (!loggedIn) {
                system("cls");
                cout << "Learning Management System Login\n";
                cout << "================================\n";
                cout << "Enter your email (or type '0' to exit): ";
                cin >> email;

                if (email == "0") {
                    cout << "Exiting program...\n";
                    return 0;
                }

                cout << "Enter your password: ";
                cin >> password;

                for (const auto& user : users) {
                    if (user->getEmail() == email && user->getPassword() == password) {
                        loggedIn = true;

                        
                        if (auto admin = dynamic_cast<Admin*>(user.get())) {
                            user->setActionStrategy(new AdminActions(admin));
                        } else if (auto teacher = dynamic_cast<Teacher*>(user.get())) {
                            user->setActionStrategy(new TeacherActions(teacher));
                        } else if (auto student = dynamic_cast<Student*>(user.get())) {
                            user->setActionStrategy(new StudentActions(student));
                        }

                        user->performAction(); 
                        break;
                    }
                }

                if (!loggedIn) {
                    cout << "Invalid login credentials. Please try again.\n";
                    system("pause");
                }
            }

            char changeRole;
            cout << "Do you want to log in as a different role? (y/n): ";
            cin >> changeRole;

            if (tolower(changeRole) == 'n') {
                cout << "Logging out...\n";
                break;
            }
            loggedIn = false;
        }
    }
    catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }

    return 0;
}