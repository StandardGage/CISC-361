#include "student.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Student *inputStudent()
{
    Student *student = (Student *)malloc(sizeof(Student));
    student->lastName = (char *)malloc(50);
    student->firstName = (char *)malloc(50);
    student->year = (char *)malloc(50);

    printf("\nEnter the student's last name: ");
    fgets(student->lastName, 50, stdin);

    printf("Enter the student's first name: ");
    fgets(student->firstName, 50, stdin);

    printf("Enter the student's ID number: ");
    scanf("%ld", &student->idNumber);
    getchar(); // clear the newline character

    printf("Enter the student's year: ");
    fgets(student->year, 50, stdin);

    printf("Enter the student's graduation year: ");
    scanf("%d", &student->graduationYear);

    return student;
}

void addStudent(Student **head, Student **tail, Student *student)
{
    student->next = NULL;
    student->prev = *tail;

    if (*head == NULL)
    {
        *head = student;
        *tail = student;
    }
    else
    {
        (*tail)->next = student;
        *tail = student;
    }
}

void deleteByLastName(Student **head, Student **tail, char *lastName)
{
    Student *current = *head;
    Student *nextNode = NULL;
    while (current != NULL)
    {
        nextNode = current->next;

        if (strcmp(current->lastName, lastName) == 0)
        {
            if (current->prev != NULL)
            {
                current->prev->next = current->next;
            }
            else
            {
                *head = current->next;
            }

            if (current->next != NULL)
            {
                current->next->prev = current->prev;
            }
            else
            {
                *tail = current->prev;
            }

            deallocateStudent(current);
        }

        current = nextNode;
    }
}

void printStudents(Student *head, int reverse)
{
    Student *current = head;
    while (current != NULL)
    {
        printf("\nLast name: %s", current->lastName);
        printf("First name: %s", current->firstName);
        printf("ID number: %ld\n", current->idNumber);
        printf("Year: %s", current->year);
        printf("Graduation year: %d\n", current->graduationYear);
        printf("\n");

        if (reverse)
        {
            current = current->prev;
        }
        else
        {
            current = current->next;
        }
    }
}

void deallocateStudent(Student *student)
{
    free(student->lastName);
    free(student->firstName);
    free(student->year);
    free(student);
}

void deallocateStudents(Student *head)
{
    Student *current = head;
    while (current != NULL)
    {
        Student *next = current->next;
        deallocateStudent(current);
        current = next;
    }
}

int main()
{
    struct Student *head = NULL;
    struct Student *tail = NULL;

    int choice;
    while (1)
    {
        printf("\n1. Add a student\n");
        printf("2. Delete all students by last name\n");
        printf("3. Print all students\n");
        printf("4. Print all students in reverse\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        scanf("%d", &choice);
        getchar(); // clear the newline character

        switch (choice)
        {
        case 1:
        {
            Student *student = inputStudent();
            addStudent(&head, &tail, student);
            break;
        }
        case 2:
        {
            char lastName[50];
            printf("\nEnter the last name of the student to delete: ");
            fgets(lastName, 50, stdin);

            deleteByLastName(&head, &tail, lastName);
            break;
        }
        case 3:
        {
            printStudents(head, 0);
            break;
        }
        case 4:
        {
            printStudents(tail, 1);
            break;
        }
        case 5:
        {
            deallocateStudents(head);
            return 0;
        }
        }
    }
}