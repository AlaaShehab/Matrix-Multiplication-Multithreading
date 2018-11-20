#include <iostream>
#include <fstream>
#include <pthread.h>
#include <ctime>

#define DIM 4
#define INFO 3
using namespace std;

ifstream readfile;
ofstream writefile;
int** matrix1;
int** matrix2;
int** resultMatrix;
int matricesSize[DIM];

struct params {
    int row;
    int col;
    int loopCounter;
};
//read matrices dimensions
//pass by reference so no return is needed
void readMatrixDim(int &row, int &col) {

    if (!readfile.is_open()) {
        cout << "File Is Not Opened" << endl;
    }
    readfile >> row >> col;

}
//first initialize the 2d pointers matrices
//return the matrices pointers
int** readMatrix(int row, int col) {

    int** matrix = 0;
    matrix = new int*[row];

    for (int i = 0; i < row; i++) {
        matrix[i] = new int[col];
        for (int j = 0; j < col; j++) {
            readfile >> matrix[i][j];
        }
    }
    return matrix;
}
//initialize result matrix (2d pointers) for synchronization reasons
int** initializeMatrix () {
    int row = matricesSize[0];
    int col = matricesSize[3];
    resultMatrix = new int*[row];
    for (int i = 0; i < row; i++) {
        resultMatrix[i] = new int[col];
        for (int j = 0; j < col; j++) {
            resultMatrix[i][j] = NULL;
        }
    }
    return resultMatrix;
}


void *computeElement(void* param) {
    //extract parameters of thread
    struct params* p = (struct params*)param;
    int loopEnd = p->loopCounter;
    //set element dimensions
    int elementValue = 0;
    int rowNum = p->row;
    int colNum = p->col;
    //loop to calculate required element
    for (int i = 0; i < loopEnd; i++) {
        elementValue += matrix1[rowNum][i] * matrix2[i][colNum];
    }
    //set value of element
    resultMatrix[rowNum][colNum] = elementValue;
}

int** executeElementMultip() {
    //calculate the number of threads required
    //num = row of first matrix and col of 2nd matrix
    int threadsNum = matricesSize[0] * matricesSize[3];
    pthread_t threads[threadsNum];
    //create threads and set the parameters for each one
   struct params param;
    for (int i = 0; i < matricesSize[0]; i++) {
        for (int j = 0; j < matricesSize[3]; j++) {
            param.row = i;
            param.col = j;
            param.loopCounter = matricesSize[2];
            //create and join thread
            int status = pthread_create(&threads[i], NULL, computeElement, (void*)&param);
            //check if an error occured while thread is being created
            if (status > 0) {
                cout << "Failed to create thread" << endl;
            }
            pthread_join(threads[i], NULL);
        }
    }
    return resultMatrix;
}
//methodNUmber parameter is just used for sake of illustration in file.out
void WriteFile(clock_t time, int methodNumber) {

    int row = matricesSize[0];
    int col = matricesSize[3];
    //write the matix and execution time in file
    writefile << " Matrix " << methodNumber << endl;
    for (int i = 0; i < row ;i++) {
        for (int j = 0; j < col; j++) {
            writefile << resultMatrix[i][j] << " ";
        }
        writefile << endl;
    }
    writefile << "Time elapsed : " << (float)time/CLOCKS_PER_SEC << "  Seconds" <<endl;

}


void *computeRow(void* param) {
    //extract parameters of the thread
    struct params* args = (struct params*) param;
    //loopEnd is the number of cols of 1st or rows of 2nd both are equal
    int loopEnd = args->loopCounter;
    //set row to be calculated
    int rowNum = args->row;
    //loop to calculate required row
    for (int j = 0; j < loopEnd; j++) {
        int elementValue = 0;
        for (int i = 0; i < loopEnd; i++) {
            elementValue += matrix1[rowNum][i] * matrix2[i][j];
        }
        resultMatrix[rowNum][j] = elementValue;
    }

}

int** executeRowMultip() {
    //calculate the number of threads required
    //num = row of result array which is = rows of 1st matix
    int threadsNum = matricesSize[0];
    pthread_t threads[threadsNum];
    //create threads and set the parameters for each one

   struct params param;
    for (int i = 0; i < threadsNum; i++) {
        param.row = i;
        param.col = 0;
        param.loopCounter = matricesSize[2];
        //create and join thread
        int status = pthread_create(&threads[i], NULL, computeRow, (void*)&param);
        //check if an error occured while thread is being created
        if (status > 0) {
                cout << "Failed to create thread" << endl;
        }
        pthread_join(threads[i], NULL);

    }
    return resultMatrix;
}


void start () {
    //open read and write files
    readfile.open("input.txt");
    writefile.open("output.txt",ios::out | ios::trunc);

    //read both matrices and their dimensions
    readMatrixDim(matricesSize[0], matricesSize[1]);
    matrix1 = readMatrix(matricesSize[0], matricesSize[1]);
    //keep file open to read second matrix
    readMatrixDim(matricesSize[2], matricesSize[3]);
    matrix2 = readMatrix(matricesSize[2], matricesSize[3]);

    //close reading file.
    readfile.close();

    //initialize matrix
    resultMatrix = initializeMatrix();
    //start timer o calculate execution time
    clock_t startTime = clock();
    //first technique
    resultMatrix = executeElementMultip();
    clock_t finishTime = clock();
    //write result from first technique
    WriteFile(finishTime - startTime, 1);

    startTime = clock();
    //second technique
    resultMatrix = executeRowMultip();
    finishTime = clock();

    WriteFile(finishTime - startTime , 2);

    writefile.close();
}

int main()
{

    start();

    return 0;
}
