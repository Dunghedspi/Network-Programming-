#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char weekDay[7][10] = {"sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "saturday"};
char AM_PM[2][10] = {"morning", "afternoon"};

typedef struct schedule
{
	char code[10],
		Courses[100],
		WeekDay[100],
		AM_PM[20],
		Pernoid[10],
		Week[30],
		Room[10];

	struct schedule *next;
} schedule;

schedule *readFile(FILE *ptr, schedule *root);
int printSchedule(schedule *root);
schedule *createSubject();
int printSchedule2(schedule *root);

int main(int argc, char const *argv[])
{

	FILE *fp;
	schedule *root = NULL;
	argv[1] = "text.txt";
	fp = fopen(argv[1], "r");

	if (fp != NULL)
	{
		root = readFile(fp, root);
	}

	printSchedule2(root);
	return 0;
}

// create new Node in Linklist
schedule *createSubject()
{
	schedule *subject = (schedule *)malloc(sizeof(schedule));
	subject->next = NULL;
	return subject;
}

/*
readfile and insert linklist
@param FILE * ptr is a pointer of the currently read file
@param schedule *root  is managed single-linked list pointer
*/
schedule *readFile(FILE *ptr, schedule *root)
{

	char buff[1000], buff1[1000];
	schedule *subject, *current = root;
	while ((fgets(buff, sizeof(buff), ptr)) != NULL)
	{

		//IT4935 Database Lab 615,616,22,25-31,D6-303;
		//--> IT4935 Database Lab 615,616,22,25-31,D6-303
		int i = strlen(buff) - 1;
		while (buff[i] != ';')
			i--;
		buff[i + 1] = '\0';

		subject = createSubject();
		//buff1 = 615,616,22,25-31,D6-303
		while (buff[i] != ' ')
			i--;
		strcpy(buff1, buff + i + 1);

		// buff = IT4935 Database Lab
		buff[i] = '\0';
		i = 0;
		while (buff[i] != ' ')
			i++;
		// i is address of the space between code and courses
		//code = IT4935
		strncpy(subject->code, buff, i);
		//courses = Database Lab
		strcpy(subject->Courses, buff + i + 1);

		// i = 615
		i = atoi(buff1);
		// pernoid = "5 - 6"
		subject->Pernoid[0] = i % 10 + '0';
		subject->Pernoid[1] = '-';

		//atoi(buff1+4) = 616
		subject->Pernoid[2] = atoi(buff1 + 4) % 10 + '0';
		subject->Pernoid[3] = '\0';

		// i = 516 -> i/100 - 1  The position is equivalent to the one that is formatted in the weekDay array
		strcpy(subject->WeekDay, weekDay[i / 100 - 1]);
		i = i % 100;
		strcpy(subject->AM_PM, AM_PM[i / 10 - 1]);

		//room
		i = 0;
		while (buff1[i] < 'A' || buff1[i] > 'Z')
			i++;

		strcpy(subject->Room, buff1 + i);
		subject->Room[strlen(subject->Room) - 1] = '\0';
		buff1[i - 1] = '\0';

		//week
		strcpy(subject->Week, buff1 + 8);

		if (root == NULL)
		{
			root = subject;
		}
		else
		{
			current->next = subject;
		}
		current = subject;
	}
	return root;
}

int printSchedule(schedule *root)
{

	printf("%-10s %-20s %-10s %-10s %-10s %-15s %-5s\n", "Code", "Course", "Week_Day", "AM/PM", "Pernoid", "Week", "Room");

	while (root != NULL)
	{

		printf("%-10s %-20s %-10s %-10s %-10s %-15s %-5s\n", root->code, root->Courses, root->WeekDay, root->AM_PM, root->Pernoid, root->Week, root->Room);
		root = root->next;
	}
	return 1;
}

int printSchedule2(schedule *root)
{

	printf("=====================================================================\nT  |");
	for (int i = 1; i < 6; i++)
	{
		printf("%-12s|", weekDay[i]);
	}
	printf("\n---------------------------------------------------------------------\n");
	schedule *ptr = root;
	for (int i = 1; i <= 12; i++)
	{
		printf("%-3d|", i);
		int count = 1, flage;
		if (i <= 6)
		{
			while (count <= 5)
			{
				ptr = root;
				flage = 1;
				while (ptr != NULL)
				{
					if (strcmp(ptr->AM_PM, "morning") == 0 && strcmp(ptr->WeekDay, weekDay[count]) == 0)
					{
						int start = atoi(ptr->Pernoid);
						int stop = atoi(ptr->Pernoid + 2);
						if (i >= start && i <= stop)
						{
							printf("%-12s|", ptr->Room);
							flage = 0;
						}
					}
					ptr = ptr->next;
				}
				if (flage)
				{
					printf("%-12s|", " ");
				}
				count++;
			}
		}
		else
		{
			while (count <= 5)
			{
				ptr = root;
				flage = 1;
				while (ptr != NULL)
				{
					if (strcmp(ptr->AM_PM, "afternoon") == 0 && strcmp(ptr->WeekDay, weekDay[count]) == 0)
					{
						int start = atoi(ptr->Pernoid);
						int stop = atoi(ptr->Pernoid + 2);
						if ((i - 6) >= start && (i - 6) <= stop)
						{
							printf("%-12s|", ptr->Room);
							flage = 0;
						}
					}
					ptr = ptr->next;
				}
				if (flage)
				{
					printf("%-12s|", " ");
				}
				count++;
			}
		}
		printf("\n---------------------------------------------------------------------\n");
	}
}