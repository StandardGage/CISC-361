#ifndef STUDENT_H
#define STUDENT_H

typedef struct Student {
    char* lastName;
    char* firstName;
    long idNumber;
    char* year;
    int graduationYear;
    struct Student* next;
    struct Student* prev;
} Student;

Student* inputStudent();

void addStudent(Student** head, Student** tail, Student* student);

void deleteByLastName(Student** head, Student** tail, char* lastName);

void printStudents(Student* head, int reverse);

void deallocateStudent(Student* student);

void deallocateStudents(Student* head);

#endif