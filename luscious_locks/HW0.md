# Welcome to Homework 0!

For these questions you'll need the mini course and  "Linux-In-TheBrowser" virtual machine (yes it really does run in a web page using javascript) at -

http://cs-education.github.io/sys/

Let's take a look at some C code (with apologies to a well known song)-
```C
// An array to hold the following bytes. "q" will hold the address of where those bytes are.
// The [] mean set aside some space and copy these bytes into teh array array
char q[] = "Do you wanna build a C99 program?";

// This will be fun if our code has the word 'or' in later...
#define or "go debugging with gdb?"

// sizeof is not the same as strlen. You need to know how to use these correctly, including why you probably want strlen+1

static unsigned int i = sizeof(or) != strlen(or);

// Reading backwards, ptr is a pointer to a character. (It holds the address of the first byte of that string constant)
char* ptr = "lathe"; 

// Print something out
size_t come = fprintf(stdout,"%s door", ptr+2);

// Challenge: Why is the value of away equal to 1?
int away = ! (int) * "";


// Some system programming - ask for some virtual memory

int* shared = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
munmap(shared,sizeof(int*));

// Now clone our process and run other programs
if(!fork()) { execlp("man","man","-3","ftell", (char*)0); perror("failed"); }
if(!fork()) { execlp("make","make", "snowman", (char*)0); execlp("make","make", (char*)0)); }

// Let's get out of it?
exit(0);
```

## So you want to master System Programming? And get a better grade than B?
```C
int main(int argc, char** argv) {
	puts("Great! We have plenty of useful resources for you, but it's up to you to");
	puts(" be an active learner and learn how to solve problems and debug code.");
	puts("Bring your near-completed answers the problems below");
	puts(" to the first lab to show that you've been working on this.");
	printf("A few \"don't knows\" or \"unsure\" is fine for lab 1.\n"); 
	puts("Warning: you and your peers will work hard in this class.");
	puts("This is not CS225; you will be pushed much harder to");
	puts(" work things out on your own.");
	fprintf(stdout,"This homework is a stepping stone to all future assignments.\n");
	char p[] = "So, you will want to clear up any confusions or misconceptions.\n";
	write(1, p, strlen(p) );
	char buffer[1024];
	sprintf(buffer,"For grading purposes, this homework 0 will be graded as part of your lab %d work.\n", 1);
	write(1, buffer, strlen(buffer));
	printf("Press Return to continue\n");
	read(0, buffer, sizeof(buffer));
	return 0;
}
```
## Watch the videos and write up your answers to the following questions

**Important!**

The virtual machine-in-your-browser and the videos you need for HW0 are here:

http://cs-education.github.io/sys/

The coursebook:

http://cs341.cs.illinois.edu/coursebook/index.html

Questions? Comments? Use Ed: (you'll need to accept the sign up link I sent you)
https://edstem.org/

The in-browser virtual machine runs entirely in Javascript and is fastest in Chrome. Note the VM and any code you write is reset when you reload the page, *so copy your code to a separate document.* The post-video challenges (e.g. Haiku poem) are not part of homework 0 but you learn the most by doing (rather than just passively watching) - so we suggest you have some fun with each end-of-video challenge.

HW0 questions are below. Copy your answers into a text document (which the course staff will grade later) because you'll need to submit them later in the course. More information will be in the first lab.

## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".
 ```C
#include <unistd.h>

int main() {
	write(1, "Hi! my name is Jerry Liu\n", 25);
	return 0;
}
```
### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
   - The triangle should look like this, for n = 3:
   ```C
   *
   **
   ***
   ```
```C
#include <unistd.h>
void write_triangle(int n) {
	int i;
	int j;
	for (i = 0; i < n; i++) {
		for (j = 0; j <= i; j++) {
			write(STDERR_FILENO, "*", 1);
		}
		write(STDERR_FILENO, "\n", 1);
	}
}

int main() {
	write_triangle(3);
	return 0;
}
```
### Writing to files
3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
   - Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).
 ```C
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	mode_t mode = S_IRUSR | S_IWUSR;
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	write(fildes, "Hi! my name is Jerry Liu\n", 25);
	close(fildes);
	return 0;
}
```
### Not everything is a system call
4. Take your program from "Writing to files" and replace `write()` with `printf()`.
   - Make sure to print to the file instead of standard out!
