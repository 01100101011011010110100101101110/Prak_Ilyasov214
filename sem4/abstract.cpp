#include <iostream>
#include <string>
#include <cstring>

using namespace std;

class Person {
    static int persons_in_hospital;
    char *first_name;
    char *second_name;
    int birth_year;
    bool vaccinated;
public:
    Person() { 
        first_name = NULL; 
        second_name = NULL;
    }

    Person(const char *name1, const char *name2, int birth, bool vacc);
    Person(const Person& pers);

    virtual ~Person();

    Person& operator=(const Person& p);
    friend ostream& operator<<(ostream& out, const Person& pers);

    const char *Get_name() const {
        return first_name;
    }
    const char *Get_surname() const {
        return second_name;
    }
    virtual void Get_info () const = 0;

};

Person::Person(const char *name1, const char *name2, int birth, bool vacc) {
    if (name1 != NULL) {
        first_name = new char[strlen(name1) + 1];
        strcpy(first_name, name1);
    } else {
        first_name = NULL;
    }
    if (name2 != NULL) {
        second_name = new char[strlen(name2) + 1];
        strcpy(second_name, name2);
    } else {
        second_name = NULL;
    }
    birth_year = birth;
    vaccinated = vacc;
    persons_in_hospital++;
}

Person::Person(const Person& pers) {
    first_name = new char[strlen(pers.first_name) + 1];
    second_name = new char[strlen(pers.second_name) + 1];
    strcpy(first_name, pers.first_name);
    strcpy(second_name, pers.second_name);
    birth_year = pers.birth_year;
    vaccinated = pers.vaccinated;
}

Person::~Person() {
    if (first_name != NULL) {
        delete[]first_name;
    }
    if (second_name != NULL) {
        delete[]second_name;
    }
    persons_in_hospital--;
}

Person& Person::operator=(const Person& pers) {
    if (first_name != NULL) {
        delete[]first_name;
    }
    first_name=new char[strlen(pers.first_name) + 1];
    if (second_name != NULL) {
        delete[]second_name;
    }
    second_name=new char[strlen(pers.second_name) + 1];
    strcpy(first_name, pers.first_name);
    strcpy(second_name,pers.second_name);
    birth_year = pers.birth_year;
    
    return *this; 
}

ostream& operator<<(ostream& out, const Person& pers) {
    out << "ФИО: " << pers.second_name << " " << pers.first_name << endl;

    return out;
}

class Patient : public Person {
    int how_long_in_hosp;   //days
public:
    Patient() : Person(){}
    Patient(const char *name1, const char *name2, int birth, bool vacc, int how_long) : Person(name1, name2, birth, vacc) {
        how_long_in_hosp = how_long;
    }
    Patient(const Patient& pat) : Person(pat) {
        how_long_in_hosp = pat.how_long_in_hosp;
    }
    virtual void Get_info () const {
        cout << "Пациент. Пребывает в больнице " << how_long_in_hosp << " дней." << endl;
    }
    ~Patient(){}
};



class Hospital_staff : public Person {
    int work_experience;    //years
public:
    Hospital_staff() : Person(){}
    Hospital_staff(const char *name1, const char *name2, int birth, bool vacc, int work_exp) : Person(name1, name2, birth, vacc) {
        work_experience = work_exp;
    }
    Hospital_staff(const Hospital_staff& staff) : Person(staff) {
        work_experience = staff.work_experience;
    }
    virtual void Get_info () const {
        cout << "Мед. персонал. Стаж работы " << work_experience << " лет." << endl;
    }
    ~Hospital_staff(){}
};

int Person::persons_in_hospital = 0;

int main() {   
    return 0;
}