```C
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	mode_t mode = S_IRUSR | S_IWUSR;
	close(1);
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	printf("Hi! my name is Jerry Liu\n");
	close(fildes);
	return 0;
}
```
5. What are some differences between `write()` and `printf()`?
```C
write() is a system call while printf() is a standard library call. write() cannot print out
integer values but printf() can. 
```

## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte?
```
At least 8 bits depending on the machine you are using.
```
2. How many bytes are there in a `char`?
```
1 byte.
```
3. How many bytes the following are on your machine?
   - `int`, `double`, `float`, `long`, and `long long`
```C
int is 4 bytes. 
double is 8 bytes.
float is 4 bytes.
long is 4 bytes.
long long is 8 bytes.
```
### Follow the int pointer
4. On a machine with 8 byte integers:
```C
int main(){
    int data[8];
} 
```
If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?
'''
Since integer is 8 bytes, the address of data+2 = 0x7fbd9d40 + 8 * 2 = 0x7fbd9d50
'''

5. What is `data[3]` equivalent to in C?
   - Hint: what does C convert `data[3]` to before dereferencing the address?
```
It is equivalent to *(data + 3) and 3[data]
```

### `sizeof` character arrays, incrementing pointers
  
Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
```C
char *ptr = "hello";
*ptr = 'J';
```
```
Because the memory of "hello", which is a string, can only be read. Segfault appears since the program tries to operate on a memory location in a way that is not allowed.
```
7. What does `sizeof("Hello\0World")` return?
```
12
```
8. What does `strlen("Hello\0World")` return?
```
5
```
9. Give an example of X such that `sizeof(X)` is 3.
```
"Hi"
```
10. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.
```
Y can be an integer or a pointer.
```

## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?
```
The first way is using argc, which is the length of argv by definition. The second way is having a counter that starts at 0, looping through every char in argv, incrementing the counter by 1 each time until null.
```

2. What does `argv[0]` represent?
```
The first argument, which is the execution name
```
### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?
```
It is stored somewhere else in memory. We need to use the extern keyword to access it.
```
### String searching (strings are just char arrays)
4. On a machine where pointers are 8 bytes, and with the following code:
```C
char *ptr = "Hello";
char array[] = "Hello";
```
What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?
```
The value of sizeof(ptr) is 8. sizeof(ptr) references the size of a char pointer, which is 8 bytes.
The value of sizeof(array) is 6. sizeof(array) references the size of the char array, which is 5 characters
for "Hello" plus 1 char for the null character in the end.  
```

### Lifetime of automatic variables

5. What data structure manages the lifetime of automatic variables?
```
Stack
```

## Chapter 4

Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?
```
It should be put onto heap by using malloc
```
2. What are the differences between heap and stack memory?
```
Stack memory is automatically freed after the function returns; variables stored on the stack live for the length of the function. 
Heap memory is not automatically freed, it needs to be manully freed using free(); varaibles stored on the heap live for the length of the process until it is manully freed.
```
3. Are there other kinds of memory in a process?
```
Yes, text segment
```
4. Fill in the blank: "In a good C program, for every malloc, there is a ___".
```free```
### Heap allocation gotchas
5. What is one reason `malloc` can fail?
```
the heap has run out of space
```
7. What are some differences between `time()` and `ctime()`?
```
time() is a system call that returns a time_t that has the value of the number of seconds since 1970. ctime() is a function in the c library. It returns a char*(string) that is the human-readable format time of its parameter
```
9. What is wrong with this code snippet?
```C
free(ptr);
free(ptr);
```
```
it frees the same pointer twice
```
8. What is wrong with this code snippet?
```C
free(ptr);
printf("%s\n", ptr);
```
```
It uses memory that has already been freed
```
9. How can one avoid the previous two mistakes? 
```
set pointers to null after they have been freed
```
### `struct`, `typedef`s, and a linked list
10. Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s).
```C
#include <stdio.h>
#include <stdlib.h>

struct Person {
	char* name;
	int age;
	struct Person** friends;
};

typedef struct Person person_t;
```
11. Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.
```C
#include <stdio.h>
#include <stdlib.h>

struct Person {
	char* name;
	int age;
	struct Person** friends;
};

typedef struct Person person_temp;

int main() {
	person_temp* ptr1 = (person_temp*) malloc(sizeof(person_temp));
	person_temp* ptr2 = (person_temp*) malloc(sizeof(person_temp));
	ptr1 -> name = "Agent Smith";
	ptr1 -> age = 128;
	ptr1 -> friends = &ptr2;
	ptr2 -> name = "Sonny Moore";
	ptr2 -> age = 256;
	ptr2 -> friends = &ptr1;
	free(ptr1);
	ptr1 = NULL;
	free(ptr2);
	ptr2 = NULL;
	return 0;
}
```
### Duplicating strings, memory allocation and deallocation of structures
Create functions to create and destroy a Person (Person's and their names should live on the heap).
12. `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).
```C
person_temp* create(char* name_, int age_) {
	person_temp* result = (person_temp*) malloc(sizeof(person_temp));
	result -> name = strdup(name_);
	result -> age = age_;
	result -> friends = malloc(sizeof(person_temp*) * 10);
	int i;
	for (i = 0; i < 10; i++) {
		result -> friends[i] = NULL;
	}
	return result;
}
```
13. `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.
```
void destroy(person_temp* ptr) {
	free(ptr -> name);
	ptr -> name = NULL;
	free(ptr -> friends);
	ptr -> friends = NULL;
	memset(ptr, 0, sizeof(person_temp));
	free(ptr);
	ptr = NULL;
}
```

## Chapter 5 

Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?
```
getchar() and putchar()
```
2. Name one issue with `gets()`.
```
gets() can lead to buffer overflow. There is no way to tell if an input is too long for gets().
```
### Introducing `sscanf` and friends
3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".
```C
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int main() {
	char first[20];
	int second;
	char third[20];
	sscanf("Hello 5 World", "%s %d %s", first, &second, third);
	printf("Result: %s %d %s\n", first, second, third);
	return 0;
}
```
### `getline` is useful
4. What does one need to define before including `getline()`?
```
_GNU_SOURCE
```
5. Write a C program to print out the content of a file line-by-line using `getline()`.
```C
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int main() {
	FILE* fp = fopen("test.txt", "r");
	char* buffer = NULL;
	size_t capacity = 0;
	while(getline(&buffer, &capacity, fp) != -1) {
		printf("%s", buffer);
	}
	fclose(fp);
	free(buffer);
	return EXIT_SUCCESS;
}
```

## C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here

1. What compiler flag is used to generate a debug build?
```
-g
```
2. You modify the Makefile to generate debug builds and type `make` again. Explain why this is insufficient to generate a new build.
```
Becasue part of the pervious build will be reused when typing make again; you need to type make clean before make to generate a new build
```
3. Are tabs or spaces used to indent the commands after the rule in a Makefile?
```
Tabs
```
4. What does `git commit` do? What's a `sha` in the context of git?
```
git commit records the project's currently staged changes; sha stands for simple hashing algorithm. It is the checksum of all changes plus a header.
```
5. What does `git log` show you?
```
The history of the repository.
```
6. What does `git status` tell you and how would the contents of `.gitignore` change its output?
```
git status shows the changes you made, what changes are staged, and which files are untracked. git status would ignore the untracked files in .gitignore.
```
7. What does `git push` do? Why is it not just sufficient to commit with `git commit -m 'fixed all bugs' `?
```
git push uploads the content in the local repository to the remote repository. It is not sufficient to just commit because commit only records the changes but does not change the remote repository.
```
8. What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?
```
This means some changes have been made to the remote repository, but these changes are not in your local repository. The most common way is to do git fetch, git rebase before doing git push again.
```

## Optional (Just for fun)
- Convert your a song lyrics into System Programming and C code and share on Ed.
- Find, in your opinion, the best and worst C code on the web and post the link to Ed.
- Write a short C program with a deliberate subtle C bug and post it on Ed to see if others can spot your bug.
- Do you have any cool/disastrous system programming bugs you've heard about? Feel free to share with your peers and the course staff on Ed.
